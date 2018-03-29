var len = 30000;
var buf = new Buffer(len);

for (var i = 0; i < len; i++) {
  buf.write('s', i);
}

setInterval(() => {
  var now = Date.now();
  buf.toString();
  console.log(Date.now() - now);
}, 500);

