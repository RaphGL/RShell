#include "builtin.hpp"
#include <cstdlib>
#include <functional>
#include <string_view>
#include <unistd.h>
#include <unordered_map>

static int cd(int argc, char *argv[]) {
  if (argc == 1) {
    return chdir(getenv("HOME"));
  }

  return chdir(argv[0]);
}

static const std::unordered_map<std::string_view,
                                std::function<int(int, char **)>>
    builtin_map{
        {"cd", cd},
    };

int builtin::cmd(std::string_view cmdname, int argc, char **argv) {
  return builtin_map.at(cmdname)(argc, argv);
}

bool builtin::has_cmd(std::string_view cmdname) {
  return builtin_map.contains(cmdname);
}
