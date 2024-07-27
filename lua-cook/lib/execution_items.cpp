#include "lua-cook/execution_items.h"

#include <lauxlib.h>
#include <lua.h>

#include <iostream>

#include "lua-cook/logger.h"

using namespace luacook;

namespace {

int msghandler(lua_State* L) {
  const char* msg = lua_tostring(L, 1);
  if (msg == nullptr) {
    if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING) {
      return 1;
    } else {
      // TODO: Pushing a string means that the original error object still
      // exists on the stack. Does it need to be removed?
      msg = lua_pushfstring(L, "(error object is a %s value)",
                            luaL_typename(L, 1));
    }
  }

  luaL_traceback(L, L, msg, 1);

  return 1;
}

class Context {
 public:
  Context(lua_State* L, const Logger& logger) : L_(L), logger_(logger) {}

  // Perform a protected call.
  int pcall(int narg, int nres) {
    int base = lua_gettop(L_) - narg;   // function index
    lua_pushcfunction(L_, msghandler);  // push message handler
    lua_insert(L_, base);               // put it under function and args
    int status = lua_pcall(L_, narg, nres, base);
    lua_remove(L_, base);  // remove message handler

    return status;
  }

  // Get the error message on top of the stack and log it.
  int report(int status) {
    if (status != LUA_OK) {
      const char* msg = lua_tostring(L_, -1);
      if (msg == nullptr) {
        msg = "(error message not a string)";
      }

      logger_.error(msg);
      lua_pop(L_, 1);  // remove message
    }

    return status;
  }

  // Print items on the stack if it has less than `LUA_MINSTACK` items.
  void printStack() {
    int n = top();
    if (n > 0) {
      luaL_checkstack(L_, LUA_MINSTACK, "too many results to print");
      lua_getglobal(L_, "print");
      lua_insert(L_, 1);
      if (lua_pcall(L_, n, 0, 0) != LUA_OK) {
        logger_.error("error calling 'print'");
      }
    }
  }

  // Load (compile) the specified buffer.
  int loadBuffer(const std::string& buffer, const std::string& name) {
    return luaL_loadbuffer(L_, buffer.c_str(), buffer.size(), name.c_str());
  }

  // Load (compile) the specified file.
  int loadFile(const std::string& fname) {
    return luaL_loadfile(L_, fname.c_str());
  }

  // Push a string to the top of the stack.
  const char* push(const std::string& str) {
    return lua_pushstring(L_, str.c_str());
  }

  // Get the string at the specified index from the stack.
  std::string getString(int index) { return lua_tostring(L_, index); }

  // Pop n elements off the stack.
  void pop(int n) { lua_pop(L_, n); }

  // Return the index of the top element in the stack.
  int top() { return lua_gettop(L_); }

 private:
  lua_State* L_;
  const Logger& logger_;
};

const char* get_prompt(bool firstline) {
  constexpr auto PROMPT = "> ";
  constexpr auto PROMPT_CONT = ">> ";
  return firstline ? PROMPT : PROMPT_CONT;
}

}  // namespace

ExecuteScript::ExecuteScript(const std::string& script) : script_(script) {}

bool ExecuteScript::execute(lua_State* L, const Logger& logger) {
  Context context(L, logger);

  int status = context.loadFile(script_);
  if (status == LUA_OK) {
    status = context.pcall(0, LUA_MULTRET);
  }

  return status == LUA_OK;
}

bool DoReadEvalPrintLoop::execute(lua_State* L, const Logger& logger) {
  Context context(L, logger);

  int status;
  bool firstline = true;
  std::string line;
  for (;;) {
    // Clear the stack
    lua_settop(L, 0);

    // Print the prompt
    std::cout << get_prompt(firstline) << std::flush;

    // Read the line and push it onto the stack. If we reached the eof, exit.
    if (!std::getline(std::cin, line)) {
      status = LUA_OK;
      break;
    }

    // Append the line to the chunk.
    chunk_ += line;

    // Try to load the line as an expression
    if ((status = context.loadBuffer("return " + chunk_ + ";", "=stdin")) ==
        LUA_OK) {
      // Successfully parsed, and execute it.
      goto execute;
    } else {
      // Pop the error string. We don't care about this one.
      context.pop(1);
    }

    // It is a multiline statement. Append this line to the chunk and try to
    // load it as a statement.
    status = context.loadBuffer(chunk_, "=stdin");
    if (status == LUA_OK) {
      // Successfully parsed the chunk, pop the loaded line and execute it.
      goto execute;
    } else if (status == LUA_ERRSYNTAX) {
      // It could be an incomplete statement. Check whether the message at the
      // top of the stack ends with the <eof> mark for incomplete statements.
      const std::string errorMsg = context.getString(-1);
      if (errorMsg.ends_with("<eof>")) {
        // We want to keep parsing, skip execution and finalization.
        chunk_ += "\n";
        continue;
      }
    }

    // Skip execution as there is an error. Keep it on the stack so that we can
    // report it.
    goto finalize;

  execute:
    // Load the chunk and execute it.
    status = context.pcall(0, LUA_MULTRET);

  finalize:
    if (status == LUA_OK) {
      context.printStack();
    } else {
      context.report(status);
    }
    firstline = true;
    chunk_ = "";
  }

  return true;
}
