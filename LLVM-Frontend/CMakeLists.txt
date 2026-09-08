add_executable(LLVM-Frontend "main.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LLVM-Frontend PROPERTY CXX_STANDARD 20)
endif()

# Ensure the target can find headers in this directory
target_include_directories(LLVM-Frontend PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link against LLVM libraries discovered in the top-level CMake
if (TARGET LLVM::Core OR TARGET LLVM)
  # Prefer modern imported targets if available
  target_link_libraries(LLVM-Frontend PRIVATE ${llvm_libs})
else()
  target_link_libraries(LLVM-Frontend PRIVATE ${llvm_libs})
endif()


