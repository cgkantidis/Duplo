cmake_minimum_required(VERSION 3.15.5)
project(duplo)
file(GLOB SOURCES src/*.cpp)

SET(DUPLO_VERSION "\"v1.0.1\"" CACHE STRING "Duplo version")

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW 1
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  thread_pool
  GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
  GIT_TAG v5.0.0
  GIT_SHALLOW 1
  OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(nlohmann_json thread_pool)

add_executable(duplo ${SOURCES})

set_target_properties(duplo PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(duplo PRIVATE DUPLO_VERSION=${DUPLO_VERSION})
target_include_directories(duplo PRIVATE src/include/)
find_package(nlohmann_json 3.11.3 REQUIRED)
find_package(thread_pool 5.0.0 REQUIRED)
add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE ${thread_pool_SOURCE_DIR}/include)
target_link_libraries(duplo PRIVATE nlohmann_json::nlohmann_json thread_pool)

if(NOT MSVC)
    target_compile_options(duplo PRIVATE -Wall -Werror)
endif()
