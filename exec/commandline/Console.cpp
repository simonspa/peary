#include "Console.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include <cstdlib>
#include <readline/history.h>
#include <readline/readline.h>

namespace CppReadline {
  namespace {

    Console* currentConsole = nullptr;
    HISTORY_STATE* emptyHistory = history_get_history_state();

  } /* namespace  */

  struct Console::Impl {
    using RegisteredCommands = std::unordered_map<std::string, CppReadline::ConsoleCommand>;

    ::std::string greeting_;
    // These are hardcoded commands. They do not do anything and are catched manually in the executeCommand function.
    RegisteredCommands commands_;
    HISTORY_STATE* history_ = nullptr;

    Impl(::std::string const& greeting) : greeting_(greeting), commands_() {}
    ~Impl() { free(history_); }

    Impl(Impl const&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl const&) = delete;
    Impl& operator=(Impl&&) = delete;
  };

  // Here we set default commands, they do nothing since we quit with them
  // Quitting behaviour is hardcoded in readLine()
  Console::Console(std::string const& greeting) : pimpl_{new Impl{greeting}} {
    // Init readline basics
    rl_attempted_completion_function = &Console::getCommandCompletions;

    // These are default hardcoded commands.
    // Help command lists available commands.
    pimpl_->commands_["help"] = ConsoleCommand(
      [this](const Arguments&) {
        auto commands = getRegisteredCommands();
        std::cout << "Available commands are:\n";
        for(auto& command : commands)
          std::cout << "\t" << command << "\n";
        return ReturnCode::Ok;
      },
      "Provide help on using this console",
      0);

    // Run command executes all commands in an external file.
    pimpl_->commands_["run"] = ConsoleCommand(
      [this](const Arguments& input) { return executeFile(input.at(1)); }, "Run the provided script", 1, "SCRIPT_FILENAME");

    // Quit and Exit simply terminate the console.
    ConsoleCommand quit = ConsoleCommand([](const Arguments&) { return ReturnCode::Quit; }, "Quit the console", 0);
    pimpl_->commands_["quit"] = quit;
    pimpl_->commands_["exit"] = quit;
  }

  Console::~Console() = default;

  void Console::registerCommand(const std::string& s, ConsoleCommand f) { pimpl_->commands_[s] = f; }

  void Console::registerCommand(
    const std::string& s, CommandFunction f, std::string helptext, size_t numArgs, std::string arglist) {
    ConsoleCommand c = ConsoleCommand(f, helptext, numArgs, arglist);
    this->registerCommand(s, c);
  }

  std::vector<std::string> Console::getRegisteredCommands() const {
    std::vector<std::string> allCommands;
    for(auto& pair : pimpl_->commands_)
      allCommands.push_back(pair.first);

    return allCommands;
  }

  void Console::saveState() {
    free(pimpl_->history_);
    pimpl_->history_ = history_get_history_state();
  }

  void Console::reserveConsole() {
    if(currentConsole == this)
      return;

    // Save state of other Console
    if(currentConsole)
      currentConsole->saveState();

    // Else we swap state
    if(!pimpl_->history_)
      history_set_history_state(emptyHistory);
    else
      history_set_history_state(pimpl_->history_);

    // Tell others we are using the console
    currentConsole = this;
  }

  void Console::setGreeting(const std::string& greeting) { pimpl_->greeting_ = greeting; }

  std::string Console::getGreeting() const { return pimpl_->greeting_; }

  std::vector<std::string> Console::split(std::string str, std::string delims) {

    // If the input string is empty, simply return empty container
    if(str.empty()) {
      return std::vector<std::string>();
    }

    // Else we have data, clear the default elements and chop the string:
    std::vector<std::string> elems;

    // Add the string identifiers as special delimiters
    delims += "\'\"";

    // Loop through the string
    std::size_t prev = 0, sprev = 0, pos;
    char ins = 0;
    while((pos = str.find_first_of(delims, sprev)) != std::string::npos) {
      sprev = pos + 1;

      // FIXME: handle escape
      if(str[pos] == '\'' || str[pos] == '\"') {
        if(!ins) {
          ins = str[pos];
        } else if(ins == str[pos]) {
          ins = 0;
        }
        continue;
      }
      if(ins) {
        continue;
      }

      if(pos > prev) {
        elems.push_back(str.substr(prev, pos - prev));
      }
      prev = pos + 1;
    }
    if(prev < str.length()) {
      elems.push_back(str.substr(prev, std::string::npos));
    }

    return elems;
  }

  int Console::executeCommand(const std::string& command) {
    // Convert input to vector
    std::vector<std::string> inputs = split(command);
    if(inputs.size() == 0)
      return ReturnCode::Ok;

    Impl::RegisteredCommands::iterator it;
    // Command exists
    if((it = pimpl_->commands_.find(inputs.at(0))) != end(pimpl_->commands_)) {
      // Number of arguments is sufficient
      if(inputs.size() > it->second.args) {
        return static_cast<int>((it->second.func)(inputs));
      } else {
        std::cout << "Usage: " << inputs.at(0) << " " << it->second.arglist << std::endl;
        std::cout << "       " << it->second.help << std::endl;
        return ReturnCode::Error;
      }
    }

    std::cout << "Command '" << inputs.at(0) << "' not found.\n";
    return ReturnCode::Error;
  }

  int Console::executeFile(const std::string& filename) {
    std::ifstream input(filename);
    if(!input) {
      std::cout << "Could not find the specified file to execute.\n";
      return ReturnCode::Error;
    }
    std::string command;
    int counter = 0, result;

    while(std::getline(input, command)) {
      if(command[0] == '#')
        continue; // Ignore comments
      // Report what the Console is executing.
      std::cout << "[" << counter << "] " << command << '\n';
      if((result = executeCommand(command)))
        return result;
      ++counter;
      std::cout << '\n';
    }

    // If we arrived successfully at the end, all is ok
    return ReturnCode::Ok;
  }

  int Console::readLine() {
    reserveConsole();

    char* buffer = readline(pimpl_->greeting_.c_str());
    if(!buffer) {
      std::cout << '\n'; // EOF doesn't put last endline so we put that so that it looks uniform.
      return ReturnCode::Quit;
    }

    // TODO: Maybe add commands to history only if succeeded?
    if(buffer[0] != '\0')
      add_history(buffer);

    std::string line(buffer);
    free(buffer);
    return executeCommand(line);
  }

  char** Console::getCommandCompletions(const char* text, int start, int) {
    char** completionList = nullptr;

    if(start == 0)
      completionList = rl_completion_matches(text, &Console::commandIterator);

    return completionList;
  }

  char* Console::commandIterator(const char* text, int state) {
    static Impl::RegisteredCommands::iterator it;
    if(!currentConsole)
      return nullptr;
    auto& commands = currentConsole->pimpl_->commands_;

    if(state == 0)
      it = begin(commands);

    while(it != end(commands)) {
      auto& command = it->first;
      ++it;
      if(command.find(text) != std::string::npos) {
        char* completion = new char[command.size()];
        strcpy(completion, command.c_str());
        return completion;
      }
    }
    return nullptr;
  }
} // namespace CppReadline
