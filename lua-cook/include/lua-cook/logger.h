#pragma once

#include <memory>

namespace luacook {

class Logger {
 public:
  virtual void error(const std::string& msg) const = 0;
  virtual void info() const = 0;
  virtual void result() const = 0;
};

}  // namespace luacook
