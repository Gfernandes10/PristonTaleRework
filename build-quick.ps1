<#
.SYNOPSIS
    Build rápido local - apenas compilação e testes

.DESCRIPTION
    Compila o projeto e executa testes sem fazer pull do LFS ou limpar builds

.PARAMETER SkipTests
    Pula a execução de testes

.EXAMPLE
    .\build-quick.ps1
    .\build-quick.ps1 -SkipTests
#>

param(
    [switch]$SkipTests
)

$ErrorActionPreference = "Stop"

# ====================================
# Configurações
# ====================================
$UE_ROOT = "D:\UE_5.6"
$PROJECT_PATH = "$PSScriptRoot\PristonTaleRework.uproject"
$WORKSPACE = $PSScriptRoot

# ====================================
# Step 1: Compile Unreal Project
# ====================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Compiling Project" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

Write-Host "Starting compilation..."
Write-Host "Project: $PROJECT_PATH"

& "$UE_ROOT\Engine\Build\BatchFiles\Build.bat" `
  PristonTaleReworkEditor `
  Win64 `
  Development `
  -Project="$PROJECT_PATH" `
  -WaitMutex `
  -FromMsBuild

if ($LASTEXITCODE -ne 0) {
  Write-Error "Compilation failed!"
  exit 1
}

Write-Host "Compilation completed!" -ForegroundColor Green

# ====================================
# Step 2: Run System Tests
# ====================================
if (-Not $SkipTests) {
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "Running System Tests" -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan

    Write-Host "[TESTS] Starting system tests (no shaders)..."

    $testResultsPath = "$WORKSPACE\TestResults"
    if (-Not (Test-Path $testResultsPath)) {
        New-Item -ItemType Directory -Path $testResultsPath -Force | Out-Null
    }

    & "$UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
      "$PROJECT_PATH" `
      -ExecCmds="Automation RunTests Project.Functional Tests" `
      -TestExit="Automation Test Queue Empty" `
      -ReportOutputPath="$testResultsPath" `
      -NullRHI `
      -Unattended `
      -NoSplash `
      -NoSound `
      -Stdout `
      -AllowStdOutLogVerbosity `
      -Log="AutomationTest.log"

    if ($LASTEXITCODE -ne 0) {
      Write-Host "[WARNING] Tests failed with exit code: $LASTEXITCODE" -ForegroundColor Yellow
    } else {
      Write-Host "[OK] Tests passed" -ForegroundColor Green
    }
}

# ====================================
# Step 3: Display Summary
# ====================================
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "Build Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$branch = git rev-parse --abbrev-ref HEAD 2>$null
$commit = git rev-parse --short HEAD 2>$null

if ($branch) {
    Write-Host "Branch:  $branch" -ForegroundColor White
}
if ($commit) {
    Write-Host "Commit:  $commit" -ForegroundColor White
}

$reportPath = "$WORKSPACE\TestResults\index.json"

if (Test-Path $reportPath) {
  try {
    $report = Get-Content $reportPath -Raw | ConvertFrom-Json

    Write-Host "`nTests:" -ForegroundColor White
    Write-Host "  Total:  $($report.totalTests)" -ForegroundColor White
    Write-Host "  Passed: $($report.passed)" -ForegroundColor Green
    
    if ($report.failed -gt 0) {
        Write-Host "  Failed: $($report.failed)" -ForegroundColor Red
    } else {
        Write-Host "  Failed: $($report.failed)" -ForegroundColor Green
    }
  } catch {
    Write-Host "Could not parse test results" -ForegroundColor Yellow
  }
}

Write-Host "========================================`n" -ForegroundColor Cyan
Write-Host "Build completed successfully!" -ForegroundColor Green