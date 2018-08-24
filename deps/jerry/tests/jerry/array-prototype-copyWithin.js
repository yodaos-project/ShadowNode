console.log([1, 2, 3, 4, 5].copyWithin(-2))
// [1, 2, 3, 1, 2]

console.log([1, 2, 3, 4, 5].copyWithin(0, 3))
// [4, 5, 3, 4, 5]

console.log([1, 2, 3, 4, 5].copyWithin(0, 3, 4))
// [4, 2, 3, 4, 5]

console.log([1, 2, 3, 4, 5].copyWithin(-2, -3, -1))
// [1, 2, 3, 3, 4]

console.log(JSON.stringify([].copyWithin.call({length: 5, 3: 1}, 0, 3)))
// {0: 1, 3: 1, length: 5}