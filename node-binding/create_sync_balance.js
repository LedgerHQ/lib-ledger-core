const commands = require('./messages/commands_pb.js')
const btc_messages = require('./messages/bitcoin/commands_pb.js')
const core_config = require('./messages/core_configuration_pb')

createGetVersionRequest = function() {
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.GET_VERSION);
    return req;
}

createBitcoinCreateAccountRequest = function() {
    var createAccReq = new btc_messages.CreateAccountRequest();
    var accConfig = new btc_messages.AccountConfiguration();
    var accSettings = new btc_messages.AccountSettings();
    
    accSettings.setName("my account1");
    accConfig.setKeychainEngine(btc_messages.KeychainEngine.BIP49_P2SH);
    accConfig.setSettings(accSettings);
    createAccReq.setXpub("xpub6CQRBg1k8KN2yZfWUywHt9dtDSEFfHhwhBrEjzuHj5YBV2p81NEviAhEhGzYpC5AzwuL6prM2wc1oyMQ8hmsCKqrWwHrjcboQvkBctC1JTq");
    createAccReq.setIndex(0);
    createAccReq.setConfig(accConfig);
    var btcReq = new btc_messages.Request();
    btcReq.setType(btc_messages.RequestType.CREATE_ACCOUNT);
    btcReq.setSubmessage(createAccReq.serializeBinary());
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createSyncAccountRequest = function(uid) {
    var syncReq = new btc_messages.SyncAccountRequest();
    syncReq.setAccUid(uid);
    var btcReq = new btc_messages.Request();
    btcReq.setType(btc_messages.RequestType.SYNC_ACCOUNT);
    btcReq.setSubmessage(syncReq.serializeBinary());
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createGetBalanceRequest = function(uid) {
    var balanceReq = new btc_messages.GetBalanceRequest()
    balanceReq.setAccUid(uid);
    var btcReq = new btc_messages.Request();
    btcReq.setType(btc_messages.RequestType.GET_ACCOUNT_BALANCE);
    btcReq.setSubmessage(balanceReq.serializeBinary());
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createSetSettingsRequest = function(path) {
    var configuration = new core_config.LibCoreConfiguration();
    configuration.setWorkingDir(path);
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.SET_CONFIGURATION);
    req.setRequestBody(configuration.serializeBinary());
    return req;
}

async function createAndSyncAccount(callbacker, outputFunction, path) {
    var resp = commands.CoreResponse.deserializeBinary(await callbacker.sendRequest(createSetSettingsRequest(path).serializeBinary()));
    if (resp.getError()) throw resp.getError();
    
    resp = commands.CoreResponse.deserializeBinary(await callbacker.sendRequest(createBitcoinCreateAccountRequest().serializeBinary()));
    if (resp.getError()) throw resp.getError();
    var createAccResp = btc_messages.CreateAccountResponse.deserializeBinary(resp.getResponseBody());
    var account = createAccResp.getCreatedAccount();
    outputFunction("Account was created :" + account.getUid());
    resp = commands.CoreResponse.deserializeBinary(await callbacker.sendRequest(createSyncAccountRequest(account.getUid()).serializeBinary()));
    if (resp.getError()) throw resp.getError();
    outputFunction("Sync finished");
    resp = commands.CoreResponse.deserializeBinary(await callbacker.sendRequest(createGetBalanceRequest(account.getUid()).serializeBinary()))
    if (resp.getError()) throw resp.getError();
    var balanceResp = btc_messages.GetBalanceResponse.deserializeBinary(resp.getResponseBody());
    outputFunction("Balance = " + balanceResp.getAmount().getValue());
}

run_test_logic = function(callbacker, outputFunction, path) {
    try {
        versionReq = createGetVersionRequest();
        callbacker.sendRequest(versionReq)
            .then((data) => {
                var resp = commands.CoreResponse.deserializeBinary(data);
                if (resp.error) throw resp.error;
                var versionResp = commands.GetVersionResponse.deserializeBinary(resp.getResponseBody());
                outputFunction("lib-core version: " + versionResp.getMajor() + "." + versionResp.getMinor() + "." + versionResp.getPatch())
            })
            .catch((err)=> outputFunction(err));
        createAndSyncAccount(callbacker, outputFunction, path)
            .then((data)=>outputFunction(data))
            .catch((e)=>outputFunction(e));
    }
    catch (error) {
        outputFunction(error.message);
    }
}