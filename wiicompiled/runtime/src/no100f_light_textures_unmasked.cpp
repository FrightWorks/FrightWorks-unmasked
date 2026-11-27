// Bloom's light list (no100f_light_textures.h, shared with FrightWorks-no100f). Bloom glows only on draws that sample a
// listed texture, and Scooby-Doo! Unmasked's light textures are not listed yet, so the list is empty and bloom shows
// nothing here. To add some: NO100F_TEXLOG / the dev menu's texture tools in no100f find them by pack name.
#include "no100f_light_textures.h"

#include <aurora/aurora.h>

namespace no100f_light_textures {

std::vector<std::string>& List() {
    static std::vector<std::string> list;
    return list;
}

void Apply() {}

} // namespace no100f_light_textures
