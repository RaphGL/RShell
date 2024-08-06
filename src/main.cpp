#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using ArgsList = std::vector<std::string>;

ArgsList get_args(std::string_view args) {
  ArgsList argv{};

  size_t curr_start = 0;
  size_t curr_end = 0;
  for (const auto ch : args) {
    if (ch == ' ') {
      std::string curr_arg{&args[curr_start], curr_end - curr_start};
      argv.push_back(std::move(curr_arg));
      ++curr_end;
      curr_start = curr_end;
      continue;
    }

    ++curr_end;
  }

  if (args.back() != ' ') {
    std::string curr_arg{&args[curr_start], args.size() - curr_start};
    argv.push_back(std::move(curr_arg));
  }

  return argv;
}

int run_cmd(ArgsList args) {
  std::vector<const char *> argv{};
  argv.reserve(args.size() + 1);

  for (const auto &arg : args) {
    argv.push_back(arg.c_str());
  }

  // argv must terminate with a NULL
  argv.push_back(nullptr);

  int stdout_pipes[2], stderr_pipes[2];
  if (pipe(stdout_pipes) == -1 || pipe(stderr_pipes) == -1) {
    std::cerr << "Failed to create redirection pipes.\n";
    return -1;
  }

  auto pid = fork();

  if (pid == -1) {
    close(stdout_pipes[0]);
    close(stdout_pipes[1]);
    close(stderr_pipes[0]);
    close(stderr_pipes[1]);
    std::cerr << "Failed to fork process.\n";
    return -1;
  }

  if (pid == 0) {
    if (dup2(stdout_pipes[1], STDOUT_FILENO) == -1 ||
        dup2(stderr_pipes[1], STDERR_FILENO) == -1) {
      std::cerr << "Failed to capture stdout and stderr.";
      return -1;
    }

    close(stdout_pipes[0]);
    close(stderr_pipes[0]);

    if (execvp(argv.front(), const_cast<char *const *>(argv.data())) == -1) {
      std::cerr << std::format("Failed to launch command: {}\n", args[0]);
      return -1;
    }
  } else {

    close(stdout_pipes[1]);
    close(stderr_pipes[1]);
    int status{};
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    std::array<char, 1024 * 4> buffer{0};
    while (read(stdout_pipes[0], buffer.data(), buffer.size()) > 0) {
      std::cout << std::string_view(buffer.begin(), buffer.end());
    }

    buffer.fill(0);
    while (read(stderr_pipes[0], buffer.data(), buffer.size()) > 0) {
      std::cerr << std::string_view(buffer.begin(), buffer.end());
    }

    return WEXITSTATUS(status);
  }

  return 0;
}

void print_prompt() {
  std::cout << std::format("{} $ ", getlogin());
  std::cout.flush();
}

int main(int argc, char **argv) {
  std::string input{};
  input.reserve(1024 * 4);
  for (;;) {
    print_prompt();

    input.clear();
    std::getline(std::cin, input);

    auto args = get_args(input);
    run_cmd(args);
  }
}
