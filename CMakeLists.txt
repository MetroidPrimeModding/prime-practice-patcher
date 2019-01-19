cmake_minimum_required(VERSION 3.12)
project(prime_practice_patcher)
set(CMAKE_CXX_STANDARD 17)

# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Deps
add_subdirectory(extern/nod)
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(extern/json)

# TODO: sodium/boost on windows
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)
# sodium doesn't  have a find_package so just make sure it's installed
find_library(SODIUM_LIBRARY libsodium.a)

# Include
include_directories(${LOGVISOR_INCLUDE_DIR})
include_directories(extern/nod/include)
include_directories(extern/json/include)

# Our srcs
set(HEADERS
  src/release/ReleaseBuilder.hpp
  src/ReleaseInfo.hpp
  src/crypto/crypto.hpp)

set(SRCS
  src/main.cpp
  src/release/ReleaseBuilder.cpp
  src/ReleaseInfo.cpp
  src/crypto/crypto.cpp)

# Targets
add_executable(${PROJECT_NAME} ${HEADERS} ${SRCS})
target_link_libraries(${PROJECT_NAME}
  nod
  logvisor
  nlohmann_json::nlohmann_json
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${SODIUM_LIBRARY}
  )
