var assert = require('assert');
var crypto = require('crypto');

function test_hash(algorithm, input, output) {
  var hash = crypto.createHash(algorithm);
  assert.equal(hash.update(input).digest('hex'), output);
}

test_hash('md5', 'foobar', '3858f62230ac3c915f300c664312c63f');
test_hash('md5', '中文', 'a7bac2239fcdcb3a067903d8077c4a07');
test_hash('sha256', 'foobar',
  'c3ab8ff13720e8ad9047dd39466b3c8974e592c2fa383d4a3960714caef0c4f2');
test_hash('sha256', '中文',
  '72726d8818f693066ceb69afa364218b692e62ea92b385782363780f47529c21');
