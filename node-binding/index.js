const binding = require('bindings')('lib-core-node')
const commands = require('./messages/commands_pb.js')

const btc_messages = require('./messages/bitcoin/commands_pb.js')
const services = require('./messages/services_pb.js')
const https = require('https');
const URL = require('url').URL;
const ubinder = require('../core/lib/ubinder/src/node/ubinder');


function OnRequest(data, callback) {
    var serviceReq = services.ServiceRequest.deserializeBinary(data);
    if (serviceReq.getType() == services.ServiceRequestType.HTTP_REQ) {
        var httpReq = services.HttpRequest.deserializeBinary(serviceReq.getRequestBody());
        const method = httpReq.getMethod();
        const headersMap = httpReq.getHeadersMap();
        let dataStr = httpReq.getBody();
        const url = new URL(httpReq.getUrl());
        const headers = {};
        headersMap.forEach((v, k) => {
            headers[k] = v;
        });
        let res;
        const param = {
            method,
            headers
        };
        param.host = url.hostname;
        param.path = url.pathname;
        if (url.port.length != 0) {
            param.port = parseInt(url.port);
        }
        if (dataStr != "") {
            param.data = dataStr;
        }

        https.request(param, (resp) => {
            let data = '';
            var serviceResp = new services.ServiceResponse();
            var respMessage = new services.HttpResponse();
            respMessage.setCode(resp.statusCode);
            // A chunk of data has been recieved.
            resp.on('data', (chunk) => {
                data += chunk;
            });

            resp.on('end', () => {
                respMessage.setBody(data);
                serviceResp.setResponseBody(respMessage.serializeBinary());
                callback(serviceResp.serializeBinary());
            });
        }).on("error", (err) => {
            var serviceResp = new services.ServiceResponse();
            serviceResp.setError(err.message);
            callback(serviceResp.serializeBinary());
        }).end();
    }
}

function OnNotification(data) {
    console.log("OnNotification");
}


callbacker = new ubinder.Callbacker(binding, OnRequest, OnNotification)


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
    accConfig.setKeychainEngine(btc_messages.BIP32_P2WSH);
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
    btcReq.setType(btc_messages.RequestType.GET_BALANCE);
    btcReq.setSubmessage(balanceReq.serializeBinary());
    var req = new commands.CoreRequest();
    req.setRequestType(commands.CoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

async function createAndSyncAccount() {
    var data = await callbacker.sendRequest(createBitcoinCreateAccountRequest().serializeBinary())
    var resp = commands.CoreResponse.deserializeBinary(data);
    if (resp.getError()) {
        throw resp.getError();
    }
    var createAccResp = btc_messages.CreateAccountResponse.deserializeBinary(resp.getResponseBody());
    var account = createAccResp.getCreatedAccount();
    console.log("Account was created :" + account.getUid());
    var syncResp = btc_messages.SyncAccountResponse.deserializeBinary(await callbacker.sendRequest(createSyncAccountRequest(account.getUid()).serializeBinary()));
    console.log("Sync finished");
    var balanceResp = btc_messages.GetBalanceResponse.deserializeBinary(await callbacker.sendRequest(createGetBalanceRequest(account.getUid()).serializeBinary()));
    console.log("Balance = " + balanceResp.getBalance());
    return 1;
}

run_test = function() {
    try {
        versionReq = createGetVersionRequest();
        console.log("GetVersionRequest " + versionReq);
        callbacker.sendRequest(versionReq)
            .then((data) => {
                var resp = commands.CoreResponse.deserializeBinary(data);
                if (resp.error) {
                    console.log(resp.error);
                    throw resp.error;
                }
                var versionResp = commands.GetVersionResponse.deserializeBinary(resp.getResponseBody());
                console.log("lib-core version: " + versionResp.getMajor() + "." + versionResp.getMinor() + "." + versionResp.getPatch())
            })
            .catch((err)=> console.log(err));
        createAndSyncAccount()
            .then((data)=>console.log(data))
            .catch((e)=>console.log(e));
    }
    catch (error) {
        console.log(error.message);
    }
}