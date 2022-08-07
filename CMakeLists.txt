cmake_minimum_required(VERSION 3.24)
project(vapoursynth-preview-2 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(vapoursynth-preview-2 main.cpp)

if (NOT WIN32)
    find_package(PkgConfig REQUIRED)

    add_executable(vapoursynth-preview-2 main.cpp)
    target_compile_definitions(vapoursynth-preview-2 PRIVATE ASIO_HAS_STD_INVOKE_RESULT)

    pkg_check_modules(vapoursynth REQUIRED IMPORTED_TARGET vapoursynth)
    target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth)

    pkg_check_modules(vapoursynth-script REQUIRED IMPORTED_TARGET vapoursynth-script)
    target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth-script)
else()
    set(VAPOURSYNTH_DIR "c:/Program Files/VapourSynth")

    add_library(vapoursynth SHARED IMPORTED)
    target_include_directories(vapoursynth INTERFACE ${VAPOURSYNTH_DIR}/sdk/include/vapoursynth)
    set_target_properties(vapoursynth PROPERTIES
        IMPORTED_LOCATION ${VAPOURSYNTH_DIR}/core/vapoursynth.dll
        IMPORTED_IMPLIB ${VAPOURSYNTH_DIR}/sdk/lib64/vapoursynth.lib
    )
    
    add_library(vapoursynth-script SHARED IMPORTED)
    target_include_directories(vapoursynth-script INTERFACE ${VAPOURSYNTH_DIR}/sdk/include/vapoursynth)
    set_target_properties(vapoursynth-script PROPERTIES
        IMPORTED_LOCATION ${VAPOURSYNTH_DIR}/core/vsscript.dll
        IMPORTED_IMPLIB ${VAPOURSYNTH_DIR}/sdk/lib64/vsscript.lib
    )

    target_link_libraries(vapoursynth-preview-2 PUBLIC vapoursynth)
    target_link_libraries(vapoursynth-preview-2 PUBLIC vapoursynth-script)
endif()

find_package(imgui CONFIG REQUIRED)
target_link_libraries(vapoursynth-preview-2 PUBLIC imgui::imgui)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(vapoursynth-preview-2 PUBLIC Microsoft.GSL::GSL)

find_package(OpenGL REQUIRED)
target_link_libraries(vapoursynth-preview-2 PRIVATE OpenGL::GL)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_link_libraries(vapoursynth-preview-2 PRIVATE vcpkg_installed/x64-windows-static-md/debug/lib/manual-link/SDL2maind)
else()
    target_link_libraries(vapoursynth-preview-2 PRIVATE vcpkg_installed/x64-windows-static-md/lib/manual-link/SDL2main)
endif()

find_package(GLEW REQUIRED)
target_link_libraries(vapoursynth-preview-2 PRIVATE GLEW::GLEW)
