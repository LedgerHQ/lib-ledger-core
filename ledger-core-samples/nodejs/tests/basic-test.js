/* eslint-disable no-console */
const {
    createWallet,
    getCurrency,
    getWallet,
} = require('../index.js')

const { signTransaction } = require('../signTransaction')
async function getOrCreateWallet(currencyId) {
    try {
        const wallet = await getWallet(currencyId)
        return wallet
    } catch (err) {
        const currency = await getCurrency(currencyId)
        const wallet = await createWallet(currencyId, currency)
        return wallet
    }
}
console.log(" >>> Get bitcoin wallet")
const wallet = getOrCreateWallet("bitcoin");
console.log(" >>> Got bitcoin wallet")
