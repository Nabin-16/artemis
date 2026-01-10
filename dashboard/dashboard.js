// Real-time device tracking with Socket.IO (Flask Server)
let devices = new Map();
let socket = null;
let countdownInterval = null;
let currentAlertDevice = null;

// ====== CONFIGURATION ======
// Change this to your laptop's IP address where Flask server is running
const FLASK_SERVER_URL = 'http://localhost:5000';  // Change to your laptop's IP if needed
// Example: 'http://192.168.1.100:5000'

// Initialize Map
const map = L.map('map').setView([27.7000, 85.3000], 12);

// Dark Theme Map Tiles
L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors &copy; <a href="https://carto.com/attributions">CARTO</a>',
    subdomains: 'abcd',
    maxZoom: 20
}).addTo(map);

// Markers storage
const markers = new Map();

// DOM Elements
const deviceListEl = document.getElementById('deviceListWrapper');
const totalEl = document.getElementById('totalDevices');
const onlineEl = document.getElementById('onlineDevices');
const offlineEl = document.getElementById('offlineDevices');
const activeCountEl = document.getElementById('activeCount');
const esp32StatusEl = document.getElementById('esp32Status');

// Initialize Socket.IO connection to Flask server
function initWebSocket() {
    socket = io(FLASK_SERVER_URL, {
        transports: ['websocket', 'polling']
    });

    socket.on('connect', () => {
        console.log('✅ Connected to Flask Server');
        esp32StatusEl.textContent = 'Connected';
        esp32StatusEl.style.color = '#69f0ae';
    });

    socket.on('disconnect', () => {
        console.log('❌ Disconnected from Flask Server');
        esp32StatusEl.textContent = 'Disconnected';
        esp32StatusEl.style.color = '#ff5252';
    });

    socket.on('connection_status', (data) => {
        console.log('Connection status:', data);
    });

    socket.on('active_devices', (data) => {
        console.log('Active devices:', data);
        // Load existing devices if any
        if (data.devices && Array.isArray(data.devices)) {
            data.devices.forEach(device => {
                addUser(device.username, device.deviceId, device.clientId || 0);
            });
        }
    });

    socket.on('device_registered', (data) => {
        console.log('Device registered:', data);
        addUser(data.username, data.deviceId, data.clientId || 0);
    });

    socket.on('gps_update', (data) => {
        console.log('GPS update:', data);
        updateGPS(data);
    });

    socket.on('imu_update', (data) => {
        console.log('IMU update:', data);
        updateIMU(data);
    });

    socket.on('device_disconnected', (data) => {
        console.log('Device disconnected:', data);
        showDisconnectAlert(data);
    });
}

// Handle incoming messages (legacy function - now handled by socket.on handlers)
function handleMessage(data) {
    console.log('Message received:', data);
    // This function is kept for compatibility but is no longer used
    // All message handling is now done through Socket.IO event handlers
}

// Add new user
function addUser(username, deviceId, clientId) {
    if (!devices.has(deviceId)) {
        devices.set(deviceId, {
            username: username,
            deviceId: deviceId,
            clientId: clientId,
            lat: null,
            lon: null,
            lastUpdate: Date.now(),
            online: true,
            battery: 100,
            imu: { alpha: 0, beta: 0, gamma: 0 }
        });

        console.log(`✅ User added: ${username} (${deviceId})`);
        renderDashboard();
    }
}

// Remove user
function removeUser(deviceId) {
    if (devices.has(deviceId)) {
        // Remove marker from map
        if (markers.has(deviceId)) {
            map.removeLayer(markers.get(deviceId));
            markers.delete(deviceId);
        }

        devices.delete(deviceId);
        console.log(`❌ User removed: ${deviceId}`);
        renderDashboard();
    }
}

// Update GPS data
function updateGPS(data) {
    if (devices.has(data.deviceId)) {
        const device = devices.get(data.deviceId);
        device.lat = data.gps.lat;
        device.lon = data.gps.lon;
        device.lastUpdate = Date.now();
        device.online = true;

        updateMarker(data.deviceId, device);
        renderDashboard();
    }
}

// Update IMU data
function updateIMU(data) {
    if (devices.has(data.deviceId)) {
        const device = devices.get(data.deviceId);
        device.imu = {
            alpha: data.imu.alpha || 0,
            beta: data.imu.beta || 0,
            gamma: data.imu.gamma || 0
        };
        device.lastUpdate = Date.now();
    }
}

// Update map marker
function updateMarker(deviceId, device) {
    if (!device.lat || !device.lon) return;

    const markerColor = device.online ? '#22c55e' : '#ef4444';

    // Remove old marker
    if (markers.has(deviceId)) {
        map.removeLayer(markers.get(deviceId));
    }

    // Create new marker
    const circle = L.circleMarker([device.lat, device.lon], {
        radius: 10,
        fillColor: markerColor,
        color: '#fff',
        weight: 2,
        opacity: 1,
        fillOpacity: 0.8
    }).addTo(map);

    circle.bindPopup(`
        <div style="color: #333;">
            <b>${device.username}</b><br>
            <small>Device: ${device.deviceId}</small><br>
            Status: ${device.online ? '🟢 Online' : '🔴 Offline'}<br>
            IMU: α:${device.imu.alpha.toFixed(1)}° β:${device.imu.beta.toFixed(1)}° γ:${device.imu.gamma.toFixed(1)}°
        </div>
    `);

    markers.set(deviceId, circle);
}

// Show disconnect alert
function showDisconnectAlert(data) {
    currentAlertDevice = data.deviceId;

    document.getElementById('alertUsername').textContent = data.username;
    document.getElementById('alertDeviceId').textContent = data.deviceId;
    document.getElementById('alertModal').classList.add('show');

    // Start countdown
    let countdown = 60;
    const countdownEl = document.getElementById('countdown');

    if (countdownInterval) clearInterval(countdownInterval);

    countdownInterval = setInterval(() => {
        countdown--;
        countdownEl.textContent = countdown;

        if (countdown <= 0) {
            clearInterval(countdownInterval);
            hideDisconnectAlert();
        }
    }, 1000);

    // Play alert sound (optional)
    try {
        const audio = new Audio('data:audio/wav;base64,UklGRnoGAABXQVZFZm10IBAAAAABAAEAQB8AAEAfAAABAAgAZGF0YQoGAACBhYqFbF1fdJivrJBhNjVgodDbq2EcBj+a2/LDciUFLIHO8tiJNwgZaLvt559NEAxQp+PwtmMcBjiR1/LMeSwFJHfH8N2QQAoUXrTp66hVFApGn+DyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwtGnuDyvmwhBSuBzvLZiTYIGmi77OibTRAJS6Xt7rReEwg=');
        audio.play();
    } catch (e) { }
}

// Hide disconnect alert
function hideDisconnectAlert() {
    document.getElementById('alertModal').classList.remove('show');
    if (countdownInterval) {
        clearInterval(countdownInterval);
        countdownInterval = null;
    }
    currentAlertDevice = null;
}

// Respond to disconnect
function respondToDisconnect(isFalseAlarm) {
    if (!currentAlertDevice || !socket) {
        hideDisconnectAlert();
        return;
    }

    const response = {
        type: 'ADMIN_RESPONSE',
        deviceId: currentAlertDevice,
        falseAlarm: isFalseAlarm,
        timestamp: Date.now()
    };

    socket.emit('admin_response', response);
    hideDisconnectAlert();
}

// Function to render dashboard
function renderDashboard() {
    deviceListEl.innerHTML = '';

    let onlineCount = 0;

    devices.forEach((device) => {
        if (device.online) onlineCount++;

        // Create Sidebar Card
        const card = document.createElement('div');
        card.className = `device-card ${!device.online ? 'out-of-range' : ''}`;
        card.onclick = () => {
            if (device.lat && device.lon) {
                map.flyTo([device.lat, device.lon], 15);
                if (markers.has(device.deviceId)) {
                    markers.get(device.deviceId).openPopup();
                }
            }
        };

        const statusClass = device.online ? 'online' : 'offline';
        const statusText = device.online ? 'ONLINE' : 'OFFLINE';

        const timeSince = Math.floor((Date.now() - device.lastUpdate) / 1000);
        const lastSeen = timeSince < 10 ? 'Just now' : `${timeSince}s ago`;

        card.innerHTML = `
            <div class="card-header">
                <span class="device-name">${device.username} <span class="user-badge">${device.deviceId}</span></span>
                <span class="status-badge ${statusClass}">${statusText}</span>
            </div>
            <div class="card-body">
                ${device.lat && device.lon ? `
                    <div class="info-row">
                        <span>📍 Coordinates:</span>
                        <span style="font-family: monospace;">${device.lat.toFixed(6)}, ${device.lon.toFixed(6)}</span>
                    </div>
                ` : '<div class="info-row"><span>📍 Waiting for GPS...</span></div>'}
                <div class="info-row">
                    <span>🧭 IMU:</span>
                    <span style="font-family: monospace;">α:${device.imu.alpha.toFixed(1)}° β:${device.imu.beta.toFixed(1)}° γ:${device.imu.gamma.toFixed(1)}°</span>
                </div>
                <span class="last-seen">Updated: ${lastSeen}</span>
            </div>
        `;

        deviceListEl.appendChild(card);
    });

    // Update Stats
    if (totalEl) totalEl.innerText = devices.size;
    if (onlineEl) onlineEl.innerText = onlineCount;
    if (offlineEl) offlineEl.innerText = devices.size - onlineCount;
    if (activeCountEl) activeCountEl.innerText = onlineCount;
}

// Initialize
initWebSocket();
renderDashboard();

// Refresh dashboard every 2 seconds
setInterval(renderDashboard, 2000);
