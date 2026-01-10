#!/bin/bash
# Artemis Flask Server - Linux/Mac Startup Script
# This script starts the Flask server for receiving ESP32 GPS/IMU data

echo "================================================"
echo "   Artemis Flask Server - Starting..."
echo "================================================"
echo

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if Python is available
if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 is not installed or not in PATH"
    echo "Please install Python 3.7+ from your package manager"
    exit 1
fi

# Check if requirements are installed
echo "Checking dependencies..."
if ! python3 -c "import flask" &> /dev/null; then
    echo "Installing dependencies..."
    if ! pip3 install -r flask_server/requirements.txt; then
        echo "ERROR: Failed to install dependencies"
        exit 1
    fi
fi

echo
echo "Starting Flask server on http://0.0.0.0:5000"
echo "Press Ctrl+C to stop the server"
echo
echo "================================================"
echo

# Start the Flask server
cd flask_server
python3 app.py
