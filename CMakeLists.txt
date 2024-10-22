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

# Platform-specific settings
if(APPLE)
    # macOS-specific linking
    target_link_libraries(FMPlugInTemplate "-framework ${CMAKE_SOURCE_DIR}/Libraries/Mac/FMWrapper.framework/Versions/A/FMWrapper")

elseif(WIN32)
    # Windows-specific linking
    target_link_directories(FMPlugInTemplate PRIVATE ${CMAKE_SOURCE_DIR}/Libraries/Win/x64)
    target_link_libraries(FMPlugInTemplate PRIVATE FMWrapper)

    # Set specific compiler flags for Windows builds
    target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:DEBUG>:_DEBUG;_WINDOWS;_USRDLL;FMPlugInTemplate_EXPORTS>)
    target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:RELEASE>:NDEBUG;_WINDOWS;_USRDLL;FMPlugInTemplate_EXPORTS>)

    # Specify the libraries to delay-load for Windows
    set_property(TARGET FMPlugInTemplate PROPERTY LINK_FLAGS "/DELAYLOAD:FMWrapper.dll")
endif()

# Set specific compiler flags for Debug and Release builds
target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:DEBUG>:_DEBUG;FMPlugInTemplate_EXPORTS>)
target_compile_definitions(FMPlugInTemplate PRIVATE $<$<CONFIG:RELEASE>:NDEBUG;FMPlugInTemplate_EXPORTS>)

# Post-build command to rename the output file to .fmx64 
add_custom_command(TARGET FMPlugInTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
    "$<TARGET_FILE:FMPlugInTemplate>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PlugInTemplate.fmx64"
)

# Post-build command to remove the Debug folder
add_custom_command(TARGET FMPlugInTemplate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
)