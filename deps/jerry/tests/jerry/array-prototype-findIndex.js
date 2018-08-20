function isPrime(element, index, array) {
  var start = 2;
  while (start <= Math.sqrt(element)) {
    if (element % start++ < 1) {
      return false;
    }
  }
  return element > 1;
}

var t = Date.now()
console.log([4, 6, 8, 12].findIndex(isPrime)); // -1
console.log([4, 5, 8, 12].findIndex(isPrime)); // 1

function test() {
  console.log(Array.prototype.findIndex.call(arguments, isPrime))
}

test(4, 6, 7, 9)

console.log(Date.now() - t)