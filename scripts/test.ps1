# Windows test script for Appveyor

pushd "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build"
cmd /c "vcvars64.bat&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd
Write-Host "`nVisual Studio 2019 Command Prompt variables set." -ForegroundColor Yellow

pushd
cd build
nmake check TESTARGS="-o xunit-results.xml,xunitxml" 2>$null

foreach ($testReport in Get-ChildItem -Path xunit-results.xml -Recurse) {
    $wc = New-Object 'System.Net.WebClient'
    $wc.UploadFile("https://ci.appveyor.com/api/testresults/junit/$($env:APPVEYOR_JOB_ID)", $testReport.FullName)
}

popd
