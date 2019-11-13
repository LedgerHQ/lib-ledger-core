const {run_test_logic} = require('./create_sync_balance.js')
const binding = require('bindings')('ledger-core')
const services = require('./messages/services_pb.js')
const https = require('https');
const URL = require('url').URL;
const {UbinderOnPromises} = require('../core/lib/ubinder/src/js_common/UbinderOnPromises.js');
const axios = require('axios');


function OnRequest(data, callback) {
    const serviceReq = services.ServiceRequest.deserializeBinary(data);
    if (serviceReq.getType() == services.ServiceRequestType.HTTP_REQ) {
        const httpReq = services.HttpRequest.deserializeBinary(serviceReq.getRequestBody());
        const method = httpReq.getMethod();
        const headersMap = httpReq.getHeadersMap();
        let dataStr = httpReq.getBody();
        const headers = {};
        headersMap.forEach((v, k) => {
            headers[k] = v;
        });
        let res;
        const param = {
            method,
            headers
        };
        param.url = httpReq.getUrl();
        if (dataStr != "") {
            param.data = dataStr;
        }
        console.log(param);
        axios(param)
          .then((resp) => {
              const serviceResp = new services.ServiceResponse();
              const respMessage = new services.HttpResponse();
              respMessage.setCode(resp.status);
              respMessage.setBody(JSON.stringify(resp.data));
              serviceResp.setResponseBody(respMessage.serializeBinary());
              callback(serviceResp.serializeBinary());
              })
          .catch((err) => {
              console.log(err);
              const serviceResp = new services.ServiceResponse();
              serviceResp.setError(err.message);
              callback(serviceResp.serializeBinary());
          });
    }
}

function OnNotification(data) {

}

function OnExit() {
    process.exit(0);
}



const run_test = async () =>  {
    const callbacker = new UbinderOnPromises(binding, OnNotification, OnRequest, OnExit);
    const result = await run_test_logic(callbacker, console.log, "data");
    callbacker.exit();
}

module.exports.run_test = run_test;
