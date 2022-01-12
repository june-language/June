cmake_minimum_required(VERSION 3.19.6)

project(June C CXX)

# Honour CMAKE_CXX_STANDARD in try_compile(), needed for check_cxx_native_regex.
if(POLICY CMP0067)
  cmake_policy(SET CMP0067 NEW)
endif()

# Convert relative paths to absolute for subdirectory `target_sources`
if(POLICY CMP0076)
  cmake_policy(SET CMP0076 NEW)
endif()

# Don't clobber existing variable values when evaluating `option()` declarations.
if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

option(JUNE_CROSS_COMPILE "Enable cross-compiling" OFF)
set(JUNE_CROSS_COMPILE_TARGET "arm-linux-gnueabihf" CACHE STRING "Target platform")
set(JUNE_CROSS_COMPILE_SYSTEM "Linux" CACHE STRING "System to cross-compile for")
set(JUNE_CROSS_COMPILE_PROCESSOR "arm" CACHE STRING "Processor to cross-compile for")

option(JUNE_DEBUG "Enable debug build" ON)

if(DEFINED ENV{PREFIX_DIR} AND NOT "$ENV{PREFIX_DIR}" STREQUAL "" AND NOT EXISTS "${JUNE_CROSS_COMPILE}")
	set(CMAKE_INSTALL_PREFIX "$ENV{PREFIX_DIR}")
else()
	set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.june")
endif()
set(CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX};${CMAKE_BINARY_DIR}")
include_directories(${PROJECT_SOURCE_DIR}/include)
add_definitions(-DINSTALL_PREFIX=${CMAKE_INSTALL_PREFIX})
message("-- Using PREFIX = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/modules")

# macOS RPath fix
set(ORIGIN "$ORIGIN")
if(APPLE)
	set(ORIGIN "@loader_path")
endif()
set(CMAKE_INSTALL_RPATH "${ORIGIN}/../lib/june;${ORIGIN}/../lib;${ORIGIN}/.")
set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

# C++ standard
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
    "Build type for June [Debug, RelWithDebInfo, Release]"
    FORCE)
  message(STATUS "No build type was specified, will default to ${CMAKE_BUILD_TYPE}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
	add_definitions(-D_WITH_GETLINE)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" OR ${JUNE_DEBUG})
  message(STATUS "Building with debug info...")
  add_definitions(-DDEBUG_INFO)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" OR ${JUNE_DEBUG})
  message(STATUS "Building for release...")
  # add_definitions(-DRELEASE_MODE)
endif()

# Cross-compiling
if("${JUNE_CROSS_COMPILE}")
  # Check if clang/clang++ is available on the system
  if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to cross-compile")
  endif()

  message(STATUS "Setting up cross-compiler...")
  message(STATUS "Target platform: ${JUNE_CROSS_COMPILE_TARGET}")
  message(STATUS "System: ${JUNE_CROSS_COMPILE_SYSTEM}")
  message(STATUS "Processor: ${JUNE_CROSS_COMPILE_PROCESSOR}")
  message(STATUS "Sysroot: ${CMAKE_SYSROOT}")


  set(CMAKE_SYSTEM_NAME ${JUNE_CROSS_COMPILE_SYSTEM})
  set(CMAKE_SYSTEM_PROCESSOR ${JUNE_CROSS_COMPILE_PROCESSOR})

  set(CMAKE_C_COMPILER clang)
  set(CMAKE_CXX_COMPILER clang++)
  set(CMAKE_C_COMPILER_TARGET ${JUNE_CROSS_COMPILE_TARGET})
  set(CMAKE_CXX_COMPILER_TARGET ${JUNE_CROSS_COMPILE_TARGET})
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# Config
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(JUNE_VERSION "0.1")
set(JUNE_COMPILER_VERSION "0.0.1" CACHE STRING "The internal version of the June compiler")

execute_process(COMMAND date +"%Y-%m-%d" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git rev-parse --short HEAD OUTPUT_VARIABLE GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file("${PROJECT_SOURCE_DIR}/include/JuneConfig.hpp.in" "${PROJECT_SOURCE_DIR}/include/JuneConfig.hpp" @ONLY)

# For libGMP on macOS and BSD
if(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
  include_directories(/usr/local/include)
endif()

find_library(GMP_LIBRARY gmp REQUIRED)
find_package(MPFR REQUIRED)
if(NOT MPFR_FOUND)
  message(FATAL_ERROR "MPFR library is required but missing")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
  message("-- Using thread flags: -pthread")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")
else()
  message("-- Using thread flags: ${CMAKE_THREAD_LIBS_INIT}")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
  message("-- Using ccache")
endif()

# Install CMake modules
install(DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/modules/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake/modules"
)

# Install Includes
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/june"
)

#[[

# JuneCommon (utils, config, etc.)
file(GLOB_RECURSE INCS RELATIVE "${PROJECT_SOURCE_DIR}" "include/JuneCommon/*.hpp")
file(GLOB_RECURSE SRCS RELATIVE "${PROJECT_SOURCE_DIR}" "src/JuneCommon/*.cpp")

add_library(junecommon SHARED ${SRCS} ${INCS})
set_target_properties(junecommon
    PROPERTIES
    OUTPUT_NAME junecomm
    LINK_FLAGS "${EXTRA_LD_FLAGS}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/june"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
install(TARGETS junecommon
	LIBRARY
	  DESTINATION lib/june
	  COMPONENT Libraries
	PUBLIC_HEADER
	  DESTINATION include/june
)

# June VM

file(GLOB_RECURSE INCS RELATIVE "${PROJECT_SOURCE_DIR}" "include/VM/*.hpp")
file(GLOB_RECURSE SRCS RELATIVE "${PROJECT_SOURCE_DIR}" "src/VM/*.cpp")

add_library(blossom-vm SHARED ${SRCS} ${INCS})
target_link_libraries(junevm blossom-common ${CMAKE_DL_LIBS} ${MPFR_LIBRARIES} ${GMP_LIBRARY})
set_target_properties(junevm
    PROPERTIES
    OUTPUT_NAME junevm
    LINK_FLAGS "${EXTRA_LD_FLAGS}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/june"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
install(TARGETS junevm
	LIBRARY
	  DESTINATION lib/june
	  COMPONENT Libraries
	PUBLIC_HEADER
	  DESTINATION include/june
)

]]#