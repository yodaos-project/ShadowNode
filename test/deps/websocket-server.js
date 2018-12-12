'use strict';
var WebSocketServer = require('websocket').server;
var http = require('http');
var fs = require('fs');
var https = require('https');
var path = require('path');
var keyPath = path.resolve(__dirname, './test-key.pem')
var certPath = path.resolve(__dirname, './test-cert.pem')

var httpServer = http.createServer(function(request, response) {
  console.log((new Date()) + ' Received request for ' + request.url);
  response.writeHead(404);
  response.end();
});
httpServer.listen(8080, function() {
  console.log((new Date()) + ' Server is listening on port 8080');
});

var httpsServer = https.createServer({
  key: fs.readFileSync(keyPath),
  cert: fs.readFileSync(certPath)
}, function (request, response) {
  console.log((new Date()) + ' Received request for ' + request.url);
  response.writeHead(404);
  response.end();
})
httpsServer.listen(8088, function() {
  console.log((new Date()) + ' Server is listening on port 8088');
});

var wsServer = new WebSocketServer({
  httpServer: [httpServer, httpsServer],
  autoAcceptConnections: false
});

wsServer.on('request', function (request) {
  var connection = request.accept('echo', request.origin);
  console.log((new Date()) + ' Connection accepted.');
  connection.on('message', function (message) {
    if (message.type === 'utf8') {
      console.log('UTF8 Message of ' + message.utf8Data.length + ' bytes');
      connection.sendUTF(message.utf8Data);
    }
    else if (message.type === 'binary') {
      console.log('Binary Message of ' + message.binaryData.length + ' bytes');
      connection.sendBytes(message.binaryData);
    }
  });
  connection.on('close', function (reasonCode, description) {
    console.log(' Peer ' + connection.remoteAddress + ' disconnected.');
  });
});
