var mqtt = require('mqtt')
var fs = require('fs')
var dataPath = __dirname + '/tmp.txt'
var data = fs.readFileSync(dataPath).slice(0,1024*36)

var client = mqtt.connect('mqtt://127.0.0.1:1883', {
  username: 'ae5d675c90dac5e39da6b8e17edc04ee',
  password: '709359e171a9d1994ece85676eac1644',
  clientId: 'ae5d675c90dac5e39da6b8e17edc04ee',
})

client.on('connect', function () {
  // client.subscribe('presence')
  client.subscribe('test')
  // setInterval(() => {
  //   client.publish('test', data)
  // }, 1000)
})

client.on('error', function (err) {
  console.log(err, '!!')
})

client.on('message', function (topic, message) {
  console.log(message.byteLength, '~~~~~~')
})
// var obj = {}
// for (var i = 0; i < 10000; ++i) {
//   obj[ 'k' + Math.random()] = Math.random()
// }
// fs.writeFileSync(dataPath, (JSON.stringify(obj)))