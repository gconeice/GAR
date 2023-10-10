cmake_minimum_required (VERSION 3.10)
project (epigram)
set(NAME "epigram")

set(CMAKE_BUILD_TYPE Debug)


add_definitions(-Wfatal-errors)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -pthread -Wall -Wextra -march=native -msse4.1 -mavx2 -g")

if (APPLE)
  set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()
find_package(OpenSSL REQUIRED)


add_subdirectory(test/doctest)
add_subdirectory(test/rapidcheck)

set(sources
  src/mode.cc
  src/resource.cc
  src/bit.cc
  src/byte.cc
  src/bitstring.cc
  src/arithmetic.cc
  src/onehot.cc
  src/permute.cc
  src/stack.cc
  src/simplestack.cc
  src/linearscan.cc
  src/epigram.cc
  src/cpu.cc
  )

add_executable(tests src/test.cc ${sources})
target_include_directories(tests PUBLIC src ${OPENSSL_INCLUDE_DIR} test/doctest/doctest ${RAPIDCHECK_INCLUDE_DIR})
target_link_libraries(tests ${OPENSSL_LIBRARIES} doctest rapidcheck)

add_executable(${NAME} src/main.cc ${sources})
target_include_directories(${NAME} PUBLIC src ${OPENSSL_INCLUDE_DIR} test/doctest/doctest ${RAPIDCHECK_INCLUDE_DIR})
target_link_libraries(${NAME} ${OPENSSL_LIBRARIES} doctest rapidcheck)
target_compile_options(${NAME} PRIVATE "-DDOCTEST_CONFIG_DISABLE=1 -NDEBUG")
