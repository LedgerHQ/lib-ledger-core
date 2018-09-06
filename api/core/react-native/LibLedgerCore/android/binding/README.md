
# react-native-ledger-core

## Getting started

`$ npm install react-native-ledger-core --save`

### Mostly automatic installation

`$ react-native link react-native-ledger-core`

### Manual installation


#### Android

1. Open up `android/app/src/main/java/[...]/MainActivity.java`
  - Add `import com.ledger.reactnative.RNLedgerCorePackage;` to the imports at the top of the file
  - Add `new RNLedgerCorePackage()` to the list returned by the `getPackages()` method
2. Append the following lines to `android/settings.gradle`:
  	```
  	include ':react-native-ledger-core'
  	project(':react-native-ledger-core').projectDir = new File(rootProject.projectDir, 	'../node_modules/react-native-ledger-core/android')
  	```
3. Insert the following lines inside the dependencies block in `android/app/build.gradle`:
  	```
      compile project(':react-native-ledger-core')
  	```


## Usage
```javascript
import RNLedgerCore from 'react-native-ledger-core';

// TODO: What to do with the module?
RNLedgerCore;
```
  