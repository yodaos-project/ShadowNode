
var fs = require("fs")
var path = require("path")
var exec = require('child_process').exec

var root = path.join(__dirname, process.argv[2])
console.log(`scanning dir ${root}`)

var items = fs.readdirSync(root)
var output = `'use strict';
var assert = require('assert')
var tests = [`
items.forEach((item) => {
  if (item.startsWith('.')) {
    return
  }
  var itemPath = path.resolve(root, item)
  var itemStat = fs.statSync(itemPath)
  if (!itemStat.isFile() || path.extname(item) !== '.js') {
    return
  }
  console.log(`handle ${itemPath}`)
  var content = fs.readFileSync(itemPath).toString()
  if (/Symbol|compareArray|verifyNotEnumerable|verifyNotWritable|Test262Error/.test(content)) {
    return
  }
  content = content.replace(/assert.sameValue\((.*), (.*), "(.*)?\);/g, 'assert($1 === $2);')
  content = content.replace(/assert.sameValue\((.*), (.*)\);/g, 'assert($1 === $2);')
  content = content.replace(/assert.sameValue\(\n +(.*),\n +(true|false),\n +(.*)\n\);/g, 'assert($1 === $2);')
  if (/assert\./.test(content)) {
    return
  }
  output += `
  {
    name: '${item}',
    exec: function () {
      ${content}
    }
  },`
})
output += `
];

for (var i = 0; i < tests.length; ++i) {
  console.log(\`testing \${tests[i].name}\`)
  tests[i].exec()
}
`
var outputPath = path.join(__dirname, 'output.js')
fs.writeFileSync(outputPath, output)

exec('eslint -c .eslintrc.js output.js --fix', (err, stdout, stderr) => {
  console.log(stdout)
  console.log(stderr)
})
