const crypto = require('crypto')
const path = require('path')
const axios = require('axios')

const binding = require('bindings')('ledger-core')

const MAX_RANDOM = 2684869021

const signTransaction = require('./signTransaction')
const { stringToBytesArray, bytesToHex, hexToBytes, bytesArrayToString, segwitMode } = require('./helpers')

/**
 * NJSExecutionContext
 * -------------------
 */

const NJSExecutionContextImpl = {
    execute: runnable => {
    try {
        runnable.run()
} catch (e) {
    console.log(e)
}
},
delay: (runnable, ms) => setTimeout(() => runnable.run(), ms),
}

/**
 * ThreadDispatcher
 * ----------------
 */

const NJSThreadDispatcherImpl = {
    contexts: {},
}

/**
 * @param: name: string, context's name
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getThreadPoolExecutionContext = name =>
NJSThreadDispatcherImpl.getSerialExecutionContext(name)

/**
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getMainExecutionContext = () =>
NJSThreadDispatcherImpl.getSerialExecutionContext('main')

/**
 * @param: name: string, context's name
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getSerialExecutionContext = name => {
    let currentContext = NJSThreadDispatcherImpl.contexts[name]
    if (currentContext === undefined) {
        currentContext = new binding.NJSExecutionContext(NJSExecutionContextImpl)
        NJSThreadDispatcherImpl.contexts[name] = currentContext
    }
    return currentContext
}

/**
 * @return: NJSLock
 */
NJSThreadDispatcherImpl.newLock = () => {
    console.log('Not implemented') // eslint-disable-line no-console
}

const NJSThreadDispatcher = new binding.NJSThreadDispatcher(NJSThreadDispatcherImpl)

// ///////////////////////////////////////////
// ////////HttpClient Implementation//////////
// //////////////////////////////////////////

const METHODS = {
    0: 'GET',
    1: 'POST',
    2: 'PUT',
    3: 'DELETE',
}

const EVENT_CODE = {
    UNDEFINED: 0,
    NEW_OPERATION: 1,
    NEW_BLOCK: 2,
    SYNCHRONIZATION_STARTED: 3,
    SYNCHRONIZATION_FAILED: 4,
    SYNCHRONIZATION_SUCCEED: 5,
    SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT: 6,
}

const NJSHttpClientImpl = {
    execute: async r => {
    const method = r.getMethod()
    const headersMap = r.getHeaders()
    let data = r.getBody()
    if (Array.isArray(data)) {
    const dataStr = bytesArrayToString(data)
    try {
        data = JSON.parse(dataStr)
    } catch (err) {}
}
const url = r.getUrl()
const headers = {}
headersMap.forEach((v, k) => {
    headers[k] = v
})
let res
try {
    res = await axios({ method: METHODS[method], url, headers, data })
    const urlConnection = createHttpConnection(res)
    r.complete(urlConnection, null)
} catch (err) {
    let strErr = ''
    // handle axios err
    if (err.response && err.response.data && err.response.data.error) {
        strErr = err.response.data.error
    } else {
        strErr = 'something went wrong'
    }
    const urlConnection = createHttpConnection(res, strErr)
    r.complete(urlConnection, { code: 0, message: strErr })
}
},
}

function createHttpConnection(res, err) {
    if (!res) {
        return null
    }
    const headersMap = new Map()
    Object.keys(res.headers).forEach(key => {
        if (typeof res.headers[key] === 'string') {
        headersMap.set(key, res.headers[key])
    }
})
    const NJSHttpUrlConnectionImpl = {
            getStatusCode: () => Number(res.status),
        getStatusText: () => res.statusText,
        getHeaders: () => headersMap,
        readBody: () => ({
        error: err ? { code: 0, message: 'something went wrong' } : null,
        data: stringToBytesArray(JSON.stringify(res.data)),
    }),
}
    return new binding.NJSHttpUrlConnection(NJSHttpUrlConnectionImpl)
}

/*
  @param: httpRequest: NJSHttprequest
*/

const NJSHttpClient = new binding.NJSHttpClient(NJSHttpClientImpl)

// ///////////////////////////////////////////////////
// ////////NJSWebSocketClient Implementation//////////
// //////////////////////////////////////////////////
const NJSWebSocketClientImpl = {}
/*
  @param: url: string
  @param: connection: NJSWebSocketConnection
*/
NJSWebSocketClientImpl.connect = (url, connection) => {
    connection.OnConnect()
}
/*
  @param: connection: NJSWebSocketConnection
  @param: data: string
*/
NJSWebSocketClientImpl.send = (connection, data) => {
    connection.OnMessage(data)
}
/*
  @param: connection: NJSWebSocketConnection
*/
NJSWebSocketClientImpl.disconnect = connection => {
    connection.OnClose()
}
const NJSWebSocketClient = new binding.NJSWebSocketClient(NJSWebSocketClientImpl)
// ///////////////////////////////////////////////////
// ////////NJSPathResolver Implementation////////////
// //////////////////////////////////////////////////
const NJSPathResolverImpl = {}
/*
  Resolves the path for a SQLite database file.
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolveDatabasePath = pathToResolve => {
    let result = pathToResolve.replace(/\//g, '__')
    result = `./database_${result}`
    const resolvedPath = path.resolve(__dirname, 'tmp', result)
    return resolvedPath
}
/*
  Resolves the path of a single log file..
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolveLogFilePath = pathToResolve => {
    let result = pathToResolve.replace(/\//g, '__')
    result = `./log_file_${result}`
    const resolvedPath = path.resolve(__dirname, 'tmp', result)
    return resolvedPath
}

/**
 * Resolves the path for a json file
 *
 * @param: path: string
 * @return: resolved path
 */
NJSPathResolverImpl.resolvePreferencesPath = pathToResolve => {
    let result = pathToResolve.replace(/\//g, '__')
    result = `./preferences_${result}`
    const resolvedPath = path.resolve(__dirname, 'tmp', result)
    return resolvedPath
}
const NJSPathResolver = new binding.NJSPathResolver(NJSPathResolverImpl)

//                         -------------------------
//                         LogPrinter implementation
//                         -------------------------

const NJSLogPrinterImpl = {
    context: {},
}

/**
 * @param: message: string
 */
const logger = (title, message) => {
    if (message) {
        // console.log(message);
    }
}

NJSLogPrinterImpl.printError = message => logger('Error', message)
NJSLogPrinterImpl.printInfo = message => logger('Info', message)
NJSLogPrinterImpl.printDebug = message => logger('Debug', message)
NJSLogPrinterImpl.printWarning = message => logger('Warning', message)
NJSLogPrinterImpl.printApdu = message => logger('Apdu', message)
NJSLogPrinterImpl.printCriticalError = message => logger('Critical Error', message)

/**
 * @return: main NJSExecutionContext
 */
NJSLogPrinterImpl.getContext = () => new binding.NJSExecutionContext(NJSExecutionContextImpl)

const NJSLogPrinter = new binding.NJSLogPrinter(NJSLogPrinterImpl)

//                    ------------------------------------
//                    RandomNumberGenerator implementation
//                    ------------------------------------

const NJSRandomNumberGeneratorImpl = {}


NJSRandomNumberGeneratorImpl.getRandomBytes = size => crypto.randomBytes(size)
NJSRandomNumberGeneratorImpl.getRandomInt = () => Math.random() * MAX_RANDOM
NJSRandomNumberGeneratorImpl.getRandomLong = () => Math.random() * MAX_RANDOM * MAX_RANDOM
NJSRandomNumberGeneratorImpl.getRandomLong = () => crypto.randomBytes(1)

const NJSRandomNumberGenerator = new binding.NJSRandomNumberGenerator(NJSRandomNumberGeneratorImpl)

//                          -----------------------
//                          Instanciate C++ objects
//                          -----------------------

const NJSDatabaseBackend = new binding.NJSDatabaseBackend()
const NJSDynamicObject = new binding.NJSDynamicObject()
const NJSNetworks = new binding.NJSNetworks()

const NJSWalletPool = new binding.NJSWalletPool(
    'test_instance',
    '',
    NJSHttpClient,
    NJSWebSocketClient,
    NJSPathResolver,
    NJSLogPrinter,
    NJSThreadDispatcher,
    NJSRandomNumberGenerator,
    NJSDatabaseBackend,
    NJSDynamicObject,
)

exports.EVENT_CODE = EVENT_CODE

exports.getWallet = function getWallet(walletName) {
    return NJSWalletPool.getWallet(walletName)
}

exports.createWallet = async (name, currency) => {
    const configuration = new binding.NJSDynamicObject();
    //configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);
    const config = new binding.NJSConfiguration();
    if(segwitMode) {
        configuration.putString("KEYCHAIN_ENGINE", "BIP49_P2SH");
        configuration.putString("KEYCHAIN_DERIVATION_SCHEME", "49'/<coin_type>'/<account>'/<node>/<address>");
    }
    const wallet = await NJSWalletPool.createWallet(name, currency, configuration)
    // const count = await NJSWalletPool.getWalletCount()
    return wallet
}

exports.createAmount = (currency, amount) => {
  return new binding.NJSAmount(currency, amount).fromLong(currency, amount);
};

exports.getCurrency = currencyName => NJSWalletPool.getCurrency(currencyName)

exports.getNextAccountCreationInfo = wallet => wallet.getNextAccountCreationInfo()

exports.createAccount = async (wallet, hwApp) => {
    console.log(" >> Get next account infos");
    const accountCreationInfos = await wallet.getNextAccountCreationInfo()
    console.log(" >> Start derivations");
    await accountCreationInfos.derivations.reduce(
        (promise, derivation) =>
    promise.then(async () => {
        const verify = false; //default
    const { publicKey, chainCode, bitcoinAddress } = await hwApp.getWalletPublicKey(derivation, verify, segwitMode)
    console.log("*************");
    console.log(bitcoinAddress);
    console.log("*************");
    accountCreationInfos.publicKeys.push(hexToBytes(publicKey))
    accountCreationInfos.chainCodes.push(hexToBytes(chainCode))
}),
    Promise.resolve(),
)

    console.log(" >> End derivations");
    const account = wallet.newAccountWithInfo(accountCreationInfos);
    return account;
}

exports.createWalletUid = function createWalletUid(walletName) {
    // TODO: use poolname in the wallet uid, if multiple pools
    return crypto
        .createHash('sha256')
        .update(walletName)
        .digest('hex')
}

function createEventReceiver(cb) {
    return new binding.NJSEventReceiver({
        onEvent: event => cb(event),
})
}
exports.createEventReceiver = createEventReceiver

function subscribeToEventBus(eventBus, receiver) {
    eventBus.subscribe(NJSThreadDispatcherImpl.contexts.main, receiver)
}
exports.subscribeToEventBus = subscribeToEventBus

exports.syncAccount = function syncAccount(account) {
    return new Promise((resolve, reject) => {
        const eventReceiver = createEventReceiver(e => {
            const code = e.getCode()
            if (code === EVENT_CODE.UNDEFINED || code === EVENT_CODE.SYNCHRONIZATION_FAILED) {
        return reject(new Error('Sync failed'))
    }
    if (
        code === EVENT_CODE.SYNCHRONIZATION_SUCCEED ||
        code === EVENT_CODE.SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT
    ) {
        resolve()
    }
})
    const eventBus = account.synchronize()
    subscribeToEventBus(eventBus, eventReceiver)
})
}

exports.signTransaction = signTransaction
