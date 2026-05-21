const express = require('express');
const http    = require('http');
const path    = require('path');
const WebSocket = require('ws');

const app    = express();
const server = http.createServer(app);
const wss    = new WebSocket.Server({ server });

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'dashboard.html'));
});

wss.on('connection', (ws) => {
    ws.on('message', (message) => {
        wss.clients.forEach(client => {
            if (client !== ws && client.readyState === WebSocket.OPEN)
                client.send(message.toString());
        });
    });
});

server.listen(3000, () => console.log('http://localhost:3000'));
