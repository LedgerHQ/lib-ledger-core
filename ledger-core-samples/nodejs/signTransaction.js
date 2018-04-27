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
  console.log(outputs[0].getDerivationPath().getDepth());
  console.log(outputs[1].getDerivationPath().getDepth());
  const changePath = outputs.find(output => {
    const derivationPath = output.getDerivationPath();
    console.log(derivationPath);
    const strDerivationPath = derivationPath.toString();
  });
  console.log(outputs);

  console.log(`INPUTS`);
  console.log(transformedInputs);
  console.log(``);

  console.log(`ASSOCIATEDKEYSETS`);
  console.log(associatedKeysets);
  console.log(``);
}

module.exports = signTransaction;
