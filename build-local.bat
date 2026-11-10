@echo off
setlocal
cd /d "%~dp0"

where pwsh >nul 2>&1
if %errorlevel%==0 (
    pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-local.ps1" %*
) else (
    powershell -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-local.ps1" %*
)

exit /b %errorlevel%
