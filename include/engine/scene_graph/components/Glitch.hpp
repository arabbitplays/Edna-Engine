#ifndef EDNA_ENGINE_GLITCH_HPP
#define EDNA_ENGINE_GLITCH_HPP
#include <memory>
#include <string>

#include "Component.hpp"

namespace RtEngine {
    class GlitchRenderer;

    class Glitch : public Component {
    public:
        Glitch();
        Glitch(const std::shared_ptr<EngineContext>& context,
               const std::shared_ptr<Node>& node);
        ~Glitch() override;

        static inline const std::string COMPONENT_NAME = "Glitch";

        void OnStart() override {}
        void OnRender(DrawContext&) override {}
        void OnUpdate() override;
        void OnDestroy() override {}

        void initProperties(const std::shared_ptr<IProperties>& config,
                            const UpdateFlagsHandle& update_flags) override;

        std::shared_ptr<GlitchRenderer> getRenderer() const { return renderer; }
        std::shared_ptr<ImageConnector> getOutputConnector() const override;

    private:
        bool tryInitialize();

        std::string source_node = "Composition";

        float shake_power      = 0.03f;
        float shake_rate       = 0.2f;
        float shake_speed      = 5.0f;
        float shake_block_size = 30.5f;
        float shake_color_rate = 0.01f;

        std::shared_ptr<GlitchRenderer> renderer;
    };
} // RtEngine

#endif //EDNA_ENGINE_GLITCH_HPP
