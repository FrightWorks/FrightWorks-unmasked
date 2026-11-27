// AX's partial address setters (from FrightWorks-no100f #51, "repeating audio"): the SDK sets one sync bit per field
// (loop flag 0x2000, loop address 0x4000, end address 0x8000), but its consumer handles those bits before the full
// address reset (0x1000). A setter called while a full reset is pending then won over it, leaving the DSP on stale
// addresses. These keep the full reset pending instead
// (AxDspHle::PartialAddressSyncFlags), so it copies the latest values, including this edit.
// Scooby-Doo! Unmasked's AX matches Night of 100 Frights' here: the same voice offsets (0x1a6, 0x1aa, 0x1ae, sync
// flags at 0x1c) and bits, checked in the DOL. The game doesn't link AXSetVoiceCurrentAddr.
#include "ax_address_sync.h"
#include "hle_stubs.h"
#include "memory.h"

extern "C" int32_t OS__DisableInterrupts_801a65ac();
extern "C" int32_t OS__RestoreInterrupts_801a65d4(int32_t level);

namespace {
void SetAddress(uint32_t voice, uint32_t offset, uint32_t address, uint32_t field) {
    const int32_t level = OS__DisableInterrupts_801a65ac();
    Memory::Write16(voice + offset, static_cast<uint16_t>(address >> 16));
    Memory::Write16(voice + offset + 2u, static_cast<uint16_t>(address));
    const uint32_t flags = Memory::Read32(voice + 0x1cu);
    Memory::Write32(voice + 0x1cu, AxDspHle::PartialAddressSyncFlags(flags, field));
    OS__RestoreInterrupts_801a65d4(level);
}
}

extern "C" void AXSetVoiceLoop_Gc(uint32_t voice, uint32_t looping) {
    const int32_t level = OS__DisableInterrupts_801a65ac();
    Memory::Write16(voice + 0x1a6u, static_cast<uint16_t>(looping));
    const uint32_t flags = Memory::Read32(voice + 0x1cu);
    Memory::Write32(voice + 0x1cu, AxDspHle::PartialAddressSyncFlags(flags, 0x2000u));
    OS__RestoreInterrupts_801a65d4(level);
}
PPC_NATIVE_OVERRIDE_VOID(8015DF1C, AXSetVoiceLoop_Gc,
    (uint32_t voice, uint32_t looping), (voice, looping));

extern "C" void AXSetVoiceLoopAddr_Gc(uint32_t voice, uint32_t address) {
    SetAddress(voice, 0x1aau, address, 0x4000u);
}
PPC_NATIVE_OVERRIDE_VOID(8015DF68, AXSetVoiceLoopAddr_Gc,
    (uint32_t voice, uint32_t address), (voice, address));

extern "C" void AXSetVoiceEndAddr_Gc(uint32_t voice, uint32_t address) {
    SetAddress(voice, 0x1aeu, address, 0x8000u);
}
PPC_NATIVE_OVERRIDE_VOID(8015DFBC, AXSetVoiceEndAddr_Gc,
    (uint32_t voice, uint32_t address), (voice, address));
