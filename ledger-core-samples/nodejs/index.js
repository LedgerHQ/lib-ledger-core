const crypto = require("crypto");
const path = require("path");
const axios = require("axios");

const binding = require("bindings")("ledger-core");

const MAX_RANDOM = 2684869021;

const signTransaction = require("./signTransaction");
const {
  stringToBytesArray,
  bytesToHex,
  hexToBytes,
  bytesArrayToString
} = require("./helpers");

/**
 * NJSExecutionContext
 * -------------------
 */

const NJSExecutionContextImpl = {
  execute: runnable => {
    try {
      runnable.run();
    } catch (e) {
      console.log(e);
    }
  },
  delay: (runnable, ms) => setTimeout(() => runnable.run(), ms)
};

/**
 * ThreadDispatcher
 * ----------------
 */

const NJSThreadDispatcherImpl = {
  contexts: {}
};

/**
 * @param: name: string, context's name
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getThreadPoolExecutionContext = name =>
  NJSThreadDispatcherImpl.getSerialExecutionContext(name);

/**
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getMainExecutionContext = () =>
  NJSThreadDispatcherImpl.getSerialExecutionContext("main");

/**
 * @param: name: string, context's name
 * @return: NJSExecutionContext
 */
NJSThreadDispatcherImpl.getSerialExecutionContext = name => {
  let currentContext = NJSThreadDispatcherImpl.contexts[name];
  if (currentContext === undefined) {
    currentContext = new binding.NJSExecutionContext(NJSExecutionContextImpl);
    NJSThreadDispatcherImpl.contexts[name] = currentContext;
  }
  return currentContext;
};

/**
 * @return: NJSLock
 */
NJSThreadDispatcherImpl.newLock = () => {
  console.log("Not implemented"); // eslint-disable-line no-console
};

const NJSThreadDispatcher = new binding.NJSThreadDispatcher(
  NJSThreadDispatcherImpl
);

// ///////////////////////////////////////////
// ////////HttpClient Implementation//////////
// //////////////////////////////////////////

const METHODS = {
  0: "GET",
  1: "POST",
  2: "PUT",
  3: "DELETE"
};

const EVENT_CODE = {
  UNDEFINED: 0,
  NEW_OPERATION: 1,
  NEW_BLOCK: 2,
  SYNCHRONIZATION_STARTED: 3,
  SYNCHRONIZATION_FAILED: 4,
  SYNCHRONIZATION_SUCCEED: 5,
  SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT: 6
};

const NJSHttpClientImpl = {
  execute: async r => {
    const method = r.getMethod();
    const headersMap = r.getHeaders();
    let data = r.getBody();
    if (Array.isArray(data)) {
      const dataStr = bytesArrayToString(data);
      try {
        data = JSON.parse(dataStr);
      } catch (err) {}
    }
    const url = r.getUrl();
    const headers = {};
    headersMap.forEach((v, k) => {
      headers[k] = v;
    });
    let res;
    try {
      res = await axios({ method: METHODS[method], url, headers, data });
      const urlConnection = createHttpConnection(res);
      r.complete(urlConnection, null);
    } catch (err) {
      console.log(err);
      const urlConnection = createHttpConnection(res, "something went wrong");
      r.complete(urlConnection, { code: 0, message: "something went wrong" });
    }
  }
};

function createHttpConnection(res, err) {
  if (!res) {
    return null;
  }
  const headersMap = new Map();
  Object.keys(res.headers).forEach(key => {
    if (typeof res.headers[key] === "string") {
      headersMap.set(key, res.headers[key]);
    }
  });
  const NJSHttpUrlConnectionImpl = {
    getStatusCode: () => Number(res.status),
    getStatusText: () => res.statusText,
    getHeaders: () => headersMap,
    readBody: () => ({
      error: err ? { code: 0, message: "something went wrong" } : null,
      data: stringToBytesArray(JSON.stringify(res.data))
    })
  };
  return new binding.NJSHttpUrlConnection(NJSHttpUrlConnectionImpl);
}

/*
  @param: httpRequest: NJSHttprequest
*/

const NJSHttpClient = new binding.NJSHttpClient(NJSHttpClientImpl);

// ///////////////////////////////////////////////////
// ////////NJSWebSocketClient Implementation//////////
// //////////////////////////////////////////////////
const NJSWebSocketClientImpl = {};
/*
  @param: url: string
  @param: connection: NJSWebSocketConnection
*/
NJSWebSocketClientImpl.connect = (url, connection) => {
  connection.OnConnect();
};
/*
  @param: connection: NJSWebSocketConnection
  @param: data: string
*/
NJSWebSocketClientImpl.send = (connection, data) => {
  connection.OnMessage(data);
};
/*
  @param: connection: NJSWebSocketConnection
*/
NJSWebSocketClientImpl.disconnect = connection => {
  connection.OnClose();
};
const NJSWebSocketClient = new binding.NJSWebSocketClient(
  NJSWebSocketClientImpl
);
// ///////////////////////////////////////////////////
// ////////NJSPathResolver Implementation////////////
// //////////////////////////////////////////////////
const NJSPathResolverImpl = {};
/*
  Resolves the path for a SQLite database file.
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolveDatabasePath = pathToResolve => {
  let result = pathToResolve.replace(/\//g, "__");
  result = `./database_${result}`;
  const resolvedPath = path.resolve(__dirname, "tmp", result);
  return resolvedPath;
};
/*
  Resolves the path of a single log file..
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolveLogFilePath = pathToResolve => {
  let result = pathToResolve.replace(/\//g, "__");
  result = `./log_file_${result}`;
  const resolvedPath = path.resolve(__dirname, "tmp", result);
  return resolvedPath;
};
/*
   Resolves the path for a json file.
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolvePreferencesPath = pathToResolve => {
  let result = pathToResolve.replace(/\//g, "__");
  result = `./preferences_${result}`;
  const resolvedPath = path.resolve(__dirname, "tmp", result);
  return resolvedPath;
};
const NJSPathResolver = new binding.NJSPathResolver(NJSPathResolverImpl);
// ///////////////////////////////////////////
// ////////LogPrinter Implementation//////////
// //////////////////////////////////////////
const NJSLogPrinterImpl = {
  context: {}
};
/*
  @param: message: string
*/

const logger = (title, message) => {
  if (message) {
    // console.log(message);
  }
};

NJSLogPrinterImpl.printError = message => {
  logger("Error", message);
};
/*
  @param: message: string
*/
NJSLogPrinterImpl.printInfo = message => {
  logger("Info", message);
};
/*
  @param: message: string
*/
NJSLogPrinterImpl.printDebug = message => {
  logger("Debug", message);
};
/*
  @param: message: string
*/
NJSLogPrinterImpl.printWarning = message => {
  logger("Warning", message);
};
/*
  @param: message: string
*/
NJSLogPrinterImpl.printApdu = message => {
  logger("Apdu", message);
};
/*
  @param: message: string
*/
NJSLogPrinterImpl.printCriticalError = message => {
  logger("Critical Error", message);
};
/*
  @return: main NJSExecutionContext
*/
NJSLogPrinterImpl.getContext = () =>
  NJSThreadDispatcher.getMainExecutionContext();

const NJSLogPrinter = new binding.NJSLogPrinter(NJSLogPrinterImpl);

// //////////////////////////////////////////////////////
// ////////RandomNumberGenerator Implementation//////////
// /////////////////////////////////////////////////////
const NJSRandomNumberGeneratorImpl = {};
/*
  @param: size: integer
*/
NJSRandomNumberGeneratorImpl.getRandomBytes = size => crypto.randomBytes(size);
NJSRandomNumberGeneratorImpl.getRandomInt = () => Math.random() * MAX_RANDOM;
NJSRandomNumberGeneratorImpl.getRandomLong = () =>
  Math.random() * MAX_RANDOM * MAX_RANDOM;
NJSRandomNumberGeneratorImpl.getRandomLong = () => crypto.randomBytes(1);
const NJSRandomNumberGenerator = new binding.NJSRandomNumberGenerator(
  NJSRandomNumberGeneratorImpl
);
// //////////////////////////////////////////////////////
// /////////////Instanciate C++ objects/////////////////
// /////////////////////////////////////////////////////
const NJSDatabaseBackend = new binding.NJSDatabaseBackend();
const NJSDynamicObject = new binding.NJSDynamicObject();
const NJSNetworks = new binding.NJSNetworks();

// NJSThreadDispatcher.getMainExecutionContext();

/*
  Test for wallet pool instanciation
static newInstance(name: string, password: optional<string>, httpClient: HttpClient,
                      webSocketClient: WebSocketClient, pathResolver: PathResolver,
                      logPrinter: LogPrinter, dispatcher: ThreadDispatcher,
                      rng: RandomNumberGenerator, backend: DatabaseBackend (in C++),
                      configuration: DynamicObject): WalletPool;
*/
logger("NJSWalletPool Instanciation");
const NJSWalletPool = new binding.NJSWalletPool(
  "test_instance",
  "",
  NJSHttpClient,
  NJSWebSocketClient,
  NJSPathResolver,
  NJSLogPrinter,
  NJSThreadDispatcher,
  NJSRandomNumberGenerator,
  NJSDatabaseBackend,
  NJSDynamicObject
);

/*
  Get wallet pool instance name
*/
logger("Test wallet pool instance", NJSWalletPool.getName());
/*
  Get wallet count
*/
NJSWalletPool.getWalletCount().then(res => {
  logger("Wallet pool count before wallet creation", res);
});

/*
  Create a new wallet
*/

exports.EVENT_CODE = EVENT_CODE;

exports.getWallet = function getWallet(walletName) {
  return NJSWalletPool.getWallet(walletName);
};

exports.createWallet = async (name, currency) => {
  const NJSDynamicObjectWallet = new binding.NJSDynamicObject();
  const wallet = await NJSWalletPool.createWallet(
    name,
    currency,
    NJSDynamicObjectWallet
  );
  // const count = await NJSWalletPool.getWalletCount()
  return wallet;
};

exports.createAmount = (currency, amount) => {
  return new binding.NJSAmount(currency, amount).fromLong(currency, amount);
};

exports.getCurrency = currencyName => NJSWalletPool.getCurrency(currencyName);

exports.getNextAccountCreationInfo = wallet =>
  wallet.getNextAccountCreationInfo();

exports.createAccount = async (wallet, hwApp) => {
  const accountCreationInfos = await wallet.getNextAccountCreationInfo();
  await accountCreationInfos.derivations.reduce(
    (promise, derivation) =>
      promise.then(async () => {
        const {
          publicKey,
          chainCode,
          bitcoinAddress
        } = await hwApp.getWalletPublicKey(derivation);
        accountCreationInfos.publicKeys.push(hexToBytes(publicKey));
        accountCreationInfos.chainCodes.push(hexToBytes(chainCode));
      }),
    Promise.resolve()
  );
  return wallet.newAccountWithInfo(accountCreationInfos);
};

exports.createWalletUid = function createWalletUid(walletName) {
  // TODO: use poolname in the wallet uid, if multiple pools
  return crypto
    .createHash("sha256")
    .update(walletName)
    .digest("hex");
};

function createEventReceiver(cb) {
  return new binding.NJSEventReceiver({
    onEvent: event => cb(event)
  });
}
exports.createEventReceiver = createEventReceiver;

function subscribeToEventBus(eventBus, receiver) {
  eventBus.subscribe(NJSThreadDispatcherImpl.contexts.main, receiver);
}
exports.subscribeToEventBus = subscribeToEventBus;

exports.syncAccount = function syncAccount(account) {
  return new Promise((resolve, reject) => {
    const eventReceiver = createEventReceiver(e => {
      const code = e.getCode();
      if (
        code === EVENT_CODE.UNDEFINED ||
        code === EVENT_CODE.SYNCHRONIZATION_FAILED
      ) {
        return reject(new Error("Sync failed"));
      }
      if (
        code === EVENT_CODE.SYNCHRONIZATION_SUCCEED ||
        code === EVENT_CODE.SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT
      ) {
        resolve();
      }
    });
    const eventBus = account.synchronize();
    subscribeToEventBus(eventBus, eventReceiver);
  });
};

exports.signTransaction = signTransaction;
