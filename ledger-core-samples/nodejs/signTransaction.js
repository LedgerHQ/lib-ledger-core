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
  const outputScriptHex = transaction.serializeOutputs();

  console.log(`outputScriptHex:`);
  console.log(`[${outputScriptHex}]`);

  // TODO: detect it with address
  const segwit = false;

  const lockTime = transaction.getLockTime();
  const initialTimestamp = transaction.getTimestamp();

  return;
  const something = await hwApp.createPaymentTransactionNew(
    inputs,
    associatedKeysets,
    changePath,
    outputScriptHex,
    lockTime
  );
}

module.exports = signTransaction;
