#ifndef EDNA_ENGINE_COLORPALETTENAME_HPP
#define EDNA_ENGINE_COLORPALETTENAME_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace color
{
#define COLOR_PALETTES(X)                                                                                              \
    X(Fire)                                                                                                            \
    X(Ocean)                                                                                                           \
    X(Rainbow)                                                                                                         \
    X(Nature)                                                                                                          \
    X(Sunburn)                                                                                                         \
    X(Twilight)                                                                                                        \
    X(Berry)                                                                                                           \
    X(Neon)

    class ColorPaletteName
    {
    public:
        enum Value
        {
#define COLOR_PALETTE_ENUM(name) name,
            COLOR_PALETTES(COLOR_PALETTE_ENUM)
#undef COLOR_PALETTE_ENUM
        };

        constexpr ColorPaletteName() = default;
        constexpr ColorPaletteName(Value v) : value_(v)
        {
        }
        constexpr operator Value() const
        {
            return value_;
        }

        constexpr std::string_view toString() const
        {
            return ALL_NAMES[static_cast<std::size_t>(value_)];
        }

        static constexpr ColorPaletteName fromString(std::string_view name, ColorPaletteName fallback)
        {
            for (std::size_t i = 0; i < ALL_NAMES.size(); ++i)
            {
                if (ALL_NAMES[i] == name)
                {
                    return static_cast<Value>(i);
                }
            }
            return fallback;
        }

        static std::vector<std::string> getAllNames()
        {
            std::vector<std::string> result;
            result.reserve(ALL_NAMES.size());
            for (const std::string_view name : ALL_NAMES)
                result.emplace_back(name);
            return result;
        }

    private:
        static constexpr std::array ALL_NAMES = {
#define COLOR_PALETTE_STR(name) std::string_view{#name},
            COLOR_PALETTES(COLOR_PALETTE_STR)
#undef COLOR_PALETTE_STR
        };

        Value value_ = Fire;
    };
} // namespace color

#endif // EDNA_ENGINE_COLORPALETTENAME_HPP
