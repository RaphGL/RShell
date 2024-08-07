#ifndef SHELL_HPP
#define SHELL_HPP

#include <string>
#include <vector>

using ArgsList = std::vector<std::string>;

class Shell {
  ArgsList m_args{};
  std::vector<const char *> m_argv{};

  int spawn_cmd();

public:
  Shell();

  void parse_args(std::string_view args);
  int run_cmd();
  void print_prompt();
};

#endif
