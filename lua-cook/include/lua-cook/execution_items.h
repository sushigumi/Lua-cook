#pragma once

#include <string>

struct lua_State;

namespace luacook {

class Logger;

class ExecutionItem {
 public:
  virtual bool execute(lua_State* L, const Logger& logger) = 0;
};

class ExecuteScript : public ExecutionItem {
 public:
  ExecuteScript(const std::string& script);

  // Execute the item.
  bool execute(lua_State* L, const Logger& logger) override;

 private:
  std::string script_;
};

class DoReadEvalPrintLoop : public ExecutionItem {
 public:
  // Execute the item.
  bool execute(lua_State* L, const Logger& logger) override;

 private:
  std::string chunk_;
};

}  // namespace luacook
