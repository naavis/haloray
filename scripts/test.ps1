# Windows test script for Appveyor

$ErrorActionPreference = "Stop";

$env:APPVEYOR_JOB_ID

pushd
cd build
ctest --no-compress-output -T Test
$testReport = Get-ChildItem -Path .\Testing\*\Test.xml | Select-Object -First 1
$testReport
$res = (New-Object 'System.Net.WebClient').UploadFile("https://ci.appveyor.com/api/testresults/xunit/$($env:APPVEYOR_JOB_ID)", $testReport.FullName)
$res
if ($res.FailedCount -gt 0) {
    throw "$($res.FailedCount) tests failed."
}
popd
