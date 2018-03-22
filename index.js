const binding = require('bindings')('ledgerapp_nodejs'); // eslint-disable-line global-require
/*
    Test NJSSecp256k1 Implementation
 */
const NJSSecp256k1Impl = new binding.NJSSecp256k1();
const pubKeys = NJSSecp256k1Impl.computePubKey([123],true);
console.log(pubKeys);
