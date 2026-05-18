param(
    [string]$MainTex = "main.tex",
    [string]$OutDir = "build",
    [int]$MaxInstallRounds = 25
)

$ErrorActionPreference = "Stop"

$miktexBin = "C:\Users\HP\AppData\Local\Programs\MiKTeX\miktex\bin\x64"
$pdflatex = Join-Path $miktexBin "pdflatex.exe"
$bibtex = Join-Path $miktexBin "bibtex.exe"
$miktex = Join-Path $miktexBin "miktex.exe"
$initexmf = Join-Path $miktexBin "initexmf.exe"

foreach ($exe in @($pdflatex, $bibtex, $miktex, $initexmf)) {
    if (-not (Test-Path $exe)) {
        throw "Missing executable: $exe"
    }
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# 2 = always install missing packages.
& $initexmf --set-config-value="[MPM]AutoInstall=2" | Out-Null
& $initexmf --enable-installer | Out-Null

function Invoke-PdfLatex {
    param([string]$texFile, [string]$outputDir)
    & $pdflatex -interaction=nonstopmode -file-line-error -output-directory=$outputDir $texFile
    return $LASTEXITCODE
}

function Install-MissingPackageFromLog {
    param([string]$logPath)
    if (-not (Test-Path $logPath)) { return $false }

    $content = Get-Content -Path $logPath -Raw
    $m = [regex]::Match($content, "LaTeX Error: File `([^']+)' not found\.")
    if (-not $m.Success) { return $false }

    $missingFile = $m.Groups[1].Value
    $pkg = [System.IO.Path]::GetFileNameWithoutExtension($missingFile)
    if ([string]::IsNullOrWhiteSpace($pkg)) { return $false }

    Write-Host "[AUTO-INSTALL] missing file: $missingFile -> package: $pkg"
    & $miktex packages install $pkg
    return $true
}

$baseName = [System.IO.Path]::GetFileNameWithoutExtension($MainTex)
$logFile = Join-Path $OutDir ($baseName + ".log")

$round = 0
while ($round -lt $MaxInstallRounds) {
    $round++
    Write-Host "[ROUND $round] pdflatex pass for dependency resolution..."
    $code = Invoke-PdfLatex -texFile $MainTex -outputDir $OutDir
    if ($code -eq 0) { break }

    $installed = Install-MissingPackageFromLog -logPath $logFile
    if (-not $installed) {
        throw "pdflatex failed and no installable missing-package pattern was found. See $logFile"
    }
}

if ($round -ge $MaxInstallRounds) {
    throw "Reached max install rounds ($MaxInstallRounds)."
}

Write-Host "[FINAL] bibtex + pdflatex x2..."
& $bibtex (Join-Path $OutDir $baseName)
& $pdflatex -interaction=nonstopmode -file-line-error -output-directory=$OutDir $MainTex
& $pdflatex -interaction=nonstopmode -file-line-error -output-directory=$OutDir $MainTex

if ($LASTEXITCODE -ne 0) {
    throw "Final pdflatex pass failed. See $logFile"
}

Write-Host "[OK] PDF generated at $OutDir\$baseName.pdf"
