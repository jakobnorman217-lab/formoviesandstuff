cmake_minimum_required(VERSION 3.20)
project(Fsr4DesktopUpscaler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_executable(Fsr4DesktopUpscaler
    src/main.cpp
    src/App.cpp
    src/Fsr4Wrapper.cpp
)

target_include_directories(Fsr4DesktopUpscaler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # TODO: Add FidelityFX SDK include path here, e.g. external/FidelityFX-SDK-2.1.0/include
)

# TODO: Add FidelityFX SDK source files or libraries needed for FSR4 integration.
# For example:
# add_subdirectory(external/FidelityFX-SDK-2.1.0) # if using as subproject
# target_link_libraries(Fsr4DesktopUpscaler PRIVATE FidelityFX_fsr4)

# Link against D3D11 and DXGI (available in Windows SDK)
target_link_libraries(Fsr4DesktopUpscaler PRIVATE
    d3d11
    dxgi
    d3dcompiler
)

# Enable Unicode on Windows
if (MSVC)
    target_compile_definitions(Fsr4DesktopUpscaler PRIVATE UNICODE _UNICODE)
    target_compile_options(Fsr4DesktopUpscaler PRIVATE /W4 /permissive-)
endif()

# Optionally copy shader files next to the executable for runtime compilation from disk.
add_custom_command(TARGET Fsr4DesktopUpscaler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:Fsr4DesktopUpscaler>/shaders)
