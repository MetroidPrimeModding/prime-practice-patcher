cmake_minimum_required(VERSION 3.12)
project(prime_practice_patcher)
set(CMAKE_CXX_STANDARD 17)

# flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Deps
add_subdirectory(extern/nod)

# on apple we use boost filesystem, everywhere else the default one
if (APPLE)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost COMPONENTS system filesystem REQUIRED)
  set(LIBS
    ${LIBS}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})
endif ()

# Include
include_directories(${LOGVISOR_INCLUDE_DIR})
include_directories(extern/nod/include)
include_directories(extern/json/include)

# Our srcs
set(HEADERS
  src/release/ReleaseBuilder.hpp
  src/patcher/Patcher.hpp
  src/dol/DolFile.hpp
  src/fs.hpp)

set(SRCS
  src/main.cpp
  src/release/ReleaseBuilder.cpp
  src/patcher/Patcher.cpp
  src/dol/DolFile.cpp)

set(LIBS
  ${LIBS}
  nod
  logvisor)

# Targets
add_executable(${PROJECT_NAME} ${HEADERS} ${SRCS})
target_link_libraries(${PROJECT_NAME}
  ${LIBS}
  )
