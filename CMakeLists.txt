cmake_minimum_required(VERSION 3.21)
project(vapoursynth-preview-2 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_executable(vapoursynth-preview-2 main.cpp)

if (CMAKE_HOST_WIN32)
    find_path(vapoursynth_DLL_PATH "vapoursynth.dll")
    set(vapoursynth_SDK_PATH "${vapoursynth_DLL_PATH}/../sdk")

    add_library(vapoursynth SHARED IMPORTED GLOBAL)
    set_target_properties(vapoursynth PROPERTIES
        IMPORTED_LOCATION ${vapoursynth_DLL_PATH}
        IMPORTED_IMPLIB "${vapoursynth_SDK_PATH}/lib64/vapoursynth.lib"
    )
    target_include_directories(vapoursynth SYSTEM INTERFACE
        "${vapoursynth_SDK_PATH}/include/vapoursynth"
    )

    target_link_libraries(vapoursynth-preview-2 PRIVATE vapoursynth)

    add_library(vapoursynth-script SHARED IMPORTED GLOBAL)
    set_target_properties(vapoursynth-script PROPERTIES
        IMPORTED_LOCATION "${vapoursynth_DLL_PATH}/vsscript.dll"
        IMPORTED_IMPLIB "${vapoursynth_SDK_PATH}/lib64/vsscript.lib"
    )
    target_include_directories(vapoursynth SYSTEM INTERFACE
        "${vapoursynth_SDK_PATH}/include/vapoursynth"
    )

    target_link_libraries(vapoursynth-preview-2 PRIVATE vapoursynth-script)

elseif (CMAKE_HOST_UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(vapoursynth REQUIRED IMPORTED_TARGET vapoursynth)
    target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth)

    pkg_check_modules(vapoursynth-script REQUIRED IMPORTED_TARGET vapoursynth-script)
    target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth-script)
endif()

target_compile_definitions(vapoursynth-preview-2 PRIVATE ASIO_HAS_STD_INVOKE_RESULT)

find_package(elements REQUIRED)
find_package(Microsoft.GSL REQUIRED)

target_link_libraries(vapoursynth-preview-2 PRIVATE cycfi::elements)
target_link_libraries(vapoursynth-preview-2 PRIVATE Microsoft.GSL::GSL)
if (APPLE)
    target_link_libraries(vapoursynth-preview-2 PRIVATE
        "-framework AppKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreText"
    )
endif()

if (NOT MSVC)
    target_compile_options(vapoursynth-preview-2 PRIVATE "-fcolor-diagnostics")
endif()