const express = require('express');
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const http = require('http');
const { Server } = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = new Server(server);

const port = new SerialPort('COM3', { baudRate: 9600 }); // Update COM port as needed
const parser = port.pipe(new Readline({ delimiter: '\n' }));

app.use(express.static('public')); // Serve frontend from "public" directory

parser.on('data', (data) => {
    console.log(`Data received: ${data}`);
    io.emit('rotorSpeed', data.trim()); // Send data to the webpage
});

io.on('connection', (socket) => {
    console.log('Client connected');
});

server.listen(3000, () => {
    console.log('Server running at http://localhost:3000');
});
