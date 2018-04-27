const {
  stringToBytesArray,
  bytesToHex,
  createBitcoinLikeHelper
} = require("./helpers");

async function signTransaction(hwApp, transaction) {
  const rawInputs = transaction.getInputs();

  // const s = transaction.serialize();
  // const bitcoinLikeHelper = createBitcoinLikeHelper();

  const inputs = await Promise.all(
    rawInputs.map(async input => {
      // previous transaction
      const rawPreviousTransaction = await input.getPreviousTransaction();
      const hexPreviousTransaction = bytesToHex(rawPreviousTransaction);
      const previousTransaction = hwApp.splitTransaction(
        hexPreviousTransaction
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
    // FIXME: remove that when we get the fix
    if (i === 0) {
      return false;
    }
    const derivationPath = output.getDerivationPath();
    const strDerivationPath = derivationPath.toString();
    const derivationArr = strDerivationPath.split("/");
    return derivationArr[derivationArr.length - 2] === "1";
  });
  const changePath = output.getDerivationPath().toString();

  // TODO: serialize transaction here, and cut it to get outputScript
  const outputScriptHex = "";

  // TODO: detect it with address
  const segwit = false;

  const lockTime = transaction.getLockTime();
  console.log(`before get timestamp`);
  const initialTimestamp = transaction.getTimestamp();
  console.log(`after get timestamp`);

  console.log(`INPUTS`);
  console.log(inputs);
  console.log(``);

  console.log(`ASSOCIATEDKEYSETS`);
  console.log(associatedKeysets);
  console.log(``);

  console.log(`CHANGEPATH`);
  console.log(changePath);

  console.log(`LOCKTIME`);
  console.log(lockTime);

  console.log(`SEGWIT`);
  console.log(segwit);

  console.log(`TIMESTAMP`);
  console.log(initialTimestamp);
}

module.exports = signTransaction;
