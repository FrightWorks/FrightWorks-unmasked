#pragma once

#include <cstdint>

// Determinism diagnostics for the lockstep mode (det_clock.h): fingerprints of guest memory taken at
// VI retraces, so two runs, or two devices, can be compared frame by frame.
//
//   NO100F_DET_HASH=<file>[:<every>]   append "<retrace> <ticks> <mem1> <mem2>" every <every>
//                                      retraces (default 60) to <file>
//   NO100F_DET_DUMP=<retrace>[,...]    write mem1/mem2 images at those retraces next to the hash file
//                                      (or in the log directory), for diffing two runs
//   NO100F_DET_HASH_SKIP_DEAD_STACK=1  leave each guest thread's unused stack (below its SP) out of the
//                                      hashes, for comparing two builds (tools/det-check.py)
//
// On Android the variables come from the debug.no100f.dethash / debug.no100f.detdump properties.
namespace DetHash {

// XXH3 of MEM1 (24 MiB) and the 64 MiB of MEM2 the game can use.
struct Digest {
    uint64_t mem1 = 0;
    uint64_t mem2 = 0;
};
Digest HashGuestMemory();

// Called at every VI retrace in deterministic mode, before guest callbacks run. `runningSp` is the
// running guest thread's stack pointer (0 if unknown).
void OnRetrace(uint32_t retraceCount, uint32_t runningSp);

} // namespace DetHash
