cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

project(rsh)

add_executable(rsh 
  src/builtin.cpp
  src/main.cpp
  src/shell.cpp
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(RSH_SANITIZERS -fsanitize=undefined,address,leak)
  target_compile_options(rsh PUBLIC ${RSH_SANITIZERS})
  target_link_options(rsh PUBLIC ${RSH_SANITIZERS})
endif()

target_compile_options(rsh PRIVATE
  -Wall -Wextra -pedantic-errors
)

