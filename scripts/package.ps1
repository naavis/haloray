# Windows artifact packaging script for Appveyor
$ErrorActionPreference = "Stop";

$version = if ($env:APPVEYOR_REPO_BRANCH -eq "master") {
    "v${env:APPVEYOR_BUILD_VERSION}"
} else {
    "${env:APPVEYOR_REPO_COMMIT}"
}

$destination = "${env:APPVEYOR_BUILD_FOLDER}\haloray-${version}.zip"
$buildLocation = "${env:APPVEYOR_BUILD_FOLDER}\build\src\Release\*"

7z a $destination $buildLocation
7z a $destination "${env:APPVEYOR_BUILD_FOLDER}\*.md"