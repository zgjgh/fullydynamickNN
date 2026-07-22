$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$outputDir = Join-Path $projectRoot "build_vldbj"
$outputName = "main_vldbj"
$sourceFile = "main_vldbj_working.tex"
$pdflatex = "C:\Users\HP\AppData\Local\Programs\MiKTeX\miktex\bin\x64\pdflatex.exe"
$bibtex = "C:\Users\HP\AppData\Local\Programs\MiKTeX\miktex\bin\x64\bibtex.exe"

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
Push-Location $projectRoot

try {
    & $pdflatex -interaction=nonstopmode -file-line-error "-jobname=$outputName" -output-directory=build_vldbj $sourceFile
    if ($LASTEXITCODE -ne 0) { throw "Initial pdflatex pass failed." }

    & $bibtex build_vldbj/main_vldbj
    if ($LASTEXITCODE -ne 0) { throw "BibTeX pass failed." }

    & $pdflatex -interaction=nonstopmode -file-line-error "-jobname=$outputName" -output-directory=build_vldbj $sourceFile
    if ($LASTEXITCODE -ne 0) { throw "Second pdflatex pass failed." }

    & $pdflatex -interaction=nonstopmode -file-line-error "-jobname=$outputName" -output-directory=build_vldbj $sourceFile
    if ($LASTEXITCODE -ne 0) { throw "Final pdflatex pass failed." }

    Write-Host "VLDBJ working PDF: $outputDir\main_vldbj.pdf"
}
finally {
    Pop-Location
}
