# Windows build script for Appveyor

pushd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
cmd /c "vcvars64.bat&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd
Write-Host "`nVisual Studio 2019 Command Prompt variables set." -ForegroundColor Yellow

$extraParameters = if ($gitBranch -eq "master") {
  $version = ${env:APPVEYOR_BUILD_VERSION}.Split('-')[0]
  "-DHALORAY_VERSION='${version}'"
}

pushd
mkdir build
cd src
& "${env:Qt5_DIR}\bin\qmake.exe" main.pro -o ..\build\ -config release
cd ..\build
nmake 2>$null
popd

pushd
cd build\haloray\release
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
