# vcpkg_from_github(
#     OUT_SOURCE_PATH SOURCE_PATH
#     REPO cycfi/elements
#     REF ae7b87a8cc53258fac7d1d5a7b96da2994116dcf
#     SHA512 0
#     HEAD_REF master
# )

if (WIN32)
vcpkg_cmake_configure(
    SOURCE_PATH "C:/Projects/elements"
    OPTIONS
        -DELEMENTS_BUILD_EXAMPLES=OFF
)
else()
vcpkg_cmake_configure(
    SOURCE_PATH "/mnt/c/Projects/elements"
    OPTIONS
        -DELEMENTS_BUILD_EXAMPLES=OFF
)
endif()

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")