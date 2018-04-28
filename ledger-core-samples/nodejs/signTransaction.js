const {
  stringToBytesArray,
  bytesToHex,
  createBitcoinLikeHelper
} = require("./helpers");

async function signTransaction(hwApp, transaction, isSegwitSupported = true) {
  const rawInputs = transaction.getInputs();

  // const s = transaction.serialize();
  // const bitcoinLikeHelper = createBitcoinLikeHelper();

  const inputs = await Promise.all(
    rawInputs.map(async input => {
      // previous transaction
      const rawPreviousTransactionHash = await input.getPreviousTxHash();
      const rawPreviousTransaction = await input.getPreviousTransaction();
      const hexPreviousTransaction = bytesToHex(rawPreviousTransaction);
      console.log(hexPreviousTransaction);
      const previousTransaction = hwApp.splitTransaction(
        hexPreviousTransaction,
        isSegwitSupported
      );

      // output index
      const outputIndex = input.getPreviousOutputIndex();

      // sequence
      const sequence = input.getSequence();

      return [
        previousTransaction,
        outputIndex,
        // we don't use that
        // TODO: document
        null,
        sequence
      ];
    })
  );

  const associatedKeysets = rawInputs.map(input => {
    const derivationPaths = input.getDerivationPath();
    return derivationPaths[0].toString();
  });

  const outputs = transaction.getOutputs();
  const output = outputs.find((output, i) => {
    // FIXME: remove that when we get the fix (by khalil: "add method `isNull`
    // to check if C++ implementation is null")
    try {
      const derivationPath = output.getDerivationPath();
      const strDerivationPath = derivationPath.toString();
      const derivationArr = strDerivationPath.split("/");
      return derivationArr[derivationArr.length - 2] === "1";
    } catch (err) {
      return false;
    }
  });
  const changePath = output.getDerivationPath().toString();

  // TODO: serialize transaction here, and cut it to get outputScript
  const outputScriptHex = transaction.getHash();

  // TODO: detect it with address
  const segwit = false;

  const lockTime = transaction.getLockTime();
  // const initialTimestamp = transaction.getTimestamp();

  console.log(`INPUTS`);
  console.log(JSON.stringify(inputs, null, 2));
  console.log(``);

  console.log(`ASSOCIATEDKEYSETS`);
  console.log(associatedKeysets);
  console.log(``);

  console.log(`CHANGEPATH`);
  console.log(changePath);
  console.log(``);

  console.log(`LOCKTIME`);
  console.log(lockTime);
  console.log(``);

  console.log(`SEGWIT`);
  console.log(segwit);
  console.log(``);

  console.log(`OUTPUTSCRIPTHEX`);
  console.log(outputScriptHex);
  console.log(``);

  const something = await hwApp.createPaymentTransactionNew(
    inputs,
    associatedKeysets,
    changePath,
    outputScriptHex,
    lockTime
  );

  console.log(something);
}

module.exports = signTransaction;
