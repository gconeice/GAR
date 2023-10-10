cmake_minimum_required (VERSION 3.20)
project (SGC2PC)
set(NAME "SGC2PC")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


add_definitions(-Wfatal-errors)
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -pthread -Wall -Wextra -march=native -msse4.1 -mavx2 -g")

#set(CMAKE_CXX_FLAGS_DEBUG "-g -msse4.1")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -msse4.1")

if (APPLE)
  set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl")
endif()
find_package(OpenSSL REQUIRED)
find_package(emp-tool)
find_package(emp-ot)

#add_subdirectory(epigram)
add_subdirectory(epigram/test/doctest)
add_subdirectory(epigram/test/rapidcheck)

include_directories(epigram/src)

set(EPISOURCES
  epigram/src/mode.cc
  epigram/src/resource.cc
  epigram/src/bit.cc
  epigram/src/byte.cc
  epigram/src/bitstring.cc
  epigram/src/arithmetic.cc
  epigram/src/onehot.cc
  epigram/src/permute.cc
  epigram/src/stack.cc
  epigram/src/simplestack.cc
  epigram/src/linearscan.cc
  epigram/src/epigram.cc
  epigram/src/cpu.cc
  )

set(PRIMITIVES "primitives")
add_library(${PRIMITIVES} cryptographic_primitives/Prf.cc)

set(SOURCES  
    AsParse.cpp
    Arithmetic.cc
    Bit.cpp
    Bit32.cpp
    Const.cpp
    SGCUtils.cpp)

option(GARDEBUG "Option description" OFF)
if(GARDEBUG)
  add_definitions(-DGARDEBUG)
endif(GARDEBUG)


add_executable(SGC2PC Main.cpp ${SOURCES} ${EPISOURCES})
target_include_directories(
    SGC2PC 
    PUBLIC
    epigram/src
    cryptographic_primitives
    epigram/test/doctest/doctest
    ${RAPIDCHECK_INCLUDE_DIR}
    ${EMP-TOOL_INCLUDE_DIR}
    ${EMP-OT_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR})
target_link_libraries(
    SGC2PC
    doctest
    rapidcheck
    ${PRIMITIVES}
    ${EMP-TOOL_LIBRARY}
    ${EMP-OT_LIBBRARY}
    ${OPENSSL_LIBRARIES})
target_compile_options(SGC2PC PRIVATE "-DDOCTEST_CONFIG_DISABLE=1 -NDEBUG")
  
