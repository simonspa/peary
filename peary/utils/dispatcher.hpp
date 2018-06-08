// Copyright 2018 Moritz Kiehn
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file
/// \brief   Command dispatcher that takes function names and string arguments
/// \author  Moritz Kiehn <msmk@cern.ch>
/// \date    2018-02-20

#pragma once

#include <cassert>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace caribou {

  /// A simple command dispatcher.
  ///
  /// You can register commands and call them by name using string arguments.
  class Dispatcher {
  public:
    /// Internally functions take variable string arguments and return a string.
    using NativeInterface = std::function<std::string(const std::vector<std::string>&)>;

    /// Register a command that implements the native dispatcher call interface.
    void add(std::string name, NativeInterface func, std::size_t nargs);
    /// Register a command with arbitrary arguments.
    ///
    /// All argument types must be constructible via the
    /// `std::istream& operator>>(...)` formatted input operator and the return
    /// type must be support the `std::ostream& operator<<(...)` formatted output
    /// operator.
    template <typename R, typename... Args> void add(std::string name, std::function<R(Args...)> func);
    template <typename R, typename... Args> void add(std::string name, R (*func)(Args...));
    template <typename T, typename R, typename... Args> void add(std::string name, R (T::*member_func)(Args...), T* t);

    /// Call a command with some arguments.
    std::string call(const std::string& name, const std::vector<std::string>& args);

    /// Return a list of registered commands and required number of arguments.
    std::vector<std::pair<std::string, std::size_t>> commands() const;

  private:
    struct Command {
      NativeInterface func;
      std::size_t nargs;
    };
    std::unordered_map<std::string, Command> m_commands;
  };

  // implementations

  inline void Dispatcher::add(std::string name, Dispatcher::NativeInterface func, std::size_t nargs) {
    if(name.empty()) {
      throw std::invalid_argument("Can not register command with empty name");
    }
    if(m_commands.count(name)) {
      throw std::invalid_argument("Can not register command '" + name + "' more than once");
    }
    m_commands[std::move(name)] = Command{func, nargs};
  }

  namespace dispatcher_impl {
    namespace {

      template <typename T> inline T str_decode(const std::string& str) {
        T tmp;
        std::istringstream is(str);
        is >> tmp;
        if(is.fail()) {
          std::string msg;
          msg += "Could not convert value '";
          msg += str;
          msg += "' to type '";
          msg += typeid(T).name();
          msg += "'";
          throw std::invalid_argument(std::move(msg));
        }
        return tmp;
      }

      template <typename T> inline std::string str_encode(const T& value) {
        std::ostringstream os;
        os << value;
        if(os.fail()) {
          std::string msg;
          msg += "Could not convert type '";
          msg += typeid(T).name();
          msg += "' to std::string";
          throw std::invalid_argument(std::move(msg));
        }
        return os.str();
      }

      // Wrap a function that returns a value
      template <typename R, typename... Args> struct NativeInterfaceWrappper {
        std::function<R(Args...)> func;

        std::string operator()(const std::vector<std::string>& args) {
          return decode_and_call(args, std::index_sequence_for<Args...>{});
        }
        template <std::size_t... I>
        std::string decode_and_call(const std::vector<std::string>& args, std::index_sequence<I...>) {
          return str_encode(func(str_decode<typename std::decay<Args>::type>(args.at(I))...));
        }
      };

      // Wrap a function that does not return anything
      template <typename... Args> struct NativeInterfaceWrappper<void, Args...> {
        std::function<void(Args...)> func;

        std::string operator()(const std::vector<std::string>& args) {
          return decode_and_call(args, std::index_sequence_for<Args...>{});
        }
        template <std::size_t... I>
        std::string decode_and_call(const std::vector<std::string>& args, std::index_sequence<I...>) {
          func(str_decode<typename std::decay<Args>::type>(args.at(I))...);
          return std::string();
        }
      };

      template <typename R, typename... Args>
      inline Dispatcher::NativeInterface make_native_interface(std::function<R(Args...)>&& function) {
        return NativeInterfaceWrappper<R, Args...>{std::move(function)};
      }

    } // namespace
  }   // namespace dispatcher_impl

  template <typename R, typename... Args> inline void Dispatcher::add(std::string name, std::function<R(Args...)> func) {
    m_commands[std::move(name)] = Command{dispatcher_impl::make_native_interface(std::move(func)), sizeof...(Args)};
  }

  template <typename R, typename... Args> inline void Dispatcher::add(std::string name, R (*func)(Args...)) {
    assert(func && "Function pointer must be non-null");
    add(std::move(name), std::function<R(Args...)>(func));
  }

  template <typename T, typename R, typename... Args>
  inline void Dispatcher::add(std::string name, R (T::*member_func)(Args...), T* t) {
    assert(member_func && "Member function pointer must be non-null");
    assert(t && "Object pointer must be non-null");
    add(std::move(name), std::function<R(Args...)>([=](Args... args) { return (t->*member_func)(args...); }));
  }

  inline std::string Dispatcher::call(const std::string& name, const std::vector<std::string>& args) {
    auto cmd = m_commands.find(name);
    if(cmd == m_commands.end()) {
      throw std::invalid_argument("Unknown command '" + name + "'");
    }
    if(args.size() != cmd->second.nargs) {
      throw std::invalid_argument("Command '" + name + "' expects " + std::to_string(cmd->second.nargs) + " arguments but " +
                                  std::to_string(args.size()) + " given");
    }
    return cmd->second.func(args);
  }

  inline std::vector<std::pair<std::string, std::size_t>> Dispatcher::commands() const {
    std::vector<std::pair<std::string, std::size_t>> cmds;

    for(const auto& cmd : m_commands) {
      cmds.emplace_back(cmd.first, cmd.second.nargs);
    }
    return cmds;
  }

} // namespace caribou
