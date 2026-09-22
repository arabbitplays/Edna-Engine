#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

#include "Engine.hpp"
#include "LoggingConfiguration.hpp"
using namespace RtEngine;

int main(int argc, char *argv[]) {
	bool verbose = false;
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "-v") {
			verbose = true;
			break;
		}
	}
	LoggingConfiguration::configure(verbose);

	std::shared_ptr<Engine> engine = std::make_shared<Engine>();
	CliArguments args{argc, argv};
	try {
		engine->run(args);
	} catch (const std::exception &e) {
		Logging::LoggerHandle logger = Logging::LogManager::getClassLogger<Engine>();
		logger->error(e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
