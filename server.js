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

    // Listen for throttle commands from the client
    socket.on('boatData', (value) => {
        // console.log(`Sending data: Latitude=${value.latitude}, Longitude=${value.longitude}, Direction=${value.direction}`);
        const dataString = `${value.latitude},${value.longitude},${value.direction}\n`;
        // console.log(dataString);
        port.write(dataString);
        // port.write(`${value}\n`);
    });

});

// Start the server
const PORT = 3000;
server.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
