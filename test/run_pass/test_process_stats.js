var bufs = []
var lastUsage = process.memoryUsage()
setInterval(() => {
  var buf = Buffer.allocUnsafe(1024)
  buf.fill('rokid')
  bufs.push(buf)
  var usage = process.memoryUsage()
  console.log('rss:' + usage.rss +
    ' total:' + usage.heapTotal + 
    ' used:' + usage.heapUsed + 
    ' used delta:' + (usage.heapUsed - lastUsage.heapUsed) +
    ' bufLen:' + bufs.length
  )
  lastUsage = usage
}, 1000)