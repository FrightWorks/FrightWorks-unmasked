#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

// Scooby-Doo! Unmasked (G5DE78): where the game keeps what it is doing, for overlays that depend on it. The interface
// is FrightWorks-no100f's (shared overlay code reads it); this game's addresses are not mapped yet. Address 0 is outside
// guest RAM, so Memory::TryRead32 fails and the overlays that need it (the main-menu hotkey guide, the credits pill)
// stay hidden until the real addresses are filled in.
namespace no100f_game_state {

// Game mode: not mapped for Unmasked yet.
inline constexpr uint32_t kModeAddress = 0;
// Game state within the mode: not mapped for Unmasked yet.
inline constexpr uint32_t kStateAddress = 0;
// The open Bink movie, 0 when none plays: not mapped for Unmasked yet.
inline constexpr uint32_t kMovieAddress = 0;
// 1 while the player has control: not mapped for Unmasked yet.
inline constexpr uint32_t kPlayerControlAddress = 0;
inline constexpr uint32_t kModeInPlay = ~0u;
inline constexpr uint32_t kModePause = ~0u;

// Set only after the loader verifies a known layout, not merely an arbitrary build's generated hash.
inline std::atomic<bool> g_diagnosticLayoutVerified{false};

// The title screen and its menus. Never true until this game's mode and state are mapped.
constexpr bool IsMainMenu(uint32_t mode, uint32_t state, uint32_t movie) {
    (void)mode;
    (void)state;
    (void)movie;
    return false;
}

// The last Bink movie the game opened is a credits roll. Nothing sets it for Unmasked yet.
inline std::atomic<bool> g_creditsMovie{false};

// The file name of the last Bink movie the game opened, for the log's movie lines.
inline std::mutex g_lastMovieMutex;
inline std::string g_lastMovieName;
inline void SetLastMovieName(std::string name) {
    std::lock_guard<std::mutex> lock(g_lastMovieMutex);
    g_lastMovieName = std::move(name);
}
inline std::string LastMovieName() {
    std::lock_guard<std::mutex> lock(g_lastMovieMutex);
    return g_lastMovieName.empty() ? "?" : g_lastMovieName;
}

// File name -> is it one of the credits movies (any case).
inline bool IsCreditsMovie(const std::string_view name) {
    constexpr std::string_view prefix = "credits";
    if (name.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i)
        if ((name[i] | 0x20) != prefix[i]) return false;
    return true;
}

} // namespace no100f_game_state
