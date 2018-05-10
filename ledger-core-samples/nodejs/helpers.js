const binding = require('bindings')('ledger-core')

const segwitMode = true;
exports.segwitMode = segwitMode;

function bytesArrayToString(bytesArray = []) {
    return bytesArray.map(b => String.fromCharCode(b)).join('')
}
exports.bytesArrayToString = bytesArrayToString

function stringToBytesArray(str = '') {
    const arr = []
    for (let i = 0; i < str.length; i++) {
        arr.push(str.charCodeAt(i))
    }
    return arr
}

exports.stringToBytesArray = stringToBytesArray

function hexToBytes(str) {
    const bytes = []
    for (let c = 0; c < str.length; c += 2) {
        bytes.push(parseInt(str.substr(c, 2), 16))
    }
    return bytes
}
exports.hexToBytes = hexToBytes

function bytesToHex(bytes = []) {
    return Array.from(bytes, byte => `0${(byte & 0xff).toString(16)}`.slice(-2)).join('')
}
exports.bytesToHex = bytesToHex

function createBitcoinLikeHelper() {
    return new binding.NJSBitcoinLikeHelper()
}
exports.createBitcoinLikeHelper = createBitcoinLikeHelper
