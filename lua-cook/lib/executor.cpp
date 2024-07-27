#include "lua-cook/executor.h"

#include <lauxlib.h>
#include <lualib.h>

#include <iostream>
#include <string>

#include "lua-cook/execution_items.h"
#include "lua-cook/logger.h"

using namespace luacook;

Executor::Executor(const Logger& logger) : logger_(logger) {}

bool Executor::initialize() {
  state_ = luaL_newstate();
  if (state_ == nullptr) {
    logger_.error("Cannot create lua state: not enough memory.");
    return false;
  }

  // Stop GC while building state.
  lua_gc(state_, LUA_GCSTOP);
  luaL_checkversion(state_);

  // Open standard libraries.
  luaL_openlibs(state_);

  // Start GC in generational mode.
  lua_gc(state_, LUA_GCRESTART);
  lua_gc(state_, LUA_GCGEN, 0, 0);

  return true;
}

void Executor::terminate() {
  lua_close(state_);
  state_ = nullptr;
}

Executor& luacook::operator<<(Executor& executor, ExecutionItem&& item) {
  item.execute(executor.state_, executor.logger_);
  return executor;
}
