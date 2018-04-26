const crypto = require("crypto");
const path = require("path");
const axios = require("axios");

const MAX_RANDOM = 2684869021;

const binding = require("bindings")("ledgerapp_nodejs");

const logger = (title, message) => {
  console.log(`======${title}======`);
  if (message) {
    console.log(message);
  }
  console.log("===================");
};

/////////////////////////////////////////////
//////////ExecutionContext Implementation////
////////////////////////////////////////////
const NJSExecutionContextImpl = {};
/*
  @param: runnable: NJSRunnable
*/
NJSExecutionContextImpl.execute = runnable => {
  try {
    runnable.run();
  } catch (e) {
    console.log(e);
  }
};
/*
  @param: runnable: NJSRunnable
  @param: millis: delay (integer)
*/
NJSExecutionContextImpl.delay = (runnable, millis) => {
  setTimeout(() => runnable.run(), millis);
};

const NJSExecutionContext = new binding.NJSExecutionContext(
  NJSExecutionContextImpl
);

/////////////////////////////////////////////
//////////ThreadDispatcher Implementation////
////////////////////////////////////////////
const NJSThreadDispatcherImpl = {
  contexts: {}
};
/*
  @param: name: string, context's name
  @return: NJSExecutionContext
*/
NJSThreadDispatcherImpl.getMainExecutionContext = () =>
  NJSThreadDispatcherImpl.getSerialExecutionContext("main");
/*
  @param: name: string, context's name
  @return: NJSExecutionContext
*/
NJSThreadDispatcherImpl.getThreadPoolExecutionContext = name =>
  NJSThreadDispatcherImpl.getSerialExecutionContext(name);
/*
  @param: name: string, context's name
  @return: NJSExecutionContext
*/
NJSThreadDispatcherImpl.getSerialExecutionContext = name => {
  let currentContext = NJSThreadDispatcherImpl.contexts[name];
  if (currentContext === undefined) {
    currentContext = new binding.NJSExecutionContext(NJSExecutionContextImpl);
    NJSThreadDispatcherImpl.contexts[name] = currentContext;
  }
  return currentContext;
};
/*
  @return: NJSLock
*/
NJSThreadDispatcherImpl.newLock = () => {
  console.log("Not implemented"); // eslint-disable-line no-console
};

const NJSThreadDispatcher = new binding.NJSThreadDispatcher(
  NJSThreadDispatcherImpl
);

/////////////////////////////////////////////
//////////HttpClient Implementation//////////
////////////////////////////////////////////

const METHODS = {
  0: "GET",
  1: "POST",
  2: "PUT",
  3: "DELETE"
};

const NJSHttpClientImpl = {
  execute: async r => {
    const method = r.getMethod();
    const headers = r.getHeaders();
    const data = r.getBody();
    const url = r.getUrl();
    console.log(headers);
    console.log(url);
    console.log(data);
    try {
      const res = await axios({ method: METHODS[method], url, headers, data });
      r.complete(
        {
          getStatusCode: () => res.status,
          getStatusText: () => res.statusText,
          getHeaders: () => res.headers,
          readBody: () => ({ error: null, data: res.body })
        },
        ""
      );
    } catch (err) {
      r.complete(
        {
          getStatusCode: () => res.status,
          getStatusText: () => res.statusText,
          getHeaders: () => res.headers,
          readBody: () => ({ error: "something went wrong" })
        },
        ""
      );
    }
  }
};

/*
  @param: httpRequest: NJSHttprequest
*/

const NJSHttpClient = new binding.NJSHttpClient(NJSHttpClientImpl);

/////////////////////////////////////////////////////
//////////NJSWebSocketClient Implementation//////////
////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////
//////////NJSPathResolver Implementation////////////
////////////////////////////////////////////////////
const NJSPathResolverImpl = {};
/*
  Resolves the path for a SQLite database file.
  @param: path: string
  @return: resolved path
*/
NJSPathResolverImpl.resolveDatabasePath = pathToResolve => {
  let result = pathToResolve.replace(/\//g, "__");
  result = "./database_" + result;
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
  result = "./log_file_" + result;
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
  result = "./preferences_" + result;
  const resolvedPath = path.resolve(__dirname, "tmp", result);
  return resolvedPath;
};
const NJSPathResolver = new binding.NJSPathResolver(NJSPathResolverImpl);
/////////////////////////////////////////////
//////////LogPrinter Implementation//////////
////////////////////////////////////////////
const NJSLogPrinterImpl = {
  context: {}
};
/*
  @param: message: string
*/
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
NJSLogPrinterImpl.getContext = () => {
  return NJSThreadDispatcher.getMainExecutionContext();
};

const NJSLogPrinter = new binding.NJSLogPrinter(NJSLogPrinterImpl);

////////////////////////////////////////////////////////
//////////RandomNumberGenerator Implementation//////////
///////////////////////////////////////////////////////
const NJSRandomNumberGeneratorImpl = {};
/*
  @param: size: integer
*/
NJSRandomNumberGeneratorImpl.getRandomBytes = size => {
  return crypto.randomBytes(size);
};
NJSRandomNumberGeneratorImpl.getRandomInt = () => {
  return Math.random() * MAX_RANDOM;
};
NJSRandomNumberGeneratorImpl.getRandomLong = () => {
  return Math.random() * MAX_RANDOM * MAX_RANDOM;
};
NJSRandomNumberGeneratorImpl.getRandomLong = () => {
  return crypto.randomBytes(1);
};
const NJSRandomNumberGenerator = new binding.NJSRandomNumberGenerator(
  NJSRandomNumberGeneratorImpl
);
////////////////////////////////////////////////////////
///////////////Instanciate C++ objects/////////////////
///////////////////////////////////////////////////////
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

// exports.getBitcoinLikeNetworkParameters = () => {
//   let bitcoinLikeNetworkParameters;
//   try {
//     bitcoinLikeNetworkParameters = NJSNetworks.bitcoin();
//   } catch (e) {
//     throw e;
//   }
//   return bitcoinLikeNetworkParameters;
// };

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
  return new binding.NJSAmount(currency, amount);
};

exports.getCurrency = currencyName => {
  return NJSWalletPool.getCurrency(currencyName);
};

exports.getNextAccountCreationInfo = wallet => {
  return wallet.getNextAccountCreationInfo();
};

exports.createAccount = (wallet, accountCreationInfos) => {
  return wallet.newAccountWithInfo(accountCreationInfos);
};

exports.createWalletUid = function createWalletUid(walletName) {
  // TODO: use poolname in the wallet uid, if multiple pools
  return crypto
    .createHash("sha256")
    .update(walletName)
    .digest("hex");
};

exports.subscribeToEventBus = function subscribeToEventBus(eventBus, cb) {
  const receiver = new binding.NJSEventReceiver({
    onEvent: event => cb(event)
  });
  eventBus.subscribe(NJSThreadDispatcherImpl.contexts.main, receiver);
};
