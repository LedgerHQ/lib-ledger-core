const logic = require('./create_sync_balance.js')
const binding = require('bindings')('lib-core-node')
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

}


callbacker = new ubinder.Callbacker(binding, OnNotification, OnRequest)

run_test = function() {
    run_test_logic(callbacker, console.log, "data");
}
