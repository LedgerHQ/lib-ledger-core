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

const { CoreLGSecp256k1 } = NativeModules;
console.log("=====NativeModules")
console.log(NativeModules)
console.log("=====CoreLGSecp256k1")
console.log(CoreLGSecp256k1)

// const RCTCoreLGSecp256k1 = NativeModules.CoreLGSecp256k1()
// console.log("=====RCTCoreLGSecp256k1")
// console.log(RCTCoreLGSecp256k1)

async function getPublicKeyFromPrivKey (privKey, compressed) {
    try {
        const publicKey = await CoreLGSecp256k1.computePubKey(privKey,compressed)
        console.log("============Public Key")
        console.log(publicKey)

    } catch (e) {
        console.log("============Error")
        console.error(e);
    }
}

getPublicKeyFromPrivKey("12345", true)

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
