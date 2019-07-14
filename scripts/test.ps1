# Windows test script for Appveyor

$ErrorActionPreference = "Stop";

pushd
cd build
ctest --no-compress-output -T Test
$testReport = Get-ChildItem -Path .\Testing\*\Test.xml | Select-Object -First 1
$wc = New-Object System.Net.WebClient
$wc.UploadFile("https://ci.appveyor.com/api/testresults/xunit/$($env:APPVEYOR_JOB_ID)", $testReport.FullName)
popd