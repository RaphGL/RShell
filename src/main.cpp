#include "shell.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  Shell shell{};
  std::string input{};
  input.reserve(1024 * 4);

  for (;;) {
    shell.print_prompt();

    input.clear();
    std::getline(std::cin, input);

    shell.parse_args(input);
    shell.run_cmd();
  }
}
