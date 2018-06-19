#!/usr/bin/env node

'use strict';

const fs = require('fs');
const argv = process.argv.slice(2);

const contents = fs.readFileSync(process.cwd() + '/' + argv[0], 'utf8');
const data = '[' + 
  contents.split('\n').filter((s) => !!s).map((s) => `{${s}}`).join(',') +
']';

var html_template = [
'<html>',
  '<head>',
  '<title>ShadowNode Profiling Graph</title>',
  '<meta charset="UTF-8">',
  '<meta name="viewport" content="width=device-width,height=device-height">',
  '<style>::-webkit-scrollbar{display:none;}html,body{overflow:hidden;height:100%;}</style>',
  '<script src="https://gw.alipayobjects.com/os/antv/assets/g2/3.1.0/g2.min.js"></script>',
  '<script src="https://gw.alipayobjects.com/os/antv/assets/data-set/0.8.7/data-set.min.js"></script>',
  '</head>',
  '<body>',
    '<h1 style="text-align:center">ShadowNode Profiling Graph</h1>',
    '<div id="mountNode" style="margin-left:50px;margin-top:50px;"></div>',
    '<script>',
      `var data = ${data};`,
    '</script>',
    `<script src="${__dirname}/browser-render.js"></script>`,
  '</body>',
'</html>'].join('\n');

console.log(html_template);
