#ifndef RANDOMUTIL_HPP
#define RANDOMUTIL_HPP

#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

namespace RtEngine
{
    class RandomUtil
    {
    public:
        static uint32_t generateInt()
        {
            static std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
            return dist(generator());
        }

        template <typename T> static T randomInRange(T min, T max)
        {
            static_assert(std::is_arithmetic_v<T>, "randomInRange requires an arithmetic type");
            if constexpr (std::is_integral_v<T>)
            {
                std::uniform_int_distribution<T> dist(min, max);
                return dist(generator());
            }
            else
            {
                std::uniform_real_distribution<T> dist(min, max);
                return dist(generator());
            }
        }

    private:
        static std::mt19937& generator()
        {
            static std::mt19937 gen{std::random_device{}()};
            return gen;
        }
    };
} // namespace RtEngine

#endif // RANDOMUTIL_HPP
