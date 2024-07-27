#include "lua-cook/executor.h"

#include <lauxlib.h>
#include <lualib.h>

#include "lua-cook/logger.h"

using namespace luacook;

Executor::Executor(const Logger& logger) : logger_(logger) {}

bool Executor::initialize() {
  state_ = luaL_newstate();
  if (state_ == nullptr) {
    logger_.error();
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

namespace {
int msghandler(lua_State* L) {
  const char* msg = lua_tostring(L, 1);
  if (msg == nullptr) {
    if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING) {
      return 1;
    } else {
      msg = lua_pushfstring(L, "(error object is a %s value)",
                            luaL_typename(L, 1));
    }
  }
  luaL_traceback(L, L, msg, 1);
  return 1;
}
}  // namespace

int Executor::pcall(int narg, int nres) {
  int base = lua_gettop(state_) - narg;   // Function index
  lua_pushcfunction(state_, msghandler);  // Push message handler
  lua_insert(state_, base);               // Put it under function and args
  int status = lua_pcall(state_, narg, nres, base);
  lua_remove(state_, base);  // Remove message handler from the stack
  return status;
}

int Executor::report(int status) {
  if (status != LUA_OK) {
    const char* msg = lua_tostring(state_, -1);
    if (msg == nullptr) {
      msg = "(error message not a string)";
    }
    logger_.error();
    lua_pop(state_, 1);  // Remove message
  }

  return status;
}

bool Executor::execute(const std::filesystem::path& path) {
  const std::string fname = path.string();
  int status = luaL_loadfile(state_, fname.c_str());
  if (status == LUA_OK) {
    status = pcall(0, LUA_MULTRET);
  }

  return report(status);
}
