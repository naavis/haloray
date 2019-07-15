# Windows test script for Appveyor

$ErrorActionPreference = "Stop";

$env:APPVEYOR_JOB_ID

pushd
cd build
ctest --no-compress-output -T Test
$testReport = Get-ChildItem -Path .\Testing\*\Test.xml | Select-Object -First 1
$testReport

Write-host "test file path:"
Write-host (Resolve-Path $testReport.FullName)

$XSLInputElement = New-Object System.Xml.Xsl.XslCompiledTransform
$XSLInputElement.Load("https://raw.githubusercontent.com/rpavlik/jenkins-ctest-plugin/master/ctest-to-junit.xsl")

Write-host "transforming..."
$XSLInputElement.Transform($(Resolve-Path $testReport.FullName), (Join-Path (Resolve-Path .) "ctest-to-junit-results.xml"))

type ctest-to-junit-results.xml

$wc = New-Object 'System.Net.WebClient'
$wc.UploadFile("https://ci.appveyor.com/api/testresults/junit/$($env:APPVEYOR_JOB_ID)", (Resolve-Path .\ctest-to-junit-results.xml))

if ($res.FailedCount -gt 0) {
    throw "$($res.FailedCount) tests failed."
}
popd
