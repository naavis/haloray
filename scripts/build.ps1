# Windows build script for Appveyor

$ErrorActionPreference = "Stop";

$gitBranch = $env:APPVEYOR_REPO_BRANCH
$extraParameters = if ($gitBranch -eq "master") {
    "-DHALORAY_VERSION='${env:APPVEYOR_BUILD_VERSION}'"
} else {
    "-DGIT_BRANCH='${gitBranch}'"
}

pushd
mkdir build
cd build

pushd
cmake .. -G "Visual Studio 15 2017 Win64" $extraParameters
cmake --build . --config Release
cd .\src\Release\
& "${env:Qt5_DIR}\bin\qtenv2.bat"
& "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
& "${env:Qt5_DIR}\bin\windeployqt.exe" `
    --no-quick-import `
    --no-translations `
    --no-webkit2 `
    --no-angle `
    --no-opengl-sw `
    --no-system-d3d-compiler `
    --release `
    haloray.exe
popd

pushd
cd .\tests\Release\
& "${env:Qt5_DIR}\bin\windeployqt.exe" `
    --no-quick-import `
    --no-translations `
    --no-webkit2 `
    --no-angle `
    --no-opengl-sw `
    --no-system-d3d-compiler `
    --release `
    .
popd

popd
