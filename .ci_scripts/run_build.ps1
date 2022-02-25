mkdir build
cd build
if ("${env:COMPILER}" -eq "msvc") {
    cmake -G "Visual Studio 17 2022" -A x64 -DSYSTEM_LIBS=OFF ..
} elseif ("${env:COMPILER}" -eq "mingw-32") {
    $env:path = "D:\msys64\mingw32\bin;${env:path}"
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=i686-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
} elseif ("${env:COMPILER}" -eq "mingw-64") {
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSYSTEM_LIBS=OFF -D CMAKE_C_COMPILER=x86_64-w64-mingw32-gcc.exe -D CMAKE_MAKE_PROGRAM=mingw32-make.exe ..
} else {
    throw "Unknown compiler: ${env:COMPILER}"
}
cmake --build . -j 4 --config Release
if (!$?) {
    throw "Build failed"
}
ctest -C Release
if (!$?) {
    throw "Tests failed"
}
