#include "det_hash.h"

#include "det_clock.h"
#include "memory.h"
#include "runtime_config.h"
#include "runtime_log.h"

#include <xxhash.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace DetHash {
namespace {

constexpr uint32_t kMem1Base = 0x80000000u;
constexpr uint32_t kMem2Base = 0x90000000u;
constexpr size_t kMem1Bytes = 24u * 1024u * 1024u;
constexpr size_t kMem2Bytes = 64u * 1024u * 1024u;

// The guest OS's thread list and the OSThread fields the dead-stack skip reads (this SDK's layout,
// hle/os/os_internal.h and OSCreateThread in os_thread.cpp).
constexpr uint32_t kThreadListHeadAddr = 0x800000DCu;
constexpr uint32_t kRunningThreadAddr = 0x800000E4u;
constexpr uint32_t kThreadSavedSpOffset = 0x004u;    // OSContext gpr[1]
constexpr uint32_t kThreadListNextOffset = 0x2FCu;
constexpr uint32_t kThreadStackTopOffset = 0x304u;
constexpr uint32_t kThreadStackBottomOffset = 0x308u;  // holds the 0xDEADBABE guard word
constexpr uint32_t kThreadBytes = 0x310u;
constexpr uint32_t kMaxStackBytes = 4u * 1024u * 1024u;

struct Settings {
    std::filesystem::path hashFile;
    uint32_t every = 60;
    std::set<uint32_t> dumps;
    std::filesystem::path dumpDirectory;
    bool skipDeadStack = false;
};

const Settings& GetSettings() {
    static const Settings settings = [] {
        Settings parsed;
        if (const char* hash = std::getenv("NO100F_DET_HASH"); hash != nullptr && *hash != '\0') {
            std::string text(hash);
            // "<file>:<every>", but a Windows drive letter's colon is part of the path.
            const size_t colon = text.rfind(':');
            if (colon != std::string::npos && colon > 1 && colon + 1 < text.size() &&
                text.find_first_not_of("0123456789", colon + 1) == std::string::npos) {
                parsed.every = static_cast<uint32_t>(std::strtoul(text.c_str() + colon + 1, nullptr, 10));
                text.resize(colon);
            }
            if (parsed.every == 0) {
                parsed.every = 1;
            }
            parsed.hashFile = std::filesystem::u8path(text);
            if (parsed.hashFile.is_relative()) {
                parsed.hashFile = RuntimeConfigFile::ApplicationDataDirectory() / parsed.hashFile;
            }
        }
        if (const char* dump = std::getenv("NO100F_DET_DUMP"); dump != nullptr && *dump != '\0') {
            for (const char* p = dump; *p != '\0';) {
                char* end = nullptr;
                const unsigned long retrace = std::strtoul(p, &end, 10);
                if (end == p) {
                    break;
                }
                parsed.dumps.insert(static_cast<uint32_t>(retrace));
                p = *end == ',' ? end + 1 : end;
            }
        }
        parsed.dumpDirectory = parsed.hashFile.empty() ? RuntimeConfigFile::ApplicationDataDirectory() / "Logs"
                                                       : parsed.hashFile.parent_path();
        if (const char* skip = std::getenv("NO100F_DET_HASH_SKIP_DEAD_STACK"); skip != nullptr && std::string(skip) == "1") {
            parsed.skipDeadStack = true;
        }
        if (!parsed.hashFile.empty() || !parsed.dumps.empty()) {
            RT_LOGF(RT_TAG_RUNTIME, "determinism diagnostics: hashes every %u retraces to %s%s, %zu dump(s)\n",
                    parsed.every, RuntimeConfigFile::PathToUtf8(parsed.hashFile).c_str(),
                    parsed.skipDeadStack ? " (dead stack skipped)" : "", parsed.dumps.size());
        }
        return parsed;
    }();
    return settings;
}

uint64_t HashRange(uint32_t base, size_t bytes) {
    if (!Memory::Contains(base, bytes)) {
        return 0;
    }
    return XXH3_64bits(Memory::GetPointer(base, bytes), bytes);
}

// The unused part of every guest thread's stack, [bottom + guard word, SP): what is there was left
// by calls that already returned, and nothing reads it before writing it again. Two builds that
// behave the same can still leave different bytes there (a function moved between native code and
// translated code, a different frame size), so a comparison of builds skips it. The running thread's
// SP is the live register (`runningSp`); the others' is the one saved at their last switch. A thread
// whose fields don't make sense is hashed whole, so a bad read can only add a difference, never hide one.
std::vector<std::pair<uint32_t, uint32_t>> DeadStackRanges(uint32_t runningSp) {
    std::vector<std::pair<uint32_t, uint32_t>> ranges;
    uint32_t running = 0;
    uint32_t thread = 0;
    if (!Memory::TryRead32(kRunningThreadAddr, running) || !Memory::TryRead32(kThreadListHeadAddr, thread)) {
        return ranges;
    }
    for (int i = 0; thread != 0 && i < 64; ++i) {
        if (!Memory::Contains(thread, kThreadBytes)) {
            break;
        }
        const uint32_t top = Memory::Read32(thread + kThreadStackTopOffset);
        const uint32_t bottom = Memory::Read32(thread + kThreadStackBottomOffset);
        const uint32_t sp = (thread == running && runningSp != 0) ? runningSp
                                                                   : Memory::Read32(thread + kThreadSavedSpOffset);
        if (bottom != 0 && bottom + 4 < sp && sp <= top && top - bottom <= kMaxStackBytes &&
            Memory::Contains(bottom, top - bottom)) {
            ranges.emplace_back(bottom + 4, sp);
        }
        thread = Memory::Read32(thread + kThreadListNextOffset);
    }
    std::sort(ranges.begin(), ranges.end());
    return ranges;
}

// XXH3 of [base, base + bytes) without the given ranges (sorted; parts outside are ignored).
uint64_t HashRangeSkipping(uint32_t base, size_t bytes, const std::vector<std::pair<uint32_t, uint32_t>>& skip) {
    if (!Memory::Contains(base, bytes)) {
        return 0;
    }
    XXH3_state_t* state = XXH3_createState();
    if (state == nullptr) {
        return 0;
    }
    XXH3_64bits_reset(state);
    const uint64_t end = static_cast<uint64_t>(base) + bytes;
    uint64_t at = base;
    for (const auto& [from, to] : skip) {
        const uint64_t lo = std::max<uint64_t>(from, at);
        const uint64_t hi = std::min<uint64_t>(to, end);
        if (lo >= hi) {
            continue;
        }
        XXH3_64bits_update(state, Memory::GetPointer(static_cast<uint32_t>(at), static_cast<size_t>(lo - at)),
                           static_cast<size_t>(lo - at));
        at = hi;
    }
    XXH3_64bits_update(state, Memory::GetPointer(static_cast<uint32_t>(at), static_cast<size_t>(end - at)),
                       static_cast<size_t>(end - at));
    const uint64_t digest = XXH3_64bits_digest(state);
    XXH3_freeState(state);
    return digest;
}

void WriteImage(const std::filesystem::path& path, uint32_t base, size_t bytes) {
    if (!Memory::Contains(base, bytes)) {
        return;
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(Memory::GetPointer(base, bytes)), static_cast<std::streamsize>(bytes));
}

} // namespace

Digest HashGuestMemory() {
    return {HashRange(kMem1Base, kMem1Bytes), HashRange(kMem2Base, kMem2Bytes)};
}

void OnRetrace(uint32_t retraceCount, uint32_t runningSp) {
    const Settings& settings = GetSettings();
    if (!settings.hashFile.empty() && retraceCount % settings.every == 0) {
        static std::ofstream file = [&] {
            std::error_code ec;
            std::filesystem::create_directories(settings.hashFile.parent_path(), ec);
            return std::ofstream(settings.hashFile, std::ios::trunc);
        }();
        if (file) {
            const Digest digest = settings.skipDeadStack
                ? Digest{HashRangeSkipping(kMem1Base, kMem1Bytes, DeadStackRanges(runningSp)),
                         HashRangeSkipping(kMem2Base, kMem2Bytes, DeadStackRanges(runningSp))}
                : HashGuestMemory();
            char line[96];
            std::snprintf(line, sizeof(line), "%u %llu %016llx %016llx\n", retraceCount,
                          static_cast<unsigned long long>(DetClock::Now()),
                          static_cast<unsigned long long>(digest.mem1), static_cast<unsigned long long>(digest.mem2));
            file << line << std::flush;
        }
    }
    if (settings.dumps.count(retraceCount) != 0) {
        std::error_code ec;
        std::filesystem::create_directories(settings.dumpDirectory, ec);
        const std::string stem = "mem_r" + std::to_string(retraceCount);
        WriteImage(settings.dumpDirectory / (stem + "_mem1.bin"), kMem1Base, kMem1Bytes);
        WriteImage(settings.dumpDirectory / (stem + "_mem2.bin"), kMem2Base, kMem2Bytes);
        RT_LOGF(RT_TAG_RUNTIME, "determinism diagnostics: memory images at retrace %u\n", retraceCount);
    }
}

} // namespace DetHash
