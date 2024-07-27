#include <iostream>

#include "lua-cook/execution_items.h"
#include "lua-cook/executor.h"
#include "lua-cook/logger.h"

using namespace luacook;

class SimpleLogger : public Logger {
 public:
  void error(const std::string& msg) const override {
    std::cerr << msg << "\n";
  }
  void info() const override {}
  void result() const override {}
};

int main(int argc, char** argv) {
  SimpleLogger logger;
  Executor executor(logger);
  if (!executor.initialize()) {
    return 1;
  }

  if (argc == 1) {
    executor << ReadEvalPrintLoop();
  } else {
    // executor.execute(std::filesystem::path(argv[1]));
  }

  executor.terminate();

  return 0;
}
