#ifndef EDNA_ENGINE_IRUNNER_HPP
#define EDNA_ENGINE_IRUNNER_HPP
#include "ISerializable.hpp"
#include "UpdateFlagValue.hpp"

namespace RtEngine {
    class IRunner : public ISerializable {
    public:
        ~IRunner() override = default;

        virtual bool isRunning() const = 0;
        virtual void renderScene() = 0;
        virtual void setUpdateFlags(const UpdateFlagsHandle &new_flags) const = 0;
    };
} // RtEngine

#endif //EDNA_ENGINE_IRUNNER_HPP
