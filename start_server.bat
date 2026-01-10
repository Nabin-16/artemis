@echo off
REM Artemis Flask Server - Windows Startup Script
REM This script starts the Flask server for receiving ESP32 GPS/IMU data

echo ================================================
echo    Artemis Flask Server - Starting...
echo ================================================
echo.

cd /d "%~dp0"

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python 3.7+ from https://www.python.org/
    pause
    exit /b 1
)

REM Check if requirements are installed
echo Checking dependencies...
pip show flask >nul 2>&1
if errorlevel 1 (
    echo Installing dependencies...
    pip install -r flask_server\requirements.txt
    if errorlevel 1 (
        echo ERROR: Failed to install dependencies
        pause
        exit /b 1
    )
)

echo.
echo Starting Flask server on http://0.0.0.0:5000
echo Press Ctrl+C to stop the server
echo.
echo ================================================
echo.

REM Start the Flask server
cd flask_server
python app.py

pause
