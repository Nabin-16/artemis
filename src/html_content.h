const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Artemis User App v2.0 (Live GPS)</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }

        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            max-width: 500px;
            width: 100%;
            overflow: hidden;
        }

        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }

        .header h1 {
            font-size: 28px;
            margin-bottom: 10px;
        }

        .status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            background: rgba(255, 255, 255, 0.2);
            font-size: 14px;
            margin-top: 10px;
        }

        .status.connected {
            background: #4caf50;
        }

        .content {
            padding: 30px;
        }

        .section {
            margin-bottom: 25px;
        }

        .section.hidden {
            display: none;
        }

        .section h2 {
            color: #333;
            margin-bottom: 15px;
            font-size: 20px;
        }

        .form-group {
            margin-bottom: 15px;
        }

        .form-group label {
            display: block;
            margin-bottom: 5px;
            color: #666;
            font-weight: 600;
        }

        .form-group input {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }

        .form-group input:focus {
            outline: none;
            border-color: #667eea;
        }

        .btn {
            width: 100%;
            padding: 14px;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
        }

        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }

        .btn-success {
            background: #4caf50;
            color: white;
        }

        .btn-danger {
            background: #f44336;
            color: white;
        }

        .toggle-container {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 15px;
            background: #f5f5f5;
            border-radius: 8px;
            margin-bottom: 20px;
        }

        .toggle-switch {
            position: relative;
            width: 60px;
            height: 30px;
        }

        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }

        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: 0.4s;
            border-radius: 30px;
        }

        .slider:before {
            position: absolute;
            content: "";
            height: 22px;
            width: 22px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: 0.4s;
            border-radius: 50%;
        }

        input:checked+.slider {
            background-color: #4caf50;
        }

        input:checked+.slider:before {
            transform: translateX(30px);
        }

        .data-display {
            background: #f9f9f9;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 10px;
        }

        .data-row {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            padding: 8px;
            background: white;
            border-radius: 5px;
        }

        .data-label {
            font-weight: 600;
            color: #666;
        }

        .data-value {
            color: #333;
        }

        .error-msg {
            color: #f44336;
            margin-top: 10px;
            font-size: 14px;
        }

        .success-msg {
            color: #4caf50;
            margin-top: 10px;
            font-size: 14px;
        }

        .info-box {
            background: #e3f2fd;
            border-left: 4px solid #2196f3;
            padding: 12px;
            border-radius: 5px;
            margin-bottom: 15px;
            font-size: 14px;
            color: #1976d2;
        }
    </style>
</head>

<body>
    <div class="container">
        <div class="header">
            <h1>🛰️ Artemis Tracker</h1>
            <p>Real-time GPS & IMU Data Sharing</p>
            <div class="status" id="connectionStatus">Disconnected</div>
        </div>

        <div class="content">
            <!-- Registration Section -->
            <div class="section" id="registerSection">
                <h2>Register Device</h2>
                <div class="info-box">
                    Connect to ESP32 WiFi network first (esp32 / 1234567890)
                </div>
                <form onsubmit="event.preventDefault(); registerUser();">
                    <div class="form-group">
                        <label>Username</label>
                        <input type="text" id="username" placeholder="Enter your name" required>
                    </div>
                    <div class="form-group">
                        <label>Device ID</label>
                        <input type="text" id="deviceId" placeholder="e.g., PHONE-001" required>
                    </div>
                    <button type="submit" class="btn btn-primary">Register & Connect</button>
                </form>
                <div id="registerError" class="error-msg"></div>
            </div>

            <!-- Main App Section -->
            <div class="section hidden" id="appSection">
                <h2>Device Control</h2>
                
                <div id="secure-warning" class="info-box" style="display:none; background:#fff3cd; color:#856404; border-color:#ffeeba;">
                     ⚠️ <strong>HTTP Detected</strong>: GPS may be blocked. See toggle for fix.
                </div>

                <div class="data-display">
                    <div class="data-row">
                        <span class="data-label">Username:</span>
                        <span class="data-value" id="displayUsername">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Device ID:</span>
                        <span class="data-value" id="displayDeviceId">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Sharing Status:</span>
                        <span class="data-value" id="sharingStatusValue"><span style="color:#f44336">In-active</span></span>
                    </div>
                </div>

                <div class="toggle-container">
                    <div>
                        <strong>Enable Data Sharing</strong>
                        <p style="font-size: 12px; color: #666; margin-top: 5px;">Share GPS & IMU data with dashboard
                        </p>
                        <p id="gpsStatusText" style="font-size: 12px; color: #f44336; margin-top: 5px; font-weight: bold;"></p>
                    </div>
                    <label class="toggle-switch">
                        <input type="checkbox" id="dataSharingToggle" onchange="toggleDataSharing()">
                        <span class="slider"></span>
                    </label>
                </div>

                <h3 style="margin-bottom: 15px;">Live Sensor Data</h3>

                <div class="data-display">
                    <h4 style="color: #667eea; margin-bottom: 10px;">📍 GPS</h4>
                    <div class="data-row">
                        <span class="data-label">Latitude:</span>
                        <span class="data-value" id="gpsLat">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Longitude:</span>
                        <span class="data-value" id="gpsLon">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Altitude:</span>
                        <span class="data-value" id="gpsAlt">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Speed:</span>
                        <span class="data-value" id="gpsSpeed">-</span>
                    </div>
                </div>

                <div class="data-display">
                    <h4 style="color: #764ba2; margin-bottom: 10px;">📊 IMU</h4>
                    <div class="data-row">
                        <span class="data-label">Accel X/Y/Z:</span>
                        <span class="data-value" id="imuAccel">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Gyro X/Y/Z:</span>
                        <span class="data-value" id="imuGyro">-</span>
                    </div>
                    <div class="data-row">
                        <span class="data-label">Mag X/Y/Z:</span>
                        <span class="data-value" id="imuMag">-</span>
                    </div>
                </div>

                <button class="btn btn-danger" onclick="disconnect()">Disconnect</button>
            </div>
        </div>
    </div>

    <script>
        let ws = null;
        let username = '';
        let deviceId = '';
        let dataSharingEnabled = false;

        // Simulate sensor data (in real app, you'd get this from device sensors)
        let sensorInterval = null;

        function debugLog(msg) {
            console.log(msg);
        }

        function connectWebSocket() {
            if(ws) ws.close(); // Close existing if any

            // Dynamic host: connects to 192.168.4.1 (AP) or Router IP (Station) automatically
            const host = window.location.hostname;
            const port = window.location.port ? ':' + window.location.port : '';
            const wsUrl = `ws://${host}${port}/ws`;
            
            console.log("Attempting connection to: " + wsUrl);
            document.getElementById('connectionStatus').textContent = 'Connecting...';
            
            try {
                ws = new WebSocket(wsUrl);

                ws.onopen = () => {
                    console.log("WebSocket Connected!");
                    updateConnectionStatus(true);
                };

                ws.onclose = (e) => {
                    console.log("Closed. Code: " + e.code);
                    updateConnectionStatus(false);
                    // Retry
                    setTimeout(connectWebSocket, 3000);
                };

                ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    updateConnectionStatus(false);
                };

                ws.onmessage = (event) => {
                    try {
                        const data = JSON.parse(event.data);
                        handleMessage(data);
                    } catch (e) {
                        console.error('Error parsing message:', e);
                    }
                };
            } catch (err) {
                console.error("Critical Error: " + err.message);
            }
        }

        function updateConnectionStatus(connected) {
            const statusEl = document.getElementById('connectionStatus');
            if (connected) {
                statusEl.textContent = 'Connected';
                statusEl.classList.add('connected');
                statusEl.style.backgroundColor = '#4caf50';
                statusEl.title = "";
            } else {
                statusEl.textContent = 'Disconnected';
                statusEl.classList.remove('connected');
                statusEl.style.backgroundColor = '';
                statusEl.title = "Connection Lost";
            }
        }

        function updateConnectionStatus(connected, url = '', reason = '') {
            const statusEl = document.getElementById('connectionStatus');
            if (connected) {
                statusEl.textContent = 'Connected';
                statusEl.classList.add('connected');
                statusEl.style.backgroundColor = '#4caf50';
            } else {
                statusEl.textContent = `Disconnected (${reason})`;
                statusEl.classList.remove('connected');
                statusEl.style.backgroundColor = '#f44336';
                
                // Show debug info only when disconnected
                if(url) {
                   statusEl.title = `Failed to connect to: ${url}`;
                }
            }
        }

        function registerUser(isAutoLogin = false) {
            username = document.getElementById('username').value;
            deviceId = document.getElementById('deviceId').value;

            if (!username || !deviceId) {
                 if(isAutoLogin) return; // Silent fail on auto-login
                 alert("Please enter both Username and Device ID");
                 return;
            }

            if (!ws || ws.readyState !== WebSocket.OPEN) {
                const msg = 'Not connected to ESP32. Please connect to WiFi network "esp32"';
                document.getElementById('registerError').textContent = msg;
                if(!isAutoLogin) alert(msg);
                return;
            }

            // Save to LocalStorage
            localStorage.setItem('artemis_username', username);
            localStorage.setItem('artemis_deviceId', deviceId);

            const registerMsg = {
                type: 'REGISTER',
                username: username,
                deviceId: deviceId
            };

            ws.send(JSON.stringify(registerMsg));
        }

        function handleMessage(data) {
            if (data.type === 'REGISTERED') {
                // Registration successful
                document.getElementById('registerSection').classList.add('hidden');
                document.getElementById('appSection').classList.remove('hidden');
                document.getElementById('displayUsername').textContent = username;
                document.getElementById('displayDeviceId').textContent = deviceId;

                // Start simulating sensor data
                startSensorSimulation();
            }
        }

        let watchId = null;

        function toggleDataSharing() {
            dataSharingEnabled = document.getElementById('dataSharingToggle').checked;
            const statusEl = document.getElementById('gpsStatusText');

            // Update sharing status text
            const shareStatusEl = document.getElementById('sharingStatusValue');
            if (dataSharingEnabled) {
                shareStatusEl.innerHTML = '<span style="color:#4caf50">✅ Live (GPS + IMU)</span>';
            } else {
                shareStatusEl.innerHTML = '<span style="color:#f44336">⛔ Paused</span>';
            }

            const msg = {
                type: 'ENABLE_SHARING',
                enabled: dataSharingEnabled
            };

            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(msg));
            }

            if (dataSharingEnabled) {
                // User explicitly asked to enable sharing -> Request GPS
                if ("geolocation" in navigator) {
                    statusEl.textContent = "Requesting GPS access...";
                    statusEl.style.color = "#FF9800"; // Orange

                    watchId = navigator.geolocation.watchPosition(
                        (position) => {
                            statusEl.textContent = "GPS Active - Tracking";
                            statusEl.style.color = "#4caf50"; // Green
                            handleGpsPosition(position);
                        },
                        (error) => {
                            console.error("GPS Error: ", error);
                            let errorMsg = "Unknown Error";
                            switch(error.code) {
                                case error.PERMISSION_DENIED: 
                                    if (!window.isSecureContext) {
                                        errorMsg = "Blocked by Browser (HTTP). Check Chrome Flags.";
                                        alert("🛑 GPS BLOCKED DUE TO HTTP\n\nMobile browsers require HTTPS for GPS.\n\nFIX (Chrome):\n1. Go to 'chrome://flags'\n2. Search 'unsafely-treat-insecure-origin-as-secure'\n3. Enable it\n4. Add this device IP (e.g., http://192.168.4.1)\n5. Relaunch Chrome");
                                    } else {
                                        errorMsg = "Permission Denied! Check Phone Settings."; 
                                    }
                                    break;
                                case error.POSITION_UNAVAILABLE: errorMsg = "Signal Weak / Unavailable."; break;
                                case error.TIMEOUT: errorMsg = "GPS Timeout."; break;
                            }
                            statusEl.textContent = errorMsg;
                            statusEl.style.color = "#f44336"; // Red
                            // If permission denied, maybe uncheck the toggle?
                            if(error.code === error.PERMISSION_DENIED) {
                                document.getElementById('dataSharingToggle').checked = false;
                                dataSharingEnabled = false;
                            }
                        },
                        {
                            enableHighAccuracy: true,
                            maximumAge: 0,
                            timeout: 10000 
                        }
                    );
                } else {
                    alert("Geolocation is not supported by your browser");
                    statusEl.textContent = "GPS Not Supported";
                }
            } else {
                // Stop tracking
                if (watchId !== null) {
                    navigator.geolocation.clearWatch(watchId);
                    watchId = null;
                }
                statusEl.textContent = "";
            }
        }

        // Extracted GPS Handler
        let lastGpsTime = 0;
        function handleGpsPosition(position) {
            const now = Date.now();
            if (now - lastGpsTime < 2000) return; // Throttle
            lastGpsTime = now;

            const gpsData = {
                type: 'GPS',
                username: username,
                deviceId: deviceId,
                timestamp: now,
                lat: position.coords.latitude,
                lon: position.coords.longitude,
                alt: position.coords.altitude || 0,
                accuracy: position.coords.accuracy || 0,
                speed: (position.coords.speed || 0) * 3.6
            };
            
            // UI Update
            document.getElementById('gpsLat').textContent = gpsData.lat.toFixed(6);
            document.getElementById('gpsLon').textContent = gpsData.lon.toFixed(6);
            document.getElementById('gpsAlt').textContent = gpsData.alt.toFixed(1) + ' m';
            document.getElementById('gpsSpeed').textContent = gpsData.speed.toFixed(1) + ' km/h';

            // Send
            if (dataSharingEnabled && ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify(gpsData));
            }
        }

        function startSensorSimulation() {
            // IMU Simulation Logic (Accelerometers)
            // Separate from GPS so it runs even if GPS fails
            if (sensorInterval) clearInterval(sensorInterval);
            
            sensorInterval = setInterval(() => {
                if(!dataSharingEnabled) return; // Only process if enabled

                const imuData = {
                    type: 'IMU',
                    username: username,
                    deviceId: deviceId,
                    timestamp: Date.now(),
                    accel: {
                        x: (Math.random() - 0.5) * 0.2, // Low noise (Stationary)
                        y: (Math.random() - 0.5) * 0.2,
                        z: 9.8 + (Math.random() - 0.5) * 0.2
                    },
                    gyro: {
                        x: (Math.random() - 0.5) * 2,
                        y: (Math.random() - 0.5) * 2,
                        z: (Math.random() - 0.5) * 2
                    },
                    mag: {
                        x: 30 + (Math.random() - 0.5) * 5,
                        y: -15 + (Math.random() - 0.5) * 5,
                        z: -40 + (Math.random() - 0.5) * 5
                    }
                };
                
                // Occasional "Fall" Simulation for testing (every ~30s)
                /*
                if (Math.random() < 0.05) {
                    imuData.accel.z = 28.0; // Spike > 25 m/s^2
                }
                */

                // UI Update
                document.getElementById('imuAccel').textContent =
                    `${imuData.accel.x.toFixed(2)}, ${imuData.accel.y.toFixed(2)}, ${imuData.accel.z.toFixed(2)} m/s²`;
                document.getElementById('imuGyro').textContent =
                    `${imuData.gyro.x.toFixed(2)}, ${imuData.gyro.y.toFixed(2)}, ${imuData.gyro.z.toFixed(2)} °/s`;
                document.getElementById('imuMag').textContent =
                    `${imuData.mag.x.toFixed(1)}, ${imuData.mag.y.toFixed(1)}, ${imuData.mag.z.toFixed(1)} µT`;

                // Send
                if (ws && ws.readyState === WebSocket.OPEN) {
                    ws.send(JSON.stringify(imuData));
                }
            }, 1000);
        }

        function disconnect() {
            if (sensorInterval) {
                clearInterval(sensorInterval);
            }
            // Clear LocalStorage on explicit logout
            localStorage.removeItem('artemis_username');
            localStorage.removeItem('artemis_deviceId');
            
            if (ws) {
                ws.close();
            }
            // Reset to registration screen
            document.getElementById('registerSection').classList.remove('hidden');
            document.getElementById('appSection').classList.add('hidden');
            document.getElementById('dataSharingToggle').checked = false;
            dataSharingEnabled = false;
            
            // Clear inputs
            document.getElementById('username').value = '';
            document.getElementById('deviceId').value = '';
        }

        // Initialize WebSocket on page load
        window.addEventListener('load', () => {
            connectWebSocket();

            // Check Secure Context
            if (!window.isSecureContext) {
                const warningEl = document.getElementById('secure-warning');
                if(warningEl) warningEl.style.display = 'block';
            }
            
            // Check for saved credentials
            const savedUser = localStorage.getItem('artemis_username');
            const savedDevice = localStorage.getItem('artemis_deviceId');
            
            if (savedUser && savedDevice) {
                document.getElementById('username').value = savedUser;
                document.getElementById('deviceId').value = savedDevice;
                
                // Try to auto-login when socket connects
                const checkSocket = setInterval(() => {
                    if (ws && ws.readyState === WebSocket.OPEN) {
                        registerUser(true); // true = isAutoLogin
                        clearInterval(checkSocket);
                    }
                }, 500);
            }
        });
    </script>
</body>

</html>
)rawliteral";
