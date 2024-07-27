#pragma once

#include <string>

struct lua_State;

namespace luacook {

class Logger;

class ExecutionItem {
 public:
  virtual int execute(lua_State* L, const Logger& logger) = 0;
};

class ReadEvalPrintLoop : public ExecutionItem {
 public:
  int execute(lua_State* L, const Logger& logger) override;

 private:
  std::string chunk_;
};

}  // namespace luacook
