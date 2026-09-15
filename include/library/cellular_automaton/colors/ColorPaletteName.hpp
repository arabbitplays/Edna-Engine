#ifndef EDNA_ENGINE_COLORPALETTENAME_HPP
#define EDNA_ENGINE_COLORPALETTENAME_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace cellular_automaton
{
#define CELLULAR_AUTOMATON_COLOR_PALETTES(X) \
    X(Fire)                                  \
    X(Ocean)                                 \
    X(Rainbow)                               \
    X(Nature)                                \
    X(Sunburn)                               \
    X(Twilight)                              \
    X(Berry)                                 \
    X(Neon)

    class ColorPaletteName
    {
    public:
        enum Value
        {
#define CELLULAR_AUTOMATON_PALETTE_ENUM(name) name,
            CELLULAR_AUTOMATON_COLOR_PALETTES(CELLULAR_AUTOMATON_PALETTE_ENUM)
#undef CELLULAR_AUTOMATON_PALETTE_ENUM
        };

        constexpr ColorPaletteName() = default;
        constexpr ColorPaletteName(Value v) : value_(v) {}
        constexpr operator Value() const { return value_; }

        constexpr std::string_view toString() const
        {
            return ALL_NAMES[static_cast<std::size_t>(value_)];
        }

        static constexpr ColorPaletteName fromString(std::string_view name,
                                                     ColorPaletteName fallback)
        {
            for (std::size_t i = 0; i < ALL_NAMES.size(); ++i) {
                if (ALL_NAMES[i] == name) {
                    return static_cast<Value>(i);
                }
            }
            return fallback;
        }

        static std::vector<std::string> getAllNames()
        {
            std::vector<std::string> result;
            result.reserve(ALL_NAMES.size());
            for (const std::string_view name : ALL_NAMES) result.emplace_back(name);
            return result;
        }

    private:
        static constexpr std::array ALL_NAMES = {
#define CELLULAR_AUTOMATON_PALETTE_STR(name) std::string_view{#name},
            CELLULAR_AUTOMATON_COLOR_PALETTES(CELLULAR_AUTOMATON_PALETTE_STR)
#undef CELLULAR_AUTOMATON_PALETTE_STR
        };

        Value value_ = Fire;
    };
}

#endif //EDNA_ENGINE_COLORPALETTENAME_HPP
