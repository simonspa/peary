#ifndef CONSOLE_CONSOLE_HEADER_FILE
#define CONSOLE_CONSOLE_HEADER_FILE

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace caribou {
  /**
   * @brief This is the function type that is used to interface with the Console class.
   *
   * These are the functions that are going to get called by Console
   * when the user types in a message. The vector will hold the
   * command elements, and the function needs to return its result.
   * The result can either be Quit (-1), OK (0), or an arbitrary
   * error (>=1).
   */
  using Arguments = std::vector<std::string>;
  using CommandFunction = std::function<int(const Arguments&)>;

  enum ReturnCode {
    Quit = -1,
    Ok = 0,
    Error = 1 // Or greater!
  };

  class ConsoleCommand {
  public:
    ConsoleCommand() : func(), help(""), arglist(""), args(0){};
    ConsoleCommand(CommandFunction f, std::string h, size_t n, std::string a = "") : func(f), help(h), arglist(a), args(n){};
    CommandFunction func;
    std::string help;
    std::string arglist;
    size_t args;
  };

  class Console {
  public:
    /**
     * @brief Basic constructor.
     *
     * The Console comes with two predefined commands: "quit" and
     * "exit", which both terminate the console, "help" which prints a
     * list of all registered commands, and "run" which executes script
     * files.
     *
     * These commands can be overridden or unregistered - but remember
     * to leave at least one to quit ;).
     *
     * @param greeting This represents the prompt of the Console.
     */
    explicit Console(std::string const& greeting);

    /**
     * @brief Basic destructor.
     *
     * Frees the history which is been produced by GNU readline.
     */
    ~Console();

    /**
     * @brief This function registers a new command within the Console.
     *
     * If the command already existed, it overwrites the previous entry.
     *
     * @param s The name of the command as inserted by the user.
     * @param f The ConsoleCommand with the function that will be called once the user writes the command.
     */
    void registerCommand(const std::string& s, ConsoleCommand f);

    /**
     * @brief This function registers a new command within the Console.
     *
     * If the command already existed, it overwrites the previous entry.
     *
     * @param s The name of the command as inserted by the user.
     * @param f The function that will be called once the user writes the command.
     * @param helptext The help text describing the purpose of the command
     * @param numArgs The number of arguments the command expects at least
     * @param arglist A string listing the required arguments for the command, printed when parameters are missing
     */
    void
    registerCommand(const std::string& s, CommandFunction f, std::string helptext, size_t numArgs, std::string arglist = "");

    /**
     * @brief This function returns a list with the currently available commands.
     *
     * @return A vector containing all registered commands names.
     */
    std::vector<std::string> getRegisteredCommands() const;

    /**
     * @brief Sets the prompt for this Console.
     *
     * @param greeting The new greeting.
     */
    void setGreeting(const std::string& greeting);

    /**
     * @brief Gets the current prompt of this Console.
     *
     * @return The currently set greeting.
     */
    std::string getGreeting() const;

    /**
     * @brief This function executes an arbitrary string as if it was inserted via stdin.
     *
     * @param command The command that needs to be executed.
     *
     * @return The result of the operation.
     */
    int executeCommand(const std::string& command);

    /**
     * @brief This function calls an external script and executes all commands inside.
     *
     * This function stops execution as soon as any single command returns something
     * different from 0, be it a quit code or an error code.
     *
     * @param filename The pathname of the script.
     *
     * @return What the last command executed returned.
     */
    int executeFile(const std::string& filename);

    /**
     * @brief This function executes a single command from the user via stdin.
     *
     * @return The result of the operation.
     */
    int readLine();

  protected:
    /** Possiblity for dervice console classes to add unregistered commands to be executed:
     */
    virtual int unregisteredCommand(const std::vector<std::string>&) { return ReturnCode::Error; }
    virtual void listUnregisteredCommands(){};

  private:
    Console(const Console&) = delete;
    Console(Console&&) = delete;
    Console& operator=(Console const&) = delete;
    Console& operator=(Console&&) = delete;

    struct Impl;
    using PImpl = ::std::unique_ptr<Impl>;
    PImpl pimpl_;

    /**
     * @brief This function saves the current state so that some other Console can make use of the GNU readline facilities.
     */
    void saveState();
    /**
     * @brief This function reserves the use of the GNU readline facilities to the calling Console instance.
     */
    void reserveConsole();

    std::vector<std::string> split(std::string str, std::string delims = " ,");

    void printUsage(const std::string& name, const caribou::ConsoleCommand& cmd);

    // GNU newline interface to our commands.
    using commandCompleterFunction = char**(const char* text, int start, int end);
    using commandIteratorFunction = char*(const char* text, int state);

    static commandCompleterFunction getCommandCompletions;
    static commandIteratorFunction commandIterator;
  };
} // namespace caribou

#endif