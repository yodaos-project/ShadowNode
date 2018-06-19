'use strict';

const list = data.reduce(function(result, item) {
  const name = item.name;
  if (!result[name]) {
    result[name] = {
      name,
      time: item.cost,
      count: 1,
    };
  } else {
    result[name].time += item.cost;
    result[name].count += 1;
  }
  return result;
}, {});

const chartData = [];
for (let name in list) {
  const item = list[name];
  chartData.push({
    name,
    avg: item.time / item.count,
    total: item.time,
    count: item.count,
  });
}

console.log(chartData);
var chart = new G2.Chart({
  container: 'mountNode',
  // forceFit: true,
  height: window.innerHeight - 200,
  width: window.innerWidth - 200,
});
chart.source(chartData);
chart.interval().position('name*total').color('name', G2.Global.colors_pie_16).style({
  lineWidth: 1,
  stroke: '#fff'
});
chart.render();