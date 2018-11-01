
const fs = require('fs');

const args = process.argv.splice(2);
const infile = args[0];
const outfile = args[1];

const inbuf = fs.readFileSync(infile, 'utf-8');

let lines = inbuf.split('\n');
const output = lines.map((line) => {
    let elements = line.split(',');
    const time = elements[0];
    elements.shift();
    const backtrace = elements.reverse().join(';');
    return backtrace + ' ' + time;
}).join('\n');

fs.writeFileSync(outfile, output);