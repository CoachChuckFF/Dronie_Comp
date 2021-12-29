
import 'package:esptouch_flutter/esptouch_flutter.dart';

class BlizzardEZConnectResult {
  final String ip;
  final String bssid;
  final int devicesLeft;
  final int totalDevices;

  BlizzardEZConnectResult({
    required this.ip,
    required this.bssid,
    required this.totalDevices,
    required this.devicesLeft,
  });

  int get deviceIndex {
    return totalDevices - devicesLeft - 1;
  }

  int get deviceNumber {
    return totalDevices - devicesLeft;
  }

  factory BlizzardEZConnectResult.fromESP(ESPTouchResult data, int totalDevices, int devicesLeft){
    return BlizzardEZConnectResult(
      ip: data.ip,
      bssid: data.bssid,
      totalDevices: totalDevices,
      devicesLeft: devicesLeft,
    );
  }

  @override
  String toString() {
    String string = "";

    string += "\n---------- EZ Connect -----------\n";
    string += "Status: ${this.ip.isNotEmpty ? 'SUCCESS' : 'FAIL'}\n";
    string += "IP: ${this.ip}\n";
    string += "BSSID: ${this.bssid}\n";
    string += "Devices Left: ${this.bssid}\n";
    string += "---------------------------------\n\n";

    return string;
  }

}