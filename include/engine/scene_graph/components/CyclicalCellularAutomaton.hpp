#ifndef EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
#define EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
#include <chrono>
#include <string>

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
        static constexpr float DEFAULT_UPDATE_CHANCE = 1;
        static constexpr float DEFAULT_MUTATION_CHANCE = 0.01f;
        static constexpr double UPDATE_INTERVAL_SECONDS = 1.0 / 30.0;
        static constexpr uint32_t DEFAULT_NEIGHBORHOOD_SIZE = 1;
        static constexpr uint32_t MAX_NEIGHBORHOOD_SIZE = 6;
        static inline const std::string DEFAULT_PALETTE_NAME = "Sunburn";
        static inline const std::string DEFAULT_NEIGHBORHOOD_SHAPE = "Box";

        uint32_t threshold = DEFAULT_THRESHOLD;
        float update_chance = DEFAULT_UPDATE_CHANCE;
        float mutation_chance = DEFAULT_MUTATION_CHANCE;
        std::string palette_name = DEFAULT_PALETTE_NAME;
        std::string applied_palette_name;
        std::string neighborhood_shape = DEFAULT_NEIGHBORHOOD_SHAPE;
        std::string applied_neighborhood_shape;
        uint32_t neighborhood_size = DEFAULT_NEIGHBORHOOD_SIZE;
        uint32_t applied_neighborhood_size = 0;

        std::shared_ptr<CyclicalCellularAutomatonRenderer> renderer;
        std::chrono::steady_clock::time_point last_update;

        SwapchainManager::RecreateCallbackHandle resize_callback_handle = 0;
    };
} // RtEngine

#endif //EDNA_ENGINE_CYCLICALCELLULARAUTOMATON_HPP
