vcpkg_from_github(
    OUT_SOURCE_PATH ELEMENTS_PATH
    REPO endilll/elements
    REF 6c8b6b51f9862982991ec4f15f1fffe782c4d096
    SHA512 b69df93fd3cffd07973a5fcc3b887ac07a1d5bd8956d4703108a54ba2b3b0fc863282f65dc64119afeda19f1291f177a772804343d6f8031578743b42287d10c
    HEAD_REF vlad/vcpkg
)

vcpkg_from_github(
    OUT_SOURCE_PATH INFRA_PATH
    REPO cycfi/infra
    REF 99a3ea7d3c7a5f2750cc7c29400e28536b6fe3e2
    SHA512 ace6c7813222a3ae0eda16c986332e4399e97bcd7f5c4e9b6ecf227f31c384359a96d3b13e6a109169a46050cf64867e386a198bba3616aa94cfb5dacc283485
)

file(REMOVE_RECURSE ${ELEMENTS_PATH}/lib/infra)
file(CREATE_LINK
    ${INFRA_PATH}
    ${ELEMENTS_PATH}/lib/infra
    SYMBOLIC
)

vcpkg_cmake_configure(
    SOURCE_PATH ${ELEMENTS_PATH}
    OPTIONS
        -DELEMENTS_BUILD_EXAMPLES=OFF
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")