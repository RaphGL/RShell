#include "builtin.hpp"
#include <cstdlib>
#include <functional>
#include <string_view>
#include <unistd.h>
#include <unordered_map>
#include <iostream>

static int cd(int argc, char *argv[]) {
  char *path = argc == 1 ? getenv("HOME") : argv[1];

  auto ret = chdir(path);
  if (ret == ENOENT) {
    std::cerr << "cd: no such file or directory: " << path << '\n';
  }

  return ret;
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
