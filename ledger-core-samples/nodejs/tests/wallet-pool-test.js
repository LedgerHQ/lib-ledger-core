const CommNodeHid = require("@ledgerhq/hw-transport-node-hid").default;
const Btc = require("@ledgerhq/hw-app-btc").default;

const {
  createWallet,
  createAccount,
  createAmount,
  getCurrency,
  syncAccount,
  EVENT_CODE
} = require("../index");

waitForDevices(async device => {
  try {
    console.log(`> Creating transport`);
    const transport = await CommNodeHid.open(device.path);

    console.log(`> Instanciate BTC app`);
    const hwApp = new Btc(transport);

    console.log(`> Get currency`);
    const currency = await getCurrency("bitcoin_testnet");

    console.log(`> Create wallet`);
    const wallet = await createWallet("khalil", currency);

    console.log(`> Create account`);
    const account = await createAccount(wallet, hwApp);

    console.log(`> Sync account`);
    await syncAccount(account);

    console.log(`> Create transaction`);
    const transaction = await createTransaction(wallet, account);

    console.log(transaction);
    process.exit(0);
    // console.log(account.getIndex());
    // console.log(account.isSynchronizing());
  } catch (err) {
    console.log(err);
  }
});

function waitForDevices(onDevice) {
  console.log(`>> Waiting for device...`);
  CommNodeHid.listen({
    error: () => {},
    complete: () => {},
    next: async e => {
      if (!e.device) {
        return;
      }
      if (e.type === "add") {
        console.log(`added ${JSON.stringify(e)}`);
        onDevice(e.device);
      }
      if (e.type === "remove") {
        console.log(`removed ${JSON.stringify(e)}`);
      }
    }
  });
}

async function createTransaction(wallet, account) {
  const ADDRESS_TO_SEND = "mzaYScsZFRECzTnn6kbcbK2UcX6bri5Ck4";
  const balance = await account.getBalance();

  const bitcoinLikeAccount = account.asBitcoinLikeAccount();
  const walletCurrency = wallet.getCurrency();
  const amount = createAmount(walletCurrency, 10000);
  const fees = createAmount(walletCurrency, 10);

  const transactionBuilder = bitcoinLikeAccount.buildTransaction();
  transactionBuilder.sendToAddress(amount, ADDRESS_TO_SEND);
  transactionBuilder.pickInputs(0, 0xffffff);
  transactionBuilder.setFeesPerByte(fees);

  return transactionBuilder.build();
}
