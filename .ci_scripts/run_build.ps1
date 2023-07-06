mkdir build
cd build
if ("${env:COMPILER}" -eq "msvc") {
    cmake -G "Visual Studio 17 2022" -A x64 -DSYSTEM_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
    cmake --build . -j 4 --config RelWithDebInfo
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $env:path = "D:\msys64\mingw32\bin;${env:path}"
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=i686-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
    cmake --build . -j 4 --config Release
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
    cmake --build . -j 4 --config Release
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}
if (!$?) {
    throw "Build failed"
}
