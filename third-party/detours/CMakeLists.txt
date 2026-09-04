add_library(detours
        detours.cpp
        modules.cpp
        disasm.cpp
        image.cpp
        creatwth.cpp
        disolx86.cpp
        disolx64.cpp
        disolia64.cpp
        disolarm.cpp
        disolarm64.cpp
)
target_include_directories(detours PUBLIC ".")

target_compile_options(detours PUBLIC "-w")

target_link_options(detours PUBLIC "-static-libgcc" "-static-libstdc++" "-static")