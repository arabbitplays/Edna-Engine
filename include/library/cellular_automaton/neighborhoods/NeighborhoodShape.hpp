#ifndef EDNA_ENGINE_NEIGHBORHOODSHAPE_HPP
#define EDNA_ENGINE_NEIGHBORHOODSHAPE_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace cellular_automaton
{
#define CELLULAR_AUTOMATON_NEIGHBORHOOD_SHAPES(X) \
    X(Box)                                        \
    X(Diamond)

    class NeighborhoodShape
    {
    public:
        enum Value
        {
#define CELLULAR_AUTOMATON_SHAPE_ENUM(name) name,
            CELLULAR_AUTOMATON_NEIGHBORHOOD_SHAPES(CELLULAR_AUTOMATON_SHAPE_ENUM)
#undef CELLULAR_AUTOMATON_SHAPE_ENUM
        };

        constexpr NeighborhoodShape() = default;
        constexpr NeighborhoodShape(Value v) : value_(v) {}
        constexpr operator Value() const { return value_; }

        constexpr std::string_view toString() const
        {
            return ALL_NAMES[static_cast<std::size_t>(value_)];
        }

        static constexpr NeighborhoodShape fromString(std::string_view name,
                                                      NeighborhoodShape fallback)
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
#define CELLULAR_AUTOMATON_SHAPE_STR(name) std::string_view{#name},
            CELLULAR_AUTOMATON_NEIGHBORHOOD_SHAPES(CELLULAR_AUTOMATON_SHAPE_STR)
#undef CELLULAR_AUTOMATON_SHAPE_STR
        };

        Value value_ = Box;
    };
}

#endif //EDNA_ENGINE_NEIGHBORHOODSHAPE_HPP
