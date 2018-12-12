'use strict';
var path = require('path');
var mosca = require('mosca');
var keyPath = path.resolve(__dirname, './test-key.pem')
var certPath = path.resolve(__dirname, './test-cert.pem')

var server = new mosca.Server({
  interfaces: [
    { type: 'mqtt', port: 1883 },
    { type: 'mqtts', port: 8883 }
  ],
  credentials: {
    keyPath: keyPath,
    certPath: certPath,
  },
});

server.on('clientConnected', function(client) {
  console.log('client connected', client.id);
});

// fired when a message is received
server.on('published', function(packet, client) {
  console.log('Published', packet.payload);
});

server.on('ready', function() {
  console.log('Mosca server is up and running');
});
