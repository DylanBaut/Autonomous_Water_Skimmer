const express = require('express');
const { SerialPort, ReadlineParser } = require('serialport');
const http = require('http');
const { Server } = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Serve static files from the 'public' folder
app.use(express.static('public'));

// Setup SerialPort
const port = new SerialPort({
    path: '/dev/cu.usbmodem141201', // Update with your Arduino's port
    baudRate: 9600,
});

const parser = port.pipe(new ReadlineParser({ delimiter: '\n' }));

// IMU and GPS data (mocked for now)
let imuData = { x: 0, y: 0, z: 0 };
let gpsData = { lat: 0.0, lon: 0.0 };

// // Handle incoming data from Arduino
// parser.on('data', (data) => {
//     console.log(`Arduino: ${data.trim()}`);
//     // Broadcast data to all connected clients
//     io.emit('fromArduino', data.trim());
// });
// Handle incoming data from Arduino
parser.on('data', (data) => {
    console.log(`Arduino: ${data.trim()}`);

    // Check if data contains throttle information
    const throttleMatch = data.match(/LEFT:(\d+)_RIGHT:(\d+)/);
    if (throttleMatch) {
        const leftThrottle = parseInt(throttleMatch[1], 10);
        const rightThrottle = parseInt(throttleMatch[2], 10);
        io.emit('throttleData', { left: leftThrottle, right: rightThrottle });
    }else if (data.startsWith('STATE:')) {
        // Handle state data
        const state = data.trim(); // Example format: STATE:VALUE
        io.emit('stateData', state);
    } else {
        // Handle other types of data
        io.emit('fromArduino', data.trim());
    }
});



// Handle connections and commands from the client
io.on('connection', (socket) => {
    console.log('Client connected');

    // Send initial GPS and IMU data
    socket.emit('imuData', imuData);
    socket.emit('gpsData', gpsData);

    // Listen for throttle commands from the client
    socket.on('setThrottle', (value) => {
        console.log(`Setting throttle to: ${value}`);
        port.write(`${value}\n`);
    });

    // Mock updates for IMU and GPS data
    setInterval(() => {
        imuData = { x: Math.random() * 10, y: Math.random() * 10, z: Math.random() * 10 }; // Example data
        gpsData = { lat: 40.0 + Math.random(), lon: -105.0 + Math.random() }; // Example data
        socket.emit('imuData', imuData);
        socket.emit('gpsData', gpsData);

        // Send data to Arduino
        const gpsImuString = `GPS:${gpsData.lat},${gpsData.lon};IMU:${imuData.x},${imuData.y},${imuData.z}\n`;
        port.write(gpsImuString);
    }, 1000); // Update every second
});

// Start the server
const PORT = 3000;
server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
