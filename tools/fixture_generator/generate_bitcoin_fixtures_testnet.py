#!/usr/bin/env python3

import sys
import requests
import json
import datetime
import dateutil.parser
import time

if len(sys.argv) < 3:
    print("Should have at least two params <name> <address> <xpub>")
    sys.exit(-1)

path = "../../core/test/fixtures/"

arguments = sys.argv[1:]
namespace = str(arguments[0])
address = (arguments[1])
xpub = (arguments[2])
if len(arguments) > 3:
  limitDate = int(arguments[3])
else:
  limitDate = None

def getHashs(txsHash, offset=0):
  url = 'https://api.blockcypher.com/v1/btc/test3/addrs/' + address
  call = requests.get(url)
  bytes = call.content
  text = bytes.decode('utf8')
  response = json.loads(text)
  for i in range(len(response['txrefs'])) :
    txsHash.append(response['txrefs'][i]['tx_hash'])
  if i == 49:
    offset += 50
    return getHashs(txsHash, offset=offset)
  else:
    return txsHash

def getTxs(hashs):
  txs = []
  url = 'https://api.ledgerwallet.com/blockchain/v2/btc_testnet/transactions/'
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

def makeH(namespace, txs):
  data = ['// This file was GENERATED by command:\n', '//     generate_bitcoin_fixtures.py\n', '// DO NOT EDIT BY HAND!!!\n', '#ifndef LEDGER_FIXTURES_TOTO_H\n', '#define LEDGER_FIXTURES_TOTO_H\n', '#include <gtest/gtest.h>\n', '#include <async/QtThreadDispatcher.hpp>\n', '#include <src/database/DatabaseSessionPool.hpp>\n', '#include <NativePathResolver.hpp>\n', '#include <unordered_set>\n', '#include <src/wallet/pool/WalletPool.hpp>\n', '#include <CoutLogPrinter.hpp>\n', '#include <src/api/DynamicObject.hpp>\n', '#include <wallet/common/CurrencyBuilder.hpp>\n', '#include <wallet/bitcoin/BitcoinLikeWallet.hpp>\n', '#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>\n', '#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>\n', '#include <wallet/common/database/AccountDatabaseHelper.h>\n', '#include <wallet/pool/database/PoolDatabaseHelper.hpp>\n', '#include <utils/JSONUtils.h>\n', '#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>\n', '#include <async/async_wait.h>\n', '#include <wallet/bitcoin/BitcoinLikeAccount.hpp>\n', '#include <api/BitcoinLikeOperation.hpp>\n', '#include <api/BitcoinLikeTransaction.hpp>\n', '#include <api/BitcoinLikeInput.hpp>\n', '#include <api/BitcoinLikeOutput.hpp>\n', '#include <api/BigInt.hpp>\n', '#include <net/QtHttpClient.hpp>\n', '#include <events/LambdaEventReceiver.hpp>\n', '#include <soci.h>\n', '#include <api/Account.hpp>\n', '#include <api/BitcoinLikeAccount.hpp>\n']
  externs = []
  externs.append("\t\t\textern core::api::ExtendedKeyAccountCreationInfo XPUB_INFO;\n")
  for i in range(len(txs)):
    externs.append("\t\t\textern const std::string TX_"+str(i+1)+";\n")
  externs.append("\n")
  externs.append("\t\t\tstd::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet);\n")
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
    "//     generate_bitcoin_fixtures.py\n",
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
  apiCalls.append('        0, {"testnet"}, {"49\'/1\'/0\'"}, {"'+xpub+'"}\n')
  apiCalls.append(');\n')
  apiCalls.append("std::shared_ptr<core::BitcoinLikeAccount> inflate(const std::shared_ptr<core::WalletPool>& pool, const std::shared_ptr<core::AbstractWallet>& wallet) {\n")
  apiCalls.append("\tauto account = std::dynamic_pointer_cast<core::BitcoinLikeAccount>(wait(wallet->newAccountWithExtendedKeyInfo(XPUB_INFO)));\n")
  apiCalls.append("\tsoci::session sql(pool->getDatabaseSessionPool()->getPool());\n")
  apiCalls.append("\tsql.begin();")
  for i,tx in enumerate(txs):
    apiCalls.append("\taccount->putTransaction(sql, *core::JSONUtils::parse<core::TransactionParser>(TX_" + str(i+1) + "));\n")
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


makeH(namespace, getTxs(getHashs([],0)))

end = time.time()

print("make H over after "+str(end-start))

makeCPP(namespace, getTxs(getHashs([],0)))

end2 = time.time()

print("make cpp over after "+str(end2-start))
