
# ledger-core-lib-ledger-core

## Getting started

`$ npm install ledger-core-lib-ledger-core --save`

### Mostly automatic installation

`$ react-native link ledger-core-lib-ledger-core`

### Manual installation


#### iOS

1. In XCode, in the project navigator, right click `Libraries` ➜ `Add Files to [your project's name]`
2. Go to `node_modules` ➜ `ledger-core-lib-ledger-core` and add `RCTCoreLibLedgerCore.xcodeproj`
3. In XCode, in the project navigator, select your project. Add `libRCTCoreLibLedgerCore.a` to your project's `Build Phases` ➜ `Link Binary With Libraries`
4. Run your project (`Cmd+R`)<

#### Android

1. Open up `android/app/src/main/java/[...]/MainActivity.java`
  - Add `import com.ledgercore.RCTCoreLibLedgerCorePackage;` to the imports at the top of the file
  - Add `new RCTCoreLibLedgerCorePackage()` to the list returned by the `getPackages()` method
2. Append the following lines to `android/settings.gradle`:
  	```
  	include ':ledger-core-lib-ledger-core'
  	project(':ledger-core-lib-ledger-core').projectDir = new File(rootProject.projectDir, 	'../node_modules/ledger-core-lib-ledger-core/android')
  	```
3. Insert the following lines inside the dependencies block in `android/app/build.gradle`:
  	```
      compile project(':ledger-core-lib-ledger-core')
  	```

#### Windows
[Read it! :D](https://github.com/ReactWindows/react-native)

1. In Visual Studio add the `RCTCoreLibLedgerCore.sln` in `node_modules/ledger-core-lib-ledger-core/windows/RCTCoreLibLedgerCore.sln` folder to their solution, reference from their app.
2. Open up your `MainPage.cs` app
  - Add `using Lib.Ledger.Core.RCTCoreLibLedgerCore;` to the usings at the top of the file
  - Add `new RCTCoreLibLedgerCorePackage()` to the `List<IReactPackage>` returned by the `Packages` method


## Usage
```javascript
import RCTCoreLibLedgerCore from 'ledger-core-lib-ledger-core';

// TODO: What to do with the module?
RCTCoreLibLedgerCore;
```
  