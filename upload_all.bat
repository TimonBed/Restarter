@echo off
REM Upload firmware + web UI (LittleFS) to ESP32
pio run -t upload
if %errorlevel% neq 0 exit /b %errorlevel%
pio run -t uploadfs
