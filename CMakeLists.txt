# Specify the minimum CMake version
cmake_minimum_required(VERSION 3.14)

# Set the project name and version
project(FMPlugInTemplate VERSION 1.0 LANGUAGES CXX)

# Set the target output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/PlugIn)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the source files
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/PlugInTemplate.cpp
)

# Define the target as a shared library
add_library(FMPlugInTemplate SHARED ${SOURCES})

# Include directories
target_include_directories(FMPlugInTemplate PRIVATE ${CMAKE_SOURCE_DIR}/Headers)
target_include_directories(FMPlugInTemplate PRIVATE ${CMAKE_SOURCE_DIR}/Libraries)

# Link against additional libraries
target_link_libraries(FMPlugInTemplate "-framework ${CMAKE_SOURCE_DIR}/Libraries/Mac/FMWrapper.framework/Versions/A/FMWrapper")

# Set specific compiler flags for Debug and Release builds
target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:DEBUG>:_DEBUG;FMPlugInTemplate_EXPORTS>)
target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:RELEASE>:NDEBUG;FMPlugInTemplate_EXPORTS>)

# Post-build command to rename the output file
add_custom_command(TARGET FMPlugInTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
    "$<TARGET_FILE:FMPlugInTemplate>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PlugIn_Template.fmx64"
)

# Post-build command to remove unnecessary directories
add_custom_command(TARGET FMPlugInTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
)