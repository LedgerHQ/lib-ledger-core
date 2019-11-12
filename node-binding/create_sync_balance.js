require('./messages/commands_pb.js');
const {LibCoreConfiguration} = require('./messages/core_configuration_pb');
const {GetBalanceRequest, BitcoinRequest, GetBalanceResponse} = require('./messages/bitcoin/commands_pb.js');
const {
    CoreRequest,
    CoreRequestType,
    CoreResponse,
    GetVersionResponse
} = require('./messages/commands_pb.js');
const {ServiceRequest} = require('./messages/services_pb');
createGetVersionRequest = function() {
    const req = new CoreRequest();
    req.setRequestType(CoreRequestType.GET_VERSION);
    return req;
}


createBitcoinCreateAccountRequest = function() {
    
    const btcReq = new Request();
    btcReq.setType(RequestType.CREATE_ACCOUNT);
    btcReq.setSubmessage(createAccReq.serializeBinary());
    const req = new CoreRequest();
    req.setRequestType(CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createSyncAccountRequest = function(accId) {
    const syncReq = new proto.ledger.core.message.bitcoin.SyncAccountRequest();
    syncReq.setAccountId(accId)
    const bitcoinRequest = new proto.ledger.core.message.bitcoin.BitcoinRequest();
    bitcoinRequest.setSyncAccount(syncReq);
    const req = new proto.ledger.core.message.CoreRequest();
    req.setRequestType(proto.ledger.core.message.CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(bitcoinRequest.serializeBinary());
    return req;
}

createGetBalanceRequest = function(uid) {
    const balanceReq = new GetBalanceRequest()
    balanceReq.setAccountId(uid);
    const btcReq = new CoreRequest();
    btcReq.setRequestType(CoreRequestType.GET_ACCOUNT_BALANCE);
    btcReq.setRequestBody(balanceReq.serializeBinary());
    const req = new CoreRequest();
    req.setRequestType(CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createSetSettingsRequest = function(path) {
    const configuration = new LibCoreConfiguration();
    configuration.setWorkingDir(path);
    const req = new CoreRequest();
    req.setRequestType(CoreRequestType.SET_CONFIGURATION);
    req.setRequestBody(configuration.serializeBinary());
    return req;
}

async function createAndSyncAccount(callbacker, outputFunction, path, accId) {
    let resp = CoreResponse.deserializeBinary(await callbacker.sendRequest(createSetSettingsRequest(path).serializeBinary()));
    if (resp.getError()) throw resp.getError();
    
    resp = CoreResponse.deserializeBinary(await callbacker.sendRequest(createSyncAccountRequest(accId).serializeBinary()));
    if (resp.getError()) throw resp.getError();
    outputFunction("Sync finished");
    resp = CoreResponse.deserializeBinary(await callbacker.sendRequest(createGetBalanceRequest(accId).serializeBinary()))
    if (resp.getError()) throw resp.getError();
    const balanceResp = GetBalanceResponse.deserializeBinary(resp.getResponseBody());
    outputFunction("Balance = " + balanceResp.getAmount().getValue());
}

run_test_logic = function(callbacker, outputFunction, path) {
    try {
        const accId = new proto.ledger.core.message.bitcoin.AccountID();
        accId.setCurrencyName('bitcoin');
        accId.setXpub("xpub6CQRBg1k8KN2yZfWUywHt9dtDSEFfHhwhBrEjzuHj5YBV2p81NEviAhEhGzYpC5AzwuL6prM2wc1oyMQ8hmsCKqrWwHrjcboQvkBctC1JTq");
        accId.setKeychainEngine(proto.ledger.core.message.bitcoin.KeychainEngine.BIP49_P2SH);
        versionReq = createGetVersionRequest();
        callbacker.sendRequest(versionReq)
            .then((data) => {
                const resp = CoreResponse.deserializeBinary(data);
                if (resp.error) throw resp.error;
                const versionResp = GetVersionResponse.deserializeBinary(resp.getResponseBody());
                outputFunction("lib-core version: " + versionResp.getMajor() + "." + versionResp.getMinor() + "." + versionResp.getPatch())
            })
            .catch((err)=> outputFunction(err));
        createAndSyncAccount(callbacker, outputFunction, path, accId)
            .then((data)=>outputFunction(data))
            .catch((e)=>outputFunction(e));
    }
    catch (error) {
        outputFunction(error.message);
    }
}

module.exports.run_test_logic = run_test_logic;