# Windows test script for Appveyor

$ErrorActionPreference = "Stop";

$XSLInputElement = New-Object System.Xml.Xsl.XslCompiledTransform
$XSLInputElement.Load($(Join-Path $env:APPVEYOR_BUILD_FOLDER "scripts\ctest-to-junit.xsl"))

pushd
cd build
ctest --no-compress-output -T Test
foreach ($testReport in Get-ChildItem -Path .\Testing\*\Test.xml)
{
    $XSLInputElement.Transform($(Resolve-Path $testReport.FullName), (Join-Path (Resolve-Path .) "ctest-to-junit-results.xml"))
    $wc = New-Object 'System.Net.WebClient'
    $wc.UploadFile("https://ci.appveyor.com/api/testresults/junit/$($env:APPVEYOR_JOB_ID)", (Resolve-Path .\ctest-to-junit-results.xml))

    if ($res.FailedCount -gt 0) {
        throw "$($res.FailedCount) tests failed."
    }
}
popd
