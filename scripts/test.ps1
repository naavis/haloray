# Windows test script for Appveyor

$ErrorActionPreference = "Stop";

$env:APPVEYOR_JOB_ID

pushd
cd build
ctest --no-compress-output -T Test
$testReport = Get-ChildItem -Path .\Testing\*\Test.xml | Select-Object -First 1
$testReport
$wc = New-Object 'System.Net.WebClient'
$wc
$res = $wc.UploadFile("https://ci.appveyor.com/api/testresults/xunit/$($env:APPVEYOR_JOB_ID)", $testReport.FullName)
$res
popd
