#pragma once

#include <iterator>
#include <string>

// Who made FrightWorks: UnMasked, as the game shows it (F10 About, the pill on the game's credits). The interface is
// FrightWorks-no100f's (shared overlay code reads it). Keep it in step with the README's Credits.
namespace no100f_credits {

inline constexpr const char* kAuthor = "BitsofBitsofBits and LoneRanger419";
// The lines under the author on the credits pill.
inline constexpr const char* kHelpers[] = {"JosiahG (original port)"};

// "Made by BitsofBitsofBits and LoneRanger419, from JosiahG's original port"
inline std::string Line() { return std::string("Made by ") + kAuthor + ", from JosiahG's original port"; }

} // namespace no100f_credits
