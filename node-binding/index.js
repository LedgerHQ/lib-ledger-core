const binding = require('bindings')('lib-core-node')
const messages = require('./messages/messages_pb.js')
const btc_messages = require('./messages/bitcoin_like_pb.js')
const services = require('./messages/services_pb.js')
const https = require('https');
const URL = require('url').URL;

function OnRequest(req, callback, args) {
    console.log("OnRequest")
    var serviceReq = services.ServiceRequest.deserializeBinary(req);
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

            // The whole response has been received. Print out the result.
            resp.on('end', () => {
                console.log("finished receiving data");
                respMessage.setBody(data);
                serviceResp.setResponseBody(respMessage.serializeBinary());
                callback(serviceResp.serializeBinary(), args);
            });
        }).on("error", (err) => {
            console.log("Error: " + err.message);
            var serviceResp = new services.ServiceResponse();
            serviceResp.setError(err.message);
            callback(serviceResp.serializeBinary(), args);
        }).end();
    }
}

function OnNotification(notification) {
    
}

register = function () {
    binding.registerLib(OnRequest, OnNotification);
}

createGetVersionRequest = function() {
    var req = new messages.LibCoreRequest();
    req.setRequestType(messages.LibCoreRequestType.GET_VERSION);
    return req;
}

createBitcoinCreateAccountRequest = function() {
    var createAccReq = new btc_messages.CreateAccountRequest();
    var accConfig = new btc_messages.AccountConfiguration();
    accConfig.setIsSegwit(true);
    accConfig.setCurrency("btc");
    createAccReq.setName("my account1");
    createAccReq.setXpub("xpub6CQRBg1k8KN2yZfWUywHt9dtDSEFfHhwhBrEjzuHj5YBV2p81NEviAhEhGzYpC5AzwuL6prM2wc1oyMQ8hmsCKqrWwHrjcboQvkBctC1JTq");
    createAccReq.setIndex(0);
    createAccReq.setConfig(accConfig);
    var btcReq = new btc_messages.BitcoinLikeRequest();
    btcReq.setType(btc_messages.BitcoinLikeRequestType.CREATE_ACCOUNT);
    btcReq.setSubmessage(createAccReq.serializeBinary());
    var req = new messages.LibCoreRequest();
    req.setRequestType(messages.LibCoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createSyncAccountRequest = function(uid) {
    var syncReq = new btc_messages.SyncAccountRequest();
    syncReq.setAccUid(uid);
    var btcReq = new btc_messages.BitcoinLikeRequest();
    btcReq.setType(btc_messages.BitcoinLikeRequestType.SYNC);
    btcReq.setSubmessage(syncReq.serializeBinary());
    var req = new messages.LibCoreRequest();
    req.setRequestType(messages.LibCoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

createGetBalanceRequest = function(uid) {
    var balanceReq = new btc_messages.GetBalanceRequest()
    balanceReq.setAccUid(uid);
    var btcReq = new btc_messages.BitcoinLikeRequest();
    btcReq.setType(btc_messages.BitcoinLikeRequestType.GET_BALANCE);
    btcReq.setSubmessage(balanceReq.serializeBinary());
    var req = new messages.LibCoreRequest();
    req.setRequestType(messages.LibCoreRequestType.BITCOIN_REQUEST);
    req.setRequestBody(btcReq.serializeBinary());
    return req;
}

sendRequest = function(req) {
    return new Promise((resolve, reject) => {
        var msg = req.serializeBinary();
        binding.sendRequest(msg, 
            function(data){
                var resp = messages.LibCoreResponse.deserializeBinary(data);
                if (resp.getError()=='') {
                    resolve(resp.getResponseBody());
                }else {
                    reject(new Error(resp.getError()));
                }
            });        
        });
}

async function createAndSyncAccount() {
    var createAccResp = btc_messages.CreateAccountResponse.deserializeBinary(await sendRequest(createBitcoinCreateAccountRequest()));
    var accUid = createAccResp.getAccUid()
    console.log("Account was created :" + accUid);
    var syncResp = btc_messages.SyncAccountResponse.deserializeBinary(await sendRequest(createSyncAccountRequest(accUid)));
    console.log("Sync finished");
    var balanceResp = btc_messages.GetBalanceResponse.deserializeBinary(await sendRequest(createGetBalanceRequest(accUid)));
    console.log("Balance = " + balanceResp.getBalance());
}

run_test = function() {
    register();
    versionReq = createGetVersionRequest();                 
    /*
    sendRequest(versionReq)
        .then((resp)=>{
            var versionResp = messages.GetVersionResponse.deserializeBinary(resp);
            console.log("lib-core version: " + versionResp.getMajor() + "." + versionResp.getMinor() + "." + versionResp.getPatch())
        })
        .catch((err)=> console.log(err));
    */
    try {
        createAndSyncAccount();
    }
    catch (error) {
        console.log(error.message());
    }
}