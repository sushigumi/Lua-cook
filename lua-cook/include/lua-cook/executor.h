#pragma once

#include <lua.h>

#include <filesystem>
#include <string>

namespace luacook {

class Logger;
class ExecutionItem;

class Executor {
 public:
  Executor(const Logger& logger);

  // Initialize the executor and creates a new lua state as the execution
  // context.
  bool initialize();

  // Terminates the executor and closes the lua state.
  void terminate();

  friend Executor& operator<<(Executor& executor, ExecutionItem&& item);

 private:
  const Logger& logger_;
  lua_State* state_ = nullptr;
};

}  // namespace luacook
