# Windows build script for Appveyor

Push-Location "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
cmd /c "vcvars64.bat&set" | ForEach-Object {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])" -value "$($v[1])"
  }
}
Pop-Location
Write-Host "`nVisual Studio 2019 Command Prompt variables set." -ForegroundColor Yellow

$gitBranch = $env:APPVEYOR_REPO_BRANCH
if ($gitBranch -eq "master") {
  $version = ${env:APPVEYOR_BUILD_VERSION}.Split('-')[0]
  $env:HALORAY_VERSION = "${version}"
}

Push-Location
mkdir build
Set-Location src
& "${env:Qt5_DIR}\bin\qmake.exe" main.pro -o ..\build\ -config release
Set-Location ..\build
nmake 2>$null
Pop-Location

Push-Location
Set-Location build\haloray\release
& "${env:Qt5_DIR}\bin\windeployqt.exe" `
  --no-quick-import `
  --no-translations `
  --no-webkit2 `
  --no-angle `
  --no-opengl-sw `
  --no-system-d3d-compiler `
  --no-compiler-runtime `
  --release `
  haloray.exe
Pop-Location
