(function() {
  console.log([].includes.call(arguments, 'a')); // true
  console.log([].includes.call(arguments, 'd')); // false
})('a','b','c');

// 数组长度是3
// fromIndex 是 -100
// computed index 是 3 + (-100) = -97

var arr = ['a', 'b', 'c'];

console.log(arr.includes('a', -100)); // true
console.log(arr.includes('b', -100)); // true
console.log(arr.includes('c', -100)); // true