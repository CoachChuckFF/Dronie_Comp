import 'dart:async';

import 'package:doxadronie/Models/ez_connect_model.dart';
import 'package:esptouch_flutter/esptouch_flutter.dart';

// ------------------------- EZ Connect Code -----------------
// All the code needed to run this in @Full
//
// XCODE
// On Target > Runner > Signing & Capabilities, use the + Capability button to add the Access WiFi Information capability to your project.
//
// INFO.PLIST
// **** <project root>/ios/Runner/Info.plist ****
// <key>NSLocationAlwaysAndWhenInUseUsageDescription</key>
// <string>Location is ONLY used to grab your SSID name, nothing else</string>
// <key>NSLocationWhenInUseUsageDescription</key>
// <string>Location is ONLY used to grab your SSID name, nothing else</string>
//
// PUBSPEC YAML
// esptouch_flutter: ^0.2.4
// wifi_info_flutter: ^2.0.0
//
// IMPORTS
// import 'dart:io'; //Needed for Platform
// import 'package:esptouch_flutter/esptouch_flutter.dart';
// import 'package:wifi_info_flutter/wifi_info_flutter.dart';
//
// LINKS
// Getting SSID, BSSID
// https://pub.dev/packages/wifi_info_flutter
// https://stackoverflow.com/questions/55716751/flutter-ios-reading-wifi-name-using-the-connectivity-or-wifi-plugin/55732656#55732656
//
// ESP Touch
// https://pub.dev/packages/esptouch_flutter
//
//
//
//
// OPERATION
//
// On SoC-It start, if no SSID is found in storage - it will start EZ connect (Assuming SoC-It is in STA mode)
// SoC-It will now default to STA mode, therefore, all new SoC-Its will start in EZ connect
//
// When SoC-It is trying to EZ connect it will flash fast green, when you see this, runEZConnect()
//
// Once a connection is established EZ connect will disable - until the SSID stored in the SoC-It is cleared
//
// To re-establish a connection:
// NVS_CONFIG SSID = ""
// NVS_CONNECTION = External (STA)
// Reboot SoC-It
//
//
// Flutter Code
//
// BlizzardEZConnectManager _manager = BlizzardEZConnectManager({params});
//
// _manager.run(pass, {params});
// _manager.cancel();
// _manager.dispose();
//
//

class BlizzardEZConnectManager {
  final String _superSecretObscureSSIDString = "B";
  final String _superSecretObscurePASSString = "L";

  final Duration timeoutDuration;
  final Function(String, String) onStart;
  final Function onCancel;
  final Function onTimeout;
  final Function onSuccess;
  final Function(BlizzardEZConnectResult) onConnected;

  BlizzardEZConnectManager({
    required this.timeoutDuration,
    required this.onStart,
    required this.onCancel,
    required this.onTimeout,
    required this.onConnected,
    required this.onSuccess,
  });

  StreamSubscription<ESPTouchResult>? _ezConnectController;
  Timer? _ezConnectWatchdog;

  bool isRunning = false;
  int deviceCount = 1;
  int devicesLeft = 1;

  void cancel(){
    _stopEZConnect();
    onCancel();
  }

  void dispose(){
    _stopEZConnect();
  }

  Future<void> run(
    String pass,
    {
      String ssid = "", //await getCurrentSSID();
      String bssid = "", //await getCurrentBSSID();
      int deviceCount = 1,
  }) async {
    Stream<ESPTouchResult> stream;
    ESPTouchTask task;

    if(isRunning){
      print("EZ Connect Already Running");
      return;
    }
    
    // if(ssid.isEmpty)
    //   ssid = await getCurrentSSID();
    // }
    // if(bssid.isEmpty){
    //   bssid = await BlizzardEZConnectManager.getCurrentBSSID();
    // }

    this.deviceCount = deviceCount;
    this.devicesLeft = deviceCount;

    //config EZConnect
    task = ESPTouchTask(ssid: ssid + _superSecretObscureSSIDString, bssid: bssid, password: pass + _superSecretObscurePASSString);

    //start EZConnect
    stream = task.execute();

    //listen to EZConnect
    _ezConnectController = stream.listen((result){
      devicesLeft--;
      onConnected(BlizzardEZConnectResult.fromESP(result, deviceCount, devicesLeft));

      if(devicesLeft <= 0){
        onSuccess();
        _stopEZConnect();
      } else {
        _ezConnectWatchdog?.cancel();
        _ezConnectWatchdog = Timer(timeoutDuration, _setWatchdog);
      }
    });

    //set timeout EZConnect
    _ezConnectWatchdog = Timer(timeoutDuration, _setWatchdog);

    //finish starting the task
    onStart(ssid, bssid);

    isRunning = true;
  }

  void _setWatchdog(){
    _stopEZConnect();
    onTimeout();
  }

  void _stopEZConnect(){
    if(_ezConnectController != null){
      _ezConnectController?.cancel();
      _ezConnectController = null;
    }

    if(_ezConnectWatchdog != null){
      _ezConnectWatchdog?.cancel();
      _ezConnectWatchdog = null;
    }

    isRunning = false;
  }
}
