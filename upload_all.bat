@echo off
setlocal

REM Upload firmware + web UI (LittleFS) to ESP32.
REM Optional: pass COM port as first argument, e.g. upload_all.bat COM3
REM If omitted, the currently connected ESP32 serial port is detected automatically.
set "UPLOAD_PORT=%~1"
set "SCRIPT_DIR=%~dp0"

for /f "usebackq delims=" %%P in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%tools\Resolve-UploadPort.ps1" -Port "%UPLOAD_PORT%"`) do set "UPLOAD_PORT=%%P"
if errorlevel 1 exit /b %errorlevel%

echo Using upload port: %UPLOAD_PORT%

pio run -t upload --upload-port %UPLOAD_PORT%
if errorlevel 1 exit /b %errorlevel%

pio run -t uploadfs --upload-port %UPLOAD_PORT%
if errorlevel 1 exit /b %errorlevel%

powershell -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT_DIR%tools\Print-DevicePasswords.ps1" -Port "%UPLOAD_PORT%"
