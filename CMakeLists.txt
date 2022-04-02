cmake_minimum_required(VERSION 3.21)
project(vapoursynth-preview-2 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS off)

find_package(PkgConfig REQUIRED)

add_executable(vapoursynth-preview-2 main.cpp)
target_compile_definitions(vapoursynth-preview-2 PRIVATE ASIO_HAS_STD_INVOKE_RESULT)

pkg_check_modules(vapoursynth REQUIRED IMPORTED_TARGET vapoursynth)
target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth)

pkg_check_modules(vapoursynth-script REQUIRED IMPORTED_TARGET vapoursynth-script)
target_link_libraries(vapoursynth-preview-2 PRIVATE PkgConfig::vapoursynth-script)

include(FetchContent)
FetchContent_Declare(
    elements
    GIT_REPOSITORY https://github.com/cycfi/elements.git
    GIT_TAG ae7b87a8cc53258fac7d1d5a7b96da2994116dcf
    GIT_SUBMODULES_RECURSE on
    GIT_SHALLOW on
    GIT_PROGRESS on
)
set(ELEMENTS_BUILD_EXAMPLES off)
FetchContent_MakeAvailable(elements)

target_link_libraries(vapoursynth-preview-2 PRIVATE cycfi::elements)
if(APPLE)
    target_link_libraries(vapoursynth-preview-2 PRIVATE "-framework CoreFoundation")
    target_link_libraries(vapoursynth-preview-2 PRIVATE "-framework CoreGraphics")
    target_link_libraries(vapoursynth-preview-2 PRIVATE "-framework CoreText")
    target_link_libraries(vapoursynth-preview-2 PRIVATE "-framework AppKit")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
