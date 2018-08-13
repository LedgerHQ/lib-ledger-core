/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 * @flow
 */

import React, { Component } from 'react';
import {
  Platform,
  StyleSheet,
  Text,
  View,
  NativeModules
} from 'react-native';

const { CoreLGSecp256k1,
        CoreLGHttpClient,
        CoreLGWebSocketClient,
        CoreLGPathResolver,
        CoreLGLogPrinter,
        CoreLGThreadDispatcher,
        CoreLGRandomNumberGenerator,
        CoreLGDatabaseBackend,
        CoreLGDynamicObject,
        CoreLGWalletPool
      } = NativeModules;
      
console.log("=====NativeModules")
console.log(NativeModules)

async function createWalletPoolInstance () {
  const httpClient = await CoreLGHttpClient.new();
  const webSocket = await CoreLGWebSocketClient.new();
  const pathResolver = await CoreLGPathResolver.new();
  const logPrinter = await CoreLGLogPrinter.new();
  const threadDispatcher = await CoreLGThreadDispatcher.new();
  const rng = await CoreLGRandomNumberGenerator.new();
  const backend = await CoreLGDatabaseBackend.getSqlite3Backend();
  const dynamicObject = await CoreLGDynamicObject.newInstance();
  const walletPoolInstance = await CoreLGWalletPool.newInstance('ledger_live_desktop',
                                                                '',
                                                                httpClient,
                                                                webSocket,
                                                                pathResolver,
                                                                logPrinter,
                                                                threadDispatcher,
                                                                rng,
                                                                backend,
                                                                dynamicObject);
    console.log(" >>> walletPoolInstance");
    console.log(walletPoolInstance);
    return walletPoolInstance;

}
const walletPool = createWalletPoolInstance();
async function getPublicKeyFromPrivKey (privKey, compressed) {
    try {
        const secp256k1 = await CoreLGSecp256k1.createInstance();
        console.log("============Secp256k1 instance")
        console.log(secp256k1)
        const publicKey = await CoreLGSecp256k1.computePubKey(secp256k1,privKey,compressed)
        console.log("============Public Key")
        console.log(publicKey)
        //console.log(Buffer.from(publicKey, 'hex'))

    } catch (e) {
        console.log("============Error")
        console.error(e);
    }
}

//getPublicKeyFromPrivKey("12345", true)

const instructions = Platform.select({
  ios: 'Press Cmd+R to reload,\n' +
    'Cmd+D or shake for dev menu',
  android: 'Double tap R on your keyboard to reload,\n' +
    'Shake or press menu button for dev menu',
});

type Props = {};
export default class App extends Component<Props> {
  render() {
    return (
      <View style={styles.container}>
        <Text style={styles.welcome}>
          Welcome to React Native!
        </Text>
        <Text style={styles.instructions}>
          To get started, edit App.js
        </Text>
        <Text style={styles.instructions}>
          {instructions}
        </Text>
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  welcome: {
    fontSize: 20,
    textAlign: 'center',
    margin: 10,
  },
  instructions: {
    textAlign: 'center',
    color: '#333333',
    marginBottom: 5,
  },
});
