# Windows artifact packaging script for Appveyor
$ErrorActionPreference = "Stop";

$destination = "${env:APPVEYOR_BUILD_FOLDER}\haloray-v${env:APPVEYOR_BUILD_VERSION}.zip"
$buildLocation = "${env:APPVEYOR_BUILD_FOLDER}\build\src\Release\*"
7z a $destination $buildLocation