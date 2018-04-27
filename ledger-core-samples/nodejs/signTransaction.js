const { stringToBytesArray, createBitcoinLikeHelper } = require("./helpers");

async function signTransaction(hwApp, transaction) {
  const inputs = transaction.getInputs();

  // const s = transaction.serialize();
  // const bitcoinLikeHelper = createBitcoinLikeHelper();

  const transformedInputs = await inputs.map(async input => {
    // TODO: make this work
    const previousTransaction = await input.getPreviousTransaction();
    console.log(previousTransaction);
    // const previousHash = input.getPreviousTxHash();

    // NOPE
    // const previousHashBytesArray = stringToBytesArray(previousHash);
    // const transaction = bitcoinLikeHelper.parseTransaction(
    //   previousHashBytesArray
    // );
    console.log(previousHash);
    return previousHash;
    // return hwApp.splitTransaction(input.getPreviousTxHash());
  });
  console.log(transformedInputs);
}

module.exports = signTransaction;
