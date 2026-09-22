#ifndef EDNA_ENGINE_EASINGFUNCTION_HPP
#define EDNA_ENGINE_EASINGFUNCTION_HPP

#include <cmath>
#include <numbers>

namespace Animation
{
    class EasingFunction
    {
    public:
        virtual ~EasingFunction() = default;
        virtual float apply(float x) const = 0;
    };

    class Linear : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            return x;
        }
    };

    // -------- Cubic --------

    class EaseInCubic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            return x * x * x;
        }
    };

    class EaseOutCubic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            const float u = 1.0f - x;
            return 1.0f - u * u * u;
        }
    };

    class EaseInOutCubic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            if (x < 0.5f)
                return 4.0f * x * x * x;

            const float u = -2.0f * x + 2.0f;
            return 1.0f - 0.5f * u * u * u;
        }
    };

    // -------- Elastic --------

    class EaseInElastic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            if (x <= 0.0f)
                return 0.0f;
            if (x >= 1.0f)
                return 1.0f;

            constexpr float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
            return -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * c4);
        }
    };

    class EaseOutElastic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            if (x <= 0.0f)
                return 0.0f;
            if (x >= 1.0f)
                return 1.0f;

            constexpr float c4 = (2.0f * std::numbers::pi_v<float>) / 3.0f;
            return std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
        }
    };

    class EaseInOutElastic : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            if (x <= 0.0f)
                return 0.0f;
            if (x >= 1.0f)
                return 1.0f;

            constexpr float c5 = (2.0f * std::numbers::pi_v<float>) / 4.5f;
            if (x < 0.5f)
                return -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) * 0.5f;

            return (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * c5)) * 0.5f + 1.0f;
        }
    };

    // -------- Bounce --------

    inline float outBounce(float x)
    {
        constexpr float n1 = 7.5625f;
        constexpr float d1 = 2.75f;

        if (x < 1.0f / d1)
            return n1 * x * x;

        if (x < 2.0f / d1)
        {
            x -= 1.5f / d1;
            return n1 * x * x + 0.75f;
        }

        if (x < 2.5f / d1)
        {
            x -= 2.25f / d1;
            return n1 * x * x + 0.9375f;
        }

        x -= 2.625f / d1;
        return n1 * x * x + 0.984375f;
    }

    class EaseInBounce : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            return 1.0f - outBounce(1.0f - x);
        }
    };

    class EaseOutBounce : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            return outBounce(x);
        }
    };

    class EaseInOutBounce : public EasingFunction
    {
    public:
        float apply(float x) const override
        {
            if (x < 0.5f)
                return (1.0f - outBounce(1.0f - 2.0f * x)) * 0.5f;

            return (1.0f + outBounce(2.0f * x - 1.0f)) * 0.5f;
        }
    };
} // namespace Animation

#endif // EDNA_ENGINE_EASINGFUNCTION_HPP
