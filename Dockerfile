FROM archlinux as build

RUN pacman -Syu --noconfirm
RUN pacman -S mingw-w64-gcc --noconfirm
RUN pacman -S cmake --noconfirm
RUN pacman -S make --noconfirm

RUN mkdir /src
COPY . /src
WORKDIR /src

RUN mkdir build-release &&  \
    cp cmake/x86_64-mingw.toolchain.cmake build-release/x86_64-mingw.toolchain.cmake && \
    cd build-release && \
    cmake -DCMAKE_TOOLCHAIN_FILE="x86_64-mingw.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j

FROM scratch AS output
COPY --from=build /src/build-release /
