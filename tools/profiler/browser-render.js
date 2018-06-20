
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
  const value = {
    name,
    avg: item.time / item.count,
    total: item.time,
    count: item.count,
  };
  value.total.__foobar__ = 'foobar';

  chartData.push(value);
}

var chart = new G2.Chart({
  container: 'mountNode',
  forceFit: true,
});
chart.source(chartData);
chart.coord().transpose().scale(1, -1);
chart.legend('name', {
  itemWidth: 300,
});
chart.axis('name', false);
chart.tooltip(true, {
  itemTpl: [
    '<ul class="g2-tooltip-list-item">',
      '<li>total: {value}</li>',
    '</ul>',
  ].join('\n'),
});
chart.intervalStack().position('name*total')
  .color('name', G2.Global.colors_pie_16)
  .style({
    lineWidth: 1,
    stroke: '#fff'
  });
chart.render();
