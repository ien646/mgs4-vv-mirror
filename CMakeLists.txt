cmake_minimum_required(VERSION 4.0)

project(${TARGET_NAME} LANGUAGES C CXX)

set(TARGET_NAME mgs4-vv.dll)

add_subdirectory(third-party)

add_library(${TARGET_NAME} SHARED
        src/main.cpp
        src/hooks.cpp
        src/mgs4_addr.cpp
        src/overlay.cpp
        src/tools.cpp
)

set_target_properties(${TARGET_NAME} PROPERTIES
        PREFIX ""
        SUFFIX ""
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
)

if (MINGW)
    target_link_options(${TARGET_NAME} PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE d3dcompiler_47 d3d11 detours imgui)

#set(AUTO_COPY_DLL_PATH "~/.steam/steam/steamapps/common/METAL GEAR SOLID 4/MGS4/mgs4-vv.asi")
#add_custom_command(
#        TARGET ${TARGET_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME} ${AUTO_COPY_DLL_PATH}
#)