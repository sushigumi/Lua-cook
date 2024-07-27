#pragma once

#include <lua.h>

#include <filesystem>
#include <string>

namespace luacook {

class Logger;

class Executor {
 public:
  Executor(const Logger& logger);

  // Initialize the executor and creates a new lua state as the execution
  // context.
  bool initialize();

  // Terminates the executor and closes the lua state.
  void terminate();

  bool execute(const std::string& script);

  // Execute the lua file that exists at the specified path.
  bool execute(const std::filesystem::path& path);

 private:
  int pcall(int narg, int nres);

  int report(int status);

  const Logger& logger_;

  lua_State* state_ = nullptr;
};

}  // namespace luacook
