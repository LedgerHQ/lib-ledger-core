const { bytesToHex, stringToBytesArray, hexToBytes, segwitMode } = require('./helpers')

async function signTransaction(hwApp, transaction, isSegwitSupported = true) {
    const rawInputs = transaction.getInputs()

    const inputs = await Promise.all(
        rawInputs.map(async input => {
        // const rawPreviousTransactionHash = await input.getPreviousTxHash()
        const rawPreviousTransaction = await input.getPreviousTransaction()
        const hexPreviousTransaction = bytesToHex(rawPreviousTransaction)
        const previousTransaction = hwApp.splitTransaction(hexPreviousTransaction, isSegwitSupported)
        const outputIndex = input.getPreviousOutputIndex()
        const sequence = input.getSequence()
        return [
            previousTransaction,
            outputIndex,
            undefined, // we don't use that TODO: document
            sequence, // 0xffffffff,
        ]
    }),
)

    const associatedKeysets = rawInputs.map(input => {
        const derivationPaths = input.getDerivationPath()
        return derivationPaths[0].toString()
    })

    const outputs = transaction.getOutputs()

    const output = outputs.find((output, i) => {
        const derivationPath = output.getDerivationPath()
        if (derivationPath.isNull()) {
        return false
    }
    const strDerivationPath = derivationPath.toString()
    const derivationArr = strDerivationPath.split('/')
    return derivationArr[derivationArr.length - 2] === '1'
})

    const changePath = output ? output.getDerivationPath().toString() : undefined
    const outputScriptHex = bytesToHex(transaction.serializeOutputs())
    const lockTime = transaction.getLockTime()
    const initialTimestamp = transaction.getTimestamp()
    const SIGHASH_ALL = 1;

    const signedTransaction = await hwApp.createPaymentTransactionNew(
        inputs,
        associatedKeysets,
        changePath,
        outputScriptHex,
        lockTime,
        SIGHASH_ALL,
        segwitMode
    )

    console.log(signedTransaction)

    return hexToBytes(signedTransaction)
}

module.exports = signTransaction
