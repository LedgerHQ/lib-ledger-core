/* eslint-disable no-console */
const { getLibCoreVersion } = require('../index.js')

const version = getLibCoreVersion();
console.log(`${version}`)
