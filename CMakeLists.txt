cmake_minimum_required(VERSION 3.20)
project(Kaleidoscope CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure LLVM (adjust LLVM_DIR if needed)
set(LLVM_DIR "C:/dev/llvm/build/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

# Add subdirectory containing the executable target
add_subdirectory(LLVM-Frontend)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			 PROPERTY VS_STARTUP_PROJECT LLVM-Frontend)
