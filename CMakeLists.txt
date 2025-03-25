cmake_minimum_required(VERSION 3.15.5)
project(duplo)

if(CMAKE_CONFIGURATION_TYPES)
    list(APPEND CMAKE_CONFIGURATION_TYPES ASAN TSAN)
    list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Add the configurations that we need"
        FORCE)
endif()


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

find_package(Threads REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)
add_library(thread_pool INTERFACE)
target_include_directories(thread_pool INTERFACE ${thread_pool_SOURCE_DIR}/include)

add_executable(duplo ${SOURCES})

set_target_properties(duplo PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_compile_definitions(duplo PRIVATE DUPLO_VERSION=${DUPLO_VERSION})
target_include_directories(duplo PRIVATE src/include/)
target_link_libraries(duplo PRIVATE nlohmann_json::nlohmann_json thread_pool Threads::Threads)

target_compile_options(duplo PRIVATE $<$<CONFIG:ASAN>:-fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O1>)
target_link_options(duplo PRIVATE $<$<CONFIG:ASAN>:-fsanitize=address>)
target_compile_options(duplo PRIVATE $<$<CONFIG:TSAN>:-fsanitize=thread -g -O1>)
target_link_options(duplo PRIVATE $<$<CONFIG:TSAN>:-fsanitize=thread>)

if(NOT MSVC)
    target_compile_options(duplo PRIVATE -Wall -Werror)
endif()
