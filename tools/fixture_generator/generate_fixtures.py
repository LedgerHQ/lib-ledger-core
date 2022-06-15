#!/usr/bin/env python3

import sys
import requests
import json
import datetime
import dateutil.parser
import time

if len(sys.argv) < 3:
    print("Should have at least two params <coin> <name> <address> <xpub>")
    sys.exit(-1)

path = "../../core/test/fixtures/"
coinTypeDict = {'btc': 1, 'btc_testnet': 1, 'abc': 145, 'btg': 156, 'zec': 133,'eth': 60}

arguments = sys.argv[1:]
coin = str(arguments[0])
namespace = str(arguments[1])
address = (arguments[2])
xpub = (arguments[3])
if len(arguments) > 4:
  limitDate = int(arguments[4])
else:
  limitDate = None

cointType = coinTypeDict.get(coin, 1)

def getHashs(txsHash):

  #Get sync token
  syncUrl = 'https://explorers.api.live.ledger.com/blockchain/v2/' + coin + '/syncToken'
  token = requests.get(syncUrl)

  token = json.loads(token.content)['token']
  headers = {'X-LedgerWallet-SyncToken' : token}


  #Get txs related to address
  url = 'https://explorers.api.live.ledger.com/blockchain/v2/'+ coin +'/addresses/' + address + '/transactions'
  call = requests.get(url, headers = headers)

  bytes = call.content
  text = bytes.decode('utf8')
  response = json.loads(text)
  for i in range(len(response['txs'])) :
    txsHash.append(response['txs'][i]['hash'])

  requests.delete(syncUrl, headers = headers)
  return txsHash

def getTxs(hashs):
  txs = []
  url = 'https://explorers.api.live.ledger.com/blockchain/v2/'+ coin +'/transactions/'
  for i in range(len(hashs)):
    bytes = requests.get(url+hashs[-i-1]).content
    text = bytes.decode('utf8')

    tx = json.loads(text)[0]
    if limitDate and time.mktime(dateutil.parser.parse(tx["received_at"]).timetuple()) > limitDate:
      break
    else:
      txs.append(tx)
  return txs

start = time.time()

if coin == 'eth':
    classPrefix = 'Ethereum'
    extension = '.h'
    parser = 'EthereumLikeTransactionParser'
else:
    classPrefix = 'Bitcoin'
    extension = '.hpp'
    parser = 'TransactionParser'

def makeH(namespace, txs):
  data = ['// This file was GENERATED by command:\n', '//     generate_fixtures.py\n', '// DO NOT EDIT BY HAND!!!\n', '#ifndef LEDGER_FIXTURES_TOTO_H\n', '#define LEDGER_FIXTURES_TOTO_H\n', '#include <gtest/gtest.h>\n', '#include <UvThreadDispatcher.hpp>\n', '#include <src/database/DatabaseSessionPool.hpp>\n', '#include <NativePathResolver.hpp>\n', '#include <src/wallet/pool/WalletPool.hpp>\n', '#include <CoutLogPrinter.hpp>\n', '#include <src/api/DynamicObject.hpp>\n', '#include <wallet/common/CurrencyBuilder.hpp>\n', '#include <wallet/'+ classPrefix.lower()+'/explorers/api/'+parser+'.hpp>\n' , '#include <wallet/'+ classPrefix.lower()+'/'+classPrefix+'LikeWallet'+extension+'>\n', '#include <wallet/'+ classPrefix.lower() +'/database/'+classPrefix+'LikeWalletDatabase.h>\n', '#include <wallet/'+ classPrefix.lower() +'/database/'+ classPrefix +'LikeTransactionDatabaseHelper.h>\n', '#include <wallet/common/database/AccountDatabaseHelper.h>\n', '#include <wallet/pool/database/PoolDatabaseHelper.hpp>\n', '#include <utils/JSONUtils.h>\n', '#include <async/async_wait.h>\n', '#include <wallet/'+ classPrefix.lower() +'/'+classPrefix+'LikeAccount'+extension+'>\n', '#include <events/LambdaEventReceiver.hpp>\n']

  if coin != 'eth':
    data = data + ['#include <api/BitcoinLikeInput.hpp>\n', '#include <api/BitcoinLikeOutput.hpp>\n']

  externs = []
  externs.append("\t\t\textern core::api::ExtendedKeyAccountCreationInfo XPUB_INFO;\n")
  for i in range(len(txs)):
    externs.append("\t\t\textern const std::string TX_"+str(i+1)+";\n")
  externs.append("\n")
  externs.append("\t\t\tstd::shared_ptr<core::"+classPrefix+"LikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet);\n")
  newLines = ["namespace ledger {\n","\tnamespace testing {\n","\t\tnamespace "+namespace+" {\n"]+externs+["\t\t}\n","\t}\n", "}\n"]
  result = data+["\n"]+newLines+["\n"]
  result[3] = "#ifndef LEDGER_FIXTURES_"+namespace.upper()+"\n"
  result[4] = "#define LEDGER_FIXTURES_"+namespace.upper()+"\n"
  result.append("#endif // LEDGER_FIXTURES_"+namespace.upper()+"\n")

  with open(path+namespace+'_fixtures.h', 'w+') as file:
    file.writelines(result)
    file.close()



def makeCPP(namespace, txs):
  data = [
    "// This file was GENERATED by command:\n",
    "//     generate_fixtures.py\n",
    "// DO NOT EDIT BY HAND!!!\n"
  ]
  newLines = []
  newLines.append("#include \""+namespace+'_fixtures.h'+"\"\n")
  newLines.append("\n")
  newLines.append("namespace ledger {\n")
  newLines.append("\tnamespace testing {\n")
  newLines.append("\t\tnamespace "+namespace+" {\n")
  apiCalls = []
  apiCalls.append("core::api::ExtendedKeyAccountCreationInfo XPUB_INFO(\n")
  apiCalls.append('        0, {"'+coin+'"}, {"49\'/1\'/0\'"}, {"'+xpub+'"}\n')
  apiCalls.append(');\n')
  apiCalls.append("std::shared_ptr<core::"+classPrefix+"LikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet) {\n")
  apiCalls.append("\tauto account = std::dynamic_pointer_cast<core::"+classPrefix+"LikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));\n")
  apiCalls.append("\tsoci::session sql(pool->getDatabaseSessionPool()->getPool());\n")
  apiCalls.append("\tsql.begin();")
  for i,tx in enumerate(txs):
    apiCalls.append("\taccount->putTransaction(sql, *core::JSONUtils::parse<core::"+parser+">(TX_" + str(i+1) + "));\n")
  apiCalls.append("\tsql.commit();\n")
  apiCalls.append("\treturn account;\n")
  apiCalls.append("}\n")
  txLines = []
  for i,tx in enumerate(txs):
    txLines.append(('const std::string TX_'+str(i+1)+' = "'+json.dumps(tx).replace('"','\\"')+'";\n'))
  namespacedLines = apiCalls+txLines
  for idx, line in enumerate(namespacedLines):
    namespacedLines[idx] = "\t\t\t"+line
  newLines += namespacedLines + ["\t\t}\n","\t}\n", "}\n"]
  result = data+newLines
  with open(path+namespace+'_fixtures.cpp', 'w+') as file:
    file.writelines(result)
    file.close()


makeH(namespace, getTxs(getHashs([])))

end = time.time()

print("make H over after "+str(end-start))

makeCPP(namespace, getTxs(getHashs([])))

end2 = time.time()

print("make cpp over after "+str(end2-start))
