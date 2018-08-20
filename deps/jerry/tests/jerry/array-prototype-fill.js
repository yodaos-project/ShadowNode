console.log([1, 2, 3].fill(4));               // [4, 4, 4]
console.log([1, 2, 3].fill(4, 1));            // [1, 4, 4]
console.log([1, 2, 3].fill(4, 1, 2));         // [1, 4, 3]
console.log([1, 2, 3].fill(4, 1, 1));         // [1, 2, 3]
console.log([1, 2, 3].fill(4, 3, 3));         // [1, 2, 3]
console.log([1, 2, 3].fill(4, -3, -2));       // [4, 2, 3]
console.log([1, 2, 3].fill(4, NaN, NaN));     // [1, 2, 3]
console.log([1, 2, 3].fill(4, 3, 5));         // [1, 2, 3]
console.log(Array(3).fill(4));                // [4, 4, 4]
console.log(JSON.stringify([].fill.call({ length: 3 }, 4)));  // {0: 4, 1: 4, 2: 4, length: 3}

// Objects by reference.
var arr = Array(3)
arr = arr.fill({}) // [{}, {}, {}];
arr[0].hi = "hi"; // [{ hi: "hi" }, { hi: "hi" }, { hi: "hi" }]
console.log(JSON.stringify(arr))