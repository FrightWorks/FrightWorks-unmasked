// The F8 dev menu's interface (no100f_devmenu.h, shared with FrightWorks-no100f), without its menu: FrightWorks-no100f's
// dev menu is built on that game's scenes, positions and memory, none of which are mapped for Scooby-Doo! Unmasked yet.
// Everything reports "off", so the overlay and the F3 recorder behave as with developer tools disabled.
#include "no100f_devmenu.h"

namespace no100f_devmenu {

bool Enabled() noexcept { return false; }
void HandleSdlEvent(const SDL_Event& event, bool settingsOpen) noexcept {
    (void)event;
    (void)settingsOpen;
}
void Draw() noexcept {}
bool IsOpen() noexcept { return false; }
std::string WhereText() { return "position not available in this game yet"; }
bool BlocksInput() noexcept { return false; }
void ApplyFreecam(uint32_t camera, CpuContext* ctx) {
    (void)camera;
    (void)ctx;
}
uint16_t PressedButtons() noexcept { return 0; }

} // namespace no100f_devmenu
