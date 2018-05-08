// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#include "NJSOperationCpp.hpp"

using namespace v8;
using namespace node;
using namespace std;

NAN_METHOD(NJSOperation::getUid) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getUid needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getUid : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getUid();

    //Wrap result in node object
    auto arg_0 = Nan::New<String>(result).ToLocalChecked();

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getAccountIndex) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getAccountIndex needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getAccountIndex : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getAccountIndex();

    //Wrap result in node object
    auto arg_0 = Nan::New<Int32>(result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getOperationType) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getOperationType needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getOperationType : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getOperationType();

    //Wrap result in node object
    auto arg_0 = Nan::New<Integer>((int)result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getDate) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getDate needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getDate : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getDate();

    //Wrap result in node object
    auto date_arg_0 = chrono::duration_cast<chrono::seconds>(result.time_since_epoch()).count();
    auto arg_0 = Nan::New<Date>(date_arg_0).ToLocalChecked();

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getSenders) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getSenders needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getSenders : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getSenders();

    //Wrap result in node object
    Local<Array> arg_0 = Nan::New<Array>();
    for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
    {
        auto arg_0_elem = Nan::New<String>(result[arg_0_id]).ToLocalChecked();
        arg_0->Set((int)arg_0_id,arg_0_elem);
    }


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getRecipients) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getRecipients needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getRecipients : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getRecipients();

    //Wrap result in node object
    Local<Array> arg_0 = Nan::New<Array>();
    for(size_t arg_0_id = 0; arg_0_id < result.size(); arg_0_id++)
    {
        auto arg_0_elem = Nan::New<String>(result[arg_0_id]).ToLocalChecked();
        arg_0->Set((int)arg_0_id,arg_0_elem);
    }


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getAmount) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getAmount needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getAmount : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getAmount();

    //Wrap result in node object
    auto arg_0_wrap = NJSAmount::wrap(result);
    auto arg_0 = Nan::ObjectWrap::Unwrap<NJSAmount>(arg_0_wrap)->handle();


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getFees) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getFees needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getFees : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getFees();

    //Wrap result in node object
    auto arg_0_wrap = NJSAmount::wrap(result);
    auto arg_0 = Nan::ObjectWrap::Unwrap<NJSAmount>(arg_0_wrap)->handle();



    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getPreferences) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getPreferences needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getPreferences : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getPreferences();

    //Wrap result in node object
    auto arg_0_wrap = NJSPreferences::wrap(result);
    auto arg_0 = Nan::ObjectWrap::Unwrap<NJSPreferences>(arg_0_wrap)->handle();


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getTrust) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getTrust needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getTrust : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getTrust();

    //Wrap result in node object
    auto arg_0_wrap = NJSTrustIndicator::wrap(result);
    auto arg_0 = Nan::ObjectWrap::Unwrap<NJSTrustIndicator>(arg_0_wrap)->handle();


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getBlockHeight) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getBlockHeight needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getBlockHeight : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getBlockHeight();

    //Wrap result in node object
    Local<Value> arg_0;
    if(result)
    {
        auto arg_0_optional = (result).value();
        auto arg_0_tmp = Nan::New<Number>(arg_0_optional);
        arg_0 = arg_0_tmp;
    }


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::asBitcoinLikeOperation) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::asBitcoinLikeOperation needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::asBitcoinLikeOperation : implementation of Operation is not valid");
    }

    auto result = cpp_impl->asBitcoinLikeOperation();

    //Wrap result in node object
    auto arg_0_wrap = NJSBitcoinLikeOperation::wrap(result);
    auto arg_0 = Nan::ObjectWrap::Unwrap<NJSBitcoinLikeOperation>(arg_0_wrap)->handle();


    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::isInstanceOfBitcoinLikeOperation) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfBitcoinLikeOperation needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfBitcoinLikeOperation : implementation of Operation is not valid");
    }

    auto result = cpp_impl->isInstanceOfBitcoinLikeOperation();

    //Wrap result in node object
    auto arg_0 = Nan::New<Boolean>(result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::isInstanceOfEthereumLikeOperation) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfEthereumLikeOperation needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfEthereumLikeOperation : implementation of Operation is not valid");
    }

    auto result = cpp_impl->isInstanceOfEthereumLikeOperation();

    //Wrap result in node object
    auto arg_0 = Nan::New<Boolean>(result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::isInstanceOfRippleLikeOperation) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfRippleLikeOperation needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::isInstanceOfRippleLikeOperation : implementation of Operation is not valid");
    }

    auto result = cpp_impl->isInstanceOfRippleLikeOperation();

    //Wrap result in node object
    auto arg_0 = Nan::New<Boolean>(result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::isComplete) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::isComplete needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::isComplete : implementation of Operation is not valid");
    }

    auto result = cpp_impl->isComplete();

    //Wrap result in node object
    auto arg_0 = Nan::New<Boolean>(result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getWalletType) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getWalletType needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getWalletType : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getWalletType();

    //Wrap result in node object
    auto arg_0 = Nan::New<Integer>((int)result);

    //Return result
    info.GetReturnValue().Set(arg_0);
}
NAN_METHOD(NJSOperation::getCurrency) {

    //Check if method called with right number of arguments
    if(info.Length() != 0)
    {
        return Nan::ThrowError("NJSOperation::getCurrency needs 0 arguments");
    }

    //Check if parameters have correct types

    //Unwrap current object and retrieve its Cpp Implementation
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_impl = obj->getCppImpl();
    if(!cpp_impl)
    {
        return Nan::ThrowError("NJSOperation::getCurrency : implementation of Operation is not valid");
    }

    auto result = cpp_impl->getCurrency();

    //Wrap result in node object
    auto arg_0 = Nan::New<Object>();
    auto arg_0_1 = Nan::New<Integer>((int)result.walletType);
    Nan::DefineOwnProperty(arg_0, Nan::New<String>("walletType").ToLocalChecked(), arg_0_1);
    auto arg_0_2 = Nan::New<String>(result.name).ToLocalChecked();
    Nan::DefineOwnProperty(arg_0, Nan::New<String>("name").ToLocalChecked(), arg_0_2);
    auto arg_0_3 = Nan::New<Int32>(result.bip44CoinType);
    Nan::DefineOwnProperty(arg_0, Nan::New<String>("bip44CoinType").ToLocalChecked(), arg_0_3);
    auto arg_0_4 = Nan::New<String>(result.paymentUriScheme).ToLocalChecked();
    Nan::DefineOwnProperty(arg_0, Nan::New<String>("paymentUriScheme").ToLocalChecked(), arg_0_4);
    Local<Array> arg_0_5 = Nan::New<Array>();
    for(size_t arg_0_5_id = 0; arg_0_5_id < result.units.size(); arg_0_5_id++)
    {
        auto arg_0_5_elem = Nan::New<Object>();
        auto arg_0_5_elem_1 = Nan::New<String>(result.units[arg_0_5_id].name).ToLocalChecked();
        Nan::DefineOwnProperty(arg_0_5_elem, Nan::New<String>("name").ToLocalChecked(), arg_0_5_elem_1);
        auto arg_0_5_elem_2 = Nan::New<String>(result.units[arg_0_5_id].symbol).ToLocalChecked();
        Nan::DefineOwnProperty(arg_0_5_elem, Nan::New<String>("symbol").ToLocalChecked(), arg_0_5_elem_2);
        auto arg_0_5_elem_3 = Nan::New<String>(result.units[arg_0_5_id].code).ToLocalChecked();
        Nan::DefineOwnProperty(arg_0_5_elem, Nan::New<String>("code").ToLocalChecked(), arg_0_5_elem_3);
        auto arg_0_5_elem_4 = Nan::New<Int32>(result.units[arg_0_5_id].numberOfDecimal);
        Nan::DefineOwnProperty(arg_0_5_elem, Nan::New<String>("numberOfDecimal").ToLocalChecked(), arg_0_5_elem_4);

        arg_0_5->Set((int)arg_0_5_id,arg_0_5_elem);
    }

    Nan::DefineOwnProperty(arg_0, Nan::New<String>("units").ToLocalChecked(), arg_0_5);
    Local<Value> arg_0_6;
    if(result.bitcoinLikeNetworkParameters)
    {
        auto arg_0_6_optional = (result.bitcoinLikeNetworkParameters).value();
        auto arg_0_6_tmp = Nan::New<Object>();
        auto arg_0_6_tmp_1 = Nan::New<String>(arg_0_6_optional.Identifier).ToLocalChecked();
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("Identifier").ToLocalChecked(), arg_0_6_tmp_1);
        Local<Array> arg_0_6_tmp_2 = Nan::New<Array>();
        for(size_t arg_0_6_tmp_2_id = 0; arg_0_6_tmp_2_id < arg_0_6_optional.P2PKHVersion.size(); arg_0_6_tmp_2_id++)
        {
            auto arg_0_6_tmp_2_elem = Nan::New<Uint32>(arg_0_6_optional.P2PKHVersion[arg_0_6_tmp_2_id]);
            arg_0_6_tmp_2->Set((int)arg_0_6_tmp_2_id,arg_0_6_tmp_2_elem);
        }

        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("P2PKHVersion").ToLocalChecked(), arg_0_6_tmp_2);
        Local<Array> arg_0_6_tmp_3 = Nan::New<Array>();
        for(size_t arg_0_6_tmp_3_id = 0; arg_0_6_tmp_3_id < arg_0_6_optional.P2SHVersion.size(); arg_0_6_tmp_3_id++)
        {
            auto arg_0_6_tmp_3_elem = Nan::New<Uint32>(arg_0_6_optional.P2SHVersion[arg_0_6_tmp_3_id]);
            arg_0_6_tmp_3->Set((int)arg_0_6_tmp_3_id,arg_0_6_tmp_3_elem);
        }

        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("P2SHVersion").ToLocalChecked(), arg_0_6_tmp_3);
        Local<Array> arg_0_6_tmp_4 = Nan::New<Array>();
        for(size_t arg_0_6_tmp_4_id = 0; arg_0_6_tmp_4_id < arg_0_6_optional.XPUBVersion.size(); arg_0_6_tmp_4_id++)
        {
            auto arg_0_6_tmp_4_elem = Nan::New<Uint32>(arg_0_6_optional.XPUBVersion[arg_0_6_tmp_4_id]);
            arg_0_6_tmp_4->Set((int)arg_0_6_tmp_4_id,arg_0_6_tmp_4_elem);
        }

        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("XPUBVersion").ToLocalChecked(), arg_0_6_tmp_4);
        auto arg_0_6_tmp_5 = Nan::New<Integer>((int)arg_0_6_optional.FeePolicy);
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("FeePolicy").ToLocalChecked(), arg_0_6_tmp_5);
        auto arg_0_6_tmp_6 = Nan::New<Number>(arg_0_6_optional.DustAmount);
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("DustAmount").ToLocalChecked(), arg_0_6_tmp_6);
        auto arg_0_6_tmp_7 = Nan::New<String>(arg_0_6_optional.MessagePrefix).ToLocalChecked();
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("MessagePrefix").ToLocalChecked(), arg_0_6_tmp_7);
        auto arg_0_6_tmp_8 = Nan::New<Boolean>(arg_0_6_optional.UsesTimestampedTransaction);
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("UsesTimestampedTransaction").ToLocalChecked(), arg_0_6_tmp_8);
        auto arg_0_6_tmp_9 = Nan::New<Number>(arg_0_6_optional.TimestampDelay);
        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("TimestampDelay").ToLocalChecked(), arg_0_6_tmp_9);
        Local<Array> arg_0_6_tmp_10 = Nan::New<Array>();
        for(size_t arg_0_6_tmp_10_id = 0; arg_0_6_tmp_10_id < arg_0_6_optional.SigHash.size(); arg_0_6_tmp_10_id++)
        {
            auto arg_0_6_tmp_10_elem = Nan::New<Uint32>(arg_0_6_optional.SigHash[arg_0_6_tmp_10_id]);
            arg_0_6_tmp_10->Set((int)arg_0_6_tmp_10_id,arg_0_6_tmp_10_elem);
        }

        Nan::DefineOwnProperty(arg_0_6_tmp, Nan::New<String>("SigHash").ToLocalChecked(), arg_0_6_tmp_10);

        arg_0_6 = arg_0_6_tmp;
    }

    Nan::DefineOwnProperty(arg_0, Nan::New<String>("bitcoinLikeNetworkParameters").ToLocalChecked(), arg_0_6);


    //Return result
    info.GetReturnValue().Set(arg_0);
}

NAN_METHOD(NJSOperation::New) {
    //Only new allowed
    if(!info.IsConstructCall())
    {
        return Nan::ThrowError("NJSOperation function can only be called as constructor (use New)");
    }
    NJSOperation *node_instance = new NJSOperation(nullptr);

    if(node_instance)
    {
        //Wrap and return node instance
        node_instance->Wrap(info.This());
        node_instance->Ref();
        info.GetReturnValue().Set(info.This());
    }
}


Nan::Persistent<ObjectTemplate> NJSOperation::Operation_prototype;

Handle<Object> NJSOperation::wrap(const std::shared_ptr<ledger::core::api::Operation> &object) {
    Nan::HandleScope scope;
    Local<ObjectTemplate> local_prototype = Nan::New(Operation_prototype);

    Handle<Object> obj;
    if(!local_prototype.IsEmpty())
    {
        obj = local_prototype->NewInstance();
        NJSOperation *new_obj = new NJSOperation(object);
        if(new_obj)
        {
            new_obj->Wrap(obj);
            new_obj->Ref();
        }
    }
    else
    {
        Nan::ThrowError("NJSOperation::wrap: object template not valid");
    }
    return obj;
}

NAN_METHOD(NJSOperation::isNull) {
    NJSOperation* obj = Nan::ObjectWrap::Unwrap<NJSOperation>(info.This());
    auto cpp_implementation = obj->getCppImpl();
    auto isNull = !cpp_implementation ? true : false;
    return info.GetReturnValue().Set(Nan::New<Boolean>(isNull));
}

void NJSOperation::Initialize(Local<Object> target) {
    Nan::HandleScope scope;

    Local<FunctionTemplate> func_template = Nan::New<FunctionTemplate>(NJSOperation::New);
    Local<ObjectTemplate> objectTemplate = func_template->InstanceTemplate();
    objectTemplate->SetInternalFieldCount(1);

    func_template->SetClassName(Nan::New<String>("NJSOperation").ToLocalChecked());

    //SetPrototypeMethod all methods
    Nan::SetPrototypeMethod(func_template,"getUid", getUid);
    Nan::SetPrototypeMethod(func_template,"getAccountIndex", getAccountIndex);
    Nan::SetPrototypeMethod(func_template,"getOperationType", getOperationType);
    Nan::SetPrototypeMethod(func_template,"getDate", getDate);
    Nan::SetPrototypeMethod(func_template,"getSenders", getSenders);
    Nan::SetPrototypeMethod(func_template,"getRecipients", getRecipients);
    Nan::SetPrototypeMethod(func_template,"getAmount", getAmount);
    Nan::SetPrototypeMethod(func_template,"getFees", getFees);
    Nan::SetPrototypeMethod(func_template,"getPreferences", getPreferences);
    Nan::SetPrototypeMethod(func_template,"getTrust", getTrust);
    Nan::SetPrototypeMethod(func_template,"getBlockHeight", getBlockHeight);
    Nan::SetPrototypeMethod(func_template,"asBitcoinLikeOperation", asBitcoinLikeOperation);
    Nan::SetPrototypeMethod(func_template,"isInstanceOfBitcoinLikeOperation", isInstanceOfBitcoinLikeOperation);
    Nan::SetPrototypeMethod(func_template,"isInstanceOfEthereumLikeOperation", isInstanceOfEthereumLikeOperation);
    Nan::SetPrototypeMethod(func_template,"isInstanceOfRippleLikeOperation", isInstanceOfRippleLikeOperation);
    Nan::SetPrototypeMethod(func_template,"isComplete", isComplete);
    Nan::SetPrototypeMethod(func_template,"getWalletType", getWalletType);
    Nan::SetPrototypeMethod(func_template,"getCurrency", getCurrency);
    //Set object prototype
    Operation_prototype.Reset(objectTemplate);
    Nan::SetPrototypeMethod(func_template,"isNull", isNull);

    //Add template to target
    target->Set(Nan::New<String>("NJSOperation").ToLocalChecked(), func_template->GetFunction());
}
