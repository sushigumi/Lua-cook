#include "lua-cook/executor.h"
#include "lua-cook/logger.h"

using namespace luacook;

class SimpleLogger : public Logger {
 public:
  void error() const override {}
  void info() const override {}
  void result() const override {}
};

int main(int argc, char** argv) {
  SimpleLogger logger;
  Executor executor(logger);
  if (!executor.initialize()) {
    return 1;
  }

  executor.execute(std::filesystem::path(argv[1]));

  executor.terminate();

  return 0;
}
