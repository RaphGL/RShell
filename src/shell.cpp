#include "shell.hpp"
#include "builtin.hpp"

#include <array>
#include <cctype>
#include <format>
#include <iostream>
#include <limits.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

Shell::Shell() {
  m_args.reserve(PATH_MAX);
  m_argv.reserve(m_args.size());
}

void Shell::parse_args(std::string_view args) {
  m_args.clear();

  size_t curr_start = 0;
  size_t curr_end = 0;
  for (const auto ch : args) {
    if (std::isspace(ch)) {
      if (curr_start == curr_end)
        continue;

      std::string curr_arg{&args[curr_start], curr_end - curr_start};
      m_args.push_back(std::move(curr_arg));
      ++curr_end;
      curr_start = curr_end;
      continue;
    }

    ++curr_end;
  }

  if (!std::isspace(args.back()) && curr_start != curr_end) {
    std::string curr_arg{&args[curr_start], args.size() - curr_start};
    m_args.push_back(std::move(curr_arg));
  }

  m_argv.clear();
  m_argv.reserve(m_args.size() + 1);

  for (const auto &arg : m_args) {
    m_argv.push_back(arg.c_str());
  }

  // argv must terminate with a NULL
  m_argv.push_back(nullptr);
}

int Shell::spawn_cmd() {
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
    close(stdout_pipes[0]);
    close(stderr_pipes[0]);

    if (dup2(stdout_pipes[1], STDOUT_FILENO) == -1 ||
        dup2(stderr_pipes[1], STDERR_FILENO) == -1) {
      std::cerr << "Failed to capture stdout and stderr.";
      exit(-1);
    }

    if (execvp(m_argv.front(), const_cast<char *const *>(m_argv.data())) ==
        -1) {
      std::cerr << std::format("Failed to launch command: {}\n", m_args[0]);
      exit(-1);
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

int Shell::run_cmd() {
  if (m_args.empty()) {
    return 0;
  }

  auto cmdname = m_args.front();
  if (builtin::has_cmd(cmdname)) {
    return builtin::cmd(cmdname, m_argv.size(),
                        const_cast<char **>(m_argv.data()));
  }

  return this->spawn_cmd();
}

void Shell::print_prompt() {
  std::string prompt{};
  prompt += getlogin();
  prompt += " ";
  char cwd[PATH_MAX]{};
  getcwd(cwd, sizeof(cwd));
  prompt += cwd;
  prompt += " ";
  prompt += "$";
  prompt += " ";
  std::cout << prompt;
  std::cout.flush();
}
