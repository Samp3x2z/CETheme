$ErrorActionPreference = "Stop"

# Build paths are always resolved from this script, never from the caller's
# current directory. The repository can therefore live anywhere on the PC.
$repoRoot = [System.IO.Path]::GetFullPath($PSScriptRoot)
$project = Join-Path $repoRoot "ThemedCET"
$outputRoot = Join-Path $repoRoot "build"
$stagingRoot = Join-Path $repoRoot ".build-staging"
$packageRoot = Join-Path $project "package"
$packageBin = Join-Path $packageRoot "bin"
$exitCode = 0
$sdkCommit = "5246e659d783e5834cc08a90e56c8932c9ed3449"

function Invoke-Xmake {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,

        [Parameter(Mandatory = $true)]
        [string]$Step
    )

    & xmake @Arguments
    $nativeExitCode = $LASTEXITCODE

    if ($nativeExitCode -ne 0) {
        throw "$Step failed (xmake exit code $nativeExitCode)."
    }
}

try {
    Write-Host "Repository : $repoRoot" -ForegroundColor DarkGray
    Write-Host "Project    : $project" -ForegroundColor DarkGray

    if (-not (Test-Path (Join-Path $project "xmake.lua"))) {
        throw "The ThemedCET project was not found. Run this script from the repository root."
    }

    if (-not (Get-Command xmake -ErrorAction SilentlyContinue)) {
        throw "Xmake is required. Install Xmake 3.1 or newer, then reopen the terminal."
    }

    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "Git for Windows is required because the version metadata is generated from Git."
    }

    # xmake.lua reads Git metadata while loading the project.
    & git -C $repoRoot rev-parse --is-inside-work-tree *> $null
    if ($LASTEXITCODE -ne 0) {
        throw "This source folder is not a Git working tree. Clone the repository with Git instead of downloading the ZIP archive."
    }

    # Restore the exact RED4ext.SDK revision pinned by the official CET project.
    # The SDK is a Git submodule so its generated headers cannot be accidentally
    # left out of a normal clone or replaced by a partial vendored copy.
    Write-Host "`n[0/5] Initializing the official RED4ext SDK submodule..." -ForegroundColor Cyan
    & git -C $repoRoot submodule sync --recursive
    if ($LASTEXITCODE -ne 0) {
        throw "Git could not synchronize the RED4ext SDK submodule URL."
    }

    & git -C $repoRoot submodule update --init --recursive -- ThemedCET/vendor/RED4ext.SDK
    if ($LASTEXITCODE -ne 0) {
        throw "Git could not download/initialize ThemedCET/vendor/RED4ext.SDK. Check your internet connection and GitHub access."
    }

    $sdkPath = Join-Path $project "vendor\RED4ext.SDK"
    if (-not (Test-Path (Join-Path $sdkPath "include\RED4ext\SharedSpinLock.hpp"))) {
        throw "The RED4ext SDK submodule is incomplete. Expected include/RED4ext/SharedSpinLock.hpp."
    }

    $sdkActualCommit = (& git -C $sdkPath rev-parse HEAD).Trim()
    if ($LASTEXITCODE -ne 0 -or $sdkActualCommit -ne "$sdkCommit") {
        throw "RED4ext SDK revision mismatch. Expected $sdkCommit, found $sdkActualCommit."
    }

    # Never leave an old staging folder behind after an interrupted build.
    if (Test-Path $stagingRoot) {
        Remove-Item $stagingRoot -Recurse -Force
    }

    Push-Location $project
    try {
        Write-Host "`n[1/5] Resetting Xmake configuration for this checkout..." -ForegroundColor Cyan
        # Re-evaluate path.absolute() in xmake.lua. This is important after moving
        # the repository: the ImGui package must not retain the previous checkout path.
        Invoke-Xmake -Arguments @("f", "-c", "-m", "release", "-y") -Step "Xmake configuration"

        Write-Host "`n[2/5] Refreshing ImGui with the current repository path..." -ForegroundColor Cyan
        # ImGui is built with src/imgui_impl/imgui_user_config.h. Force its package
        # rebuild so a cached package compiled from an older absolute path is not reused.
        Invoke-Xmake -Arguments @("require", "-f", "-y", "imgui") -Step "ImGui dependency installation"

        Write-Host "`n[3/5] Cleaning previous compilation outputs..." -ForegroundColor Cyan
        Invoke-Xmake -Arguments @("clean", "-a") -Step "Xmake clean"

        Write-Host "`n[4/5] Compiling Cyber Engine Tweaks (Release, x64)..." -ForegroundColor Cyan
        Invoke-Xmake -Arguments @("-y") -Step "Compilation"

        Write-Host "`n[5/5] Creating the install package..." -ForegroundColor Cyan
        Invoke-Xmake -Arguments @("package", "-y") -Step "Package creation"

        # Validate the complete package before replacing the last known-good build.
        if (-not (Test-Path (Join-Path $packageBin "x64\version.dll"))) {
            throw "The package is missing x64/version.dll."
        }

        $pluginsPath = Join-Path $packageBin "x64\plugins"
        $asiPath = Join-Path $pluginsPath "cyber_engine_tweaks.asi"
        $darkThemePath = Join-Path $pluginsPath "cyber_engine_tweaks\themes\Dark.json"

        if (-not (Test-Path $asiPath)) {
            throw "The package is missing x64/plugins/cyber_engine_tweaks.asi."
        }

        if (-not (Test-Path $darkThemePath)) {
            throw "The package is missing the default CET theme (Dark.json)."
        }

        # Stage the new output first. A failed build/package therefore never deletes
        # the previous successful build/ directory.
        New-Item -ItemType Directory -Force -Path (Join-Path $stagingRoot "bin") | Out-Null
        Copy-Item -Path (Join-Path $packageBin "*") -Destination (Join-Path $stagingRoot "bin") -Recurse -Force

        if (-not (Test-Path (Join-Path $stagingRoot "bin\x64\plugins\cyber_engine_tweaks.asi"))) {
            throw "The staged install package is incomplete; the ASI file was not copied."
        }

        if (Test-Path $outputRoot) {
            Remove-Item $outputRoot -Recurse -Force
        }

        Move-Item -Path $stagingRoot -Destination $outputRoot

        Write-Host "`nBUILD SUCCEEDED" -ForegroundColor Green
        Write-Host "Install-ready files: $outputRoot\bin" -ForegroundColor Green
        Write-Host "Copy the contents of build\bin into the Cyberpunk 2077 game directory."
    }
    finally {
        Pop-Location
    }
}
catch {
    $exitCode = 1
    Write-Host "`nBUILD FAILED" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host "The previous successful build, if any, has been preserved."
    Write-Host "For detailed dependency/compiler logs, check Xmake's output above and its logs under $env:LOCALAPPDATA\.xmake."
}
finally {
    if (Test-Path $stagingRoot) {
        Remove-Item $stagingRoot -Recurse -Force -ErrorAction SilentlyContinue
    }

    Write-Host ""
    if ($Host.Name -eq "ConsoleHost") {
        Read-Host "Press Enter to close this build window"
    }
}

exit $exitCode
