#ifndef EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
#define EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
#include <chrono>

#include "Component.hpp"
#include "SwapchainManager.hpp"

namespace RtEngine {
    class CyclicalCellularAutomatonRenderer;

    class CyclicalCellularAutomaton : public Component {
    public:
        CyclicalCellularAutomaton() = default;
        CyclicalCellularAutomaton(const std::shared_ptr<EngineContext>& context,
                                   const std::shared_ptr<Node>& node)
            : Component(context, node) {}

        static inline const std::string COMPONENT_NAME = "CyclicalCellularAutomaton";

        void OnStart() override;
        void OnRender(DrawContext&) override {}
        void OnUpdate() override;
        void OnDestroy() override;

        void initProperties(const std::shared_ptr<IProperties>& config,
                            const UpdateFlagsHandle& update_flags) override;

    private:
        static constexpr uint32_t DEFAULT_THRESHOLD = 1;
        static constexpr double UPDATE_INTERVAL_SECONDS = 1.0 / 30.0;

        std::shared_ptr<CyclicalCellularAutomatonRenderer> renderer;
        std::chrono::steady_clock::time_point last_update;

        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // RtEngine

#endif //EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
