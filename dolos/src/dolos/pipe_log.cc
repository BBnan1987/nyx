#include "dolos/pipe_log.h"

#include <spdlog/pattern_formatter.h>

namespace dolos {

static std::shared_ptr<pipe_sink_mt> g_pipe_sink;

bool InitializePipeLog(const std::string& logger_name) {
  g_pipe_sink = std::make_shared<pipe_sink_mt>();
  bool connected = g_pipe_sink->connect();

  auto logger = std::make_shared<spdlog::logger>(logger_name, g_pipe_sink);
  logger->set_pattern("[%l] %v");
  logger->set_level(spdlog::level::trace);
  logger->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(logger);

  return connected;
}

void ShutdownPipeLog() {
  if (auto logger = spdlog::default_logger()) {
    logger->flush();
    spdlog::drop(logger->name());
  }

  if (g_pipe_sink) {
    g_pipe_sink->disconnect();
    g_pipe_sink.reset();
  }
}

bool IsPipeLogConnected() {
  return g_pipe_sink && g_pipe_sink->is_connected();
}

}  // namespace dolos
