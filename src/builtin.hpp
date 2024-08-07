#ifndef BUILTIN_HPP
#define BUILTIN_HPP
#include <string_view>

namespace builtin {
int cmd(std::string_view cmdname, int argc, char **argv);
bool has_cmd(std::string_view cmdname);
}

#endif
