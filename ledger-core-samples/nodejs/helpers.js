const binding = require("bindings")("ledgerapp_nodejs");

function stringToBytesArray(str = "") {
  const arr = [];
  for (let i = 0; i < str.length; i++) {
    arr.push(str.charCodeAt(i));
  }
  return arr;
}

exports.stringToBytesArray = stringToBytesArray;

function hexToBytes(str) {
  for (var bytes = [], c = 0; c < str.length; c += 2) {
    bytes.push(parseInt(str.substr(c, 2), 16));
  }
  return bytes;
}

exports.hexToBytes = hexToBytes;

function createBitcoinLikeHelper() {
  return new binding.NJSBitcoinLikeHelper();
}
exports.createBitcoinLikeHelper = createBitcoinLikeHelper;
