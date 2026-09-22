#ifndef EDNA_ENGINE_LOGGINGCONFIGURATION_HPP
#define EDNA_ENGINE_LOGGINGCONFIGURATION_HPP
#include <logging/LogManager.hpp>
#include <logging/configuration/LogConfigurationBuilder.hpp>
#include <logging/targets/ConsoleTarget.hpp>

namespace RtEngine {
    class LoggingConfiguration {
    public:
        static void configure(bool verbose) {
            Logging::Severity min_severity = verbose ? Logging::TRACE : Logging::INFO;
            auto console_target = std::make_shared<Logging::ConsoleTarget>();
            auto log_config = Logging::LogConfigurationBuilder()
                .addRule("*", min_severity, console_target)
                .addTarget(console_target).build();
            Logging::LogManager::setLogConfiguration(log_config);
        }
    };
}

#endif //EDNA_ENGINE_LOGGINGCONFIGURATION_HPP
