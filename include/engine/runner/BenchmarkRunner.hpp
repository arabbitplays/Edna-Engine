#ifndef BENCHMARKRENDERER_HPP
#define BENCHMARKRENDERER_HPP
#include <../rendering/renderer/RaytracingRenderer.hpp>

#include "Runner.hpp"

namespace RtEngine {
	class BenchmarkRunner : public Runner {
	public:
		BenchmarkRunner(const std::shared_ptr<EngineContext> &engine_context, const std::shared_ptr<SceneManager> &scene_manager);

		void loadScene(const std::string &scene_path) override;
		void renderScene() override;
		void drawFrame(const std::shared_ptr<DrawContext> &draw_context) override;
		//void initProperties() override;

	private:
		void prepareFrame(const std::shared_ptr<DrawContext> &draw_context, uint32_t frame_idx) override;

		std::string getTmpImagePath(uint32_t samples);
		std::string getOutputFilePath();
		std::string getRefFilePath();
		void clearTmpfolder();

		void outputBenchmarkDataToCsv();

		static float calculateMSE(const uint8_t *ref_data, const uint8_t *data, uint32_t size);

		std::string TMP_FOLDER = "./tmp";
		std::string OUT_FOLDER = "../resources/benchmarks";
		std::string REF_FOLDER = "../resources/references";

		std::shared_ptr<DrawContext> draw_context;

		uint32_t error_calculation_sample_count = 1;
		uint32_t final_sample_count = 1 << 12;
	};

} // namespace RtEngine
#endif // BENCHMARKRENDERER_HPP
