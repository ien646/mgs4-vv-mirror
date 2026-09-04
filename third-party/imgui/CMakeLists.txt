file(GLOB SOURCES "*.cpp")
add_library(imgui ${SOURCES})
target_include_directories(imgui PUBLIC ".")
target_link_libraries(imgui PRIVATE d3dcompiler_47 dwmapi)