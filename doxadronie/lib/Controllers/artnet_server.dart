/*
* Blizzard Pro. LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 7/26/19
*
*/
//Extras
import 'dart:async';
import 'dart:io';

import 'package:wifi_info_flutter/wifi_info_flutter.dart';
import 'package:doxadronie/Controllers/ez_connect_manager.dart';
import 'package:doxadronie/Models/artnet.dart';
import 'package:doxadronie/Models/dronie_state.dart';
import 'package:doxadronie/Models/mac.dart';

class ArtnetServerDefines{
  static const int success = 0;
  static const int getIpFailure = 1;
  static const int startSocketFailure = 2;
  static const int alreadyStarted = 3;
}

class ArtnetStateData {
  final Mac mac;

  final InternetAddress ip;
  final int strength;
  final DronieState state;

  ArtnetStateData({
    required this.mac,
    required this.ip,
    required this.strength,
    required this.state,
  });

  factory ArtnetStateData.fromState(
    ArtnetStateData state, {InternetAddress? withIp, int? withStrength, DronieState? withData}
  ){
    return ArtnetStateData(
      mac: state.mac,
      ip: withIp ?? state.ip,
      strength: withStrength ?? state.strength,
      state: withData ?? state.state
    );
  }
}

class ArtnetServer{
  static final ArtnetServer _artnetServer = ArtnetServer._internal();

  WifiInfo wifiInfo = WifiInfo();
  InternetAddress ip = InternetAddress.anyIPv4;
  String ssid = "";
  String bssid = "";
  Map<Mac, ArtnetStateData> devices = Map<Mac, ArtnetStateData>();
  ArtnetPollPacket _poll = ArtnetPollPacket();
  RawDatagramSocket? _socket;

  bool _connecting = false;

  bool _running = false;
  StreamSubscription<int>? _server;

  static const int pollIntervalMS = 5000;
  static const int lives = 10;

  factory ArtnetServer() {
    return _artnetServer;
  }
  ArtnetServer._internal();


  void startServer() async {

    stopServer();


    // _updateIP();

    //Bind the _socket
    await RawDatagramSocket.bind(
      InternetAddress.anyIPv4,
      ArtnetHelpers.port
    ).then((RawDatagramSocket freshSocket) {
      _socket = freshSocket;
      _socket!.broadcastEnabled = true;
      _socket!.listen(
        _handlePacket,
        onError: (_){
          print("Internal _socket error");
          _running = false;
        },
        onDone: () {
          stopServer();
        },
        cancelOnError: false
      );

      print("running");
      _running = true;
      _server = _runServer().listen((event) {});
    }).catchError((error) {
      stopServer();
    });

  }

  void stopServer(){
    _running = false;
    if(_server != null){
      _server!.cancel().then((_){
        _socket!.close();
      });
    }
  }

  void clearDronies(){
    devices = {};
  }

  void connectNodes(String password, {Function? onSuccess, Function? onTimeout}){
    if(ssid.isEmpty){print("Bad SSID"); return;}
    if(_connecting){print("Already Connecing"); return;}
    print(ssid);
    print(password);
    BlizzardEZConnectManager manager = BlizzardEZConnectManager(
      onCancel: (){print("[EZ] Cancelled");},
      onStart: (ssid, pass){print("[EZ] Started");},
      onConnected: (results){print("[EZ] Connected ($results)");},
      onSuccess: (){print("[EZ] Success"); _connecting = false; if(onSuccess != null) onSuccess();},
      onTimeout: (){print("[EZ] Timeout"); _connecting = false; if(onTimeout != null) onTimeout();},
      timeoutDuration: Duration(seconds: 55),
    );

    _connecting = true;
    manager.run(password, ssid: ssid, bssid: bssid);
  }

  void _handlePacket(_){
    Datagram ?gram = _socket?.receive();

    if(gram == null) return;
    // if(ownIP == gram.address) return;
    // if(ownIP == InternetAddress.anyIPv4) return;

    if(!ArtnetHelpers.artnetCheckPacket(gram.data)) return;

    if(ArtnetHelpers.artnetGetOpCode(gram.data) == ArtnetPollReplyPacket.opCode){
      ArtnetPollReplyPacket reply = ArtnetPollReplyPacket(gram.data);
      DronieState state = DronieState.fromPoll(Mac(reply.mac), ArtnetPollReplyPacket(gram.data));
      Mac mac = Mac(reply.mac);

      devices[Mac(reply.mac)] = ArtnetStateData(mac: mac, ip: gram.address, strength: lives, state: state);
    } 

  }

  void sendToAllDevices(ArtnetPacket packet){
    for(ArtnetStateData state in devices.values){
      sendPacket(packet.udpPacket, state.ip);
    }
  }

  void sendBurst(
    int count,
    List<int> packet,
    InternetAddress destIP, 
    {
      String description = "N/A"
    }
  ){
    for (var i = 0; i < count; i++) {
      sendPacket(packet, destIP, description: description);
    }
  }

  void sendPacket(
    List<int> packet, 
    InternetAddress destIP, 
    {
      String description = "N/A"
    }
  ){
    if(_running) _socket!.send(packet, destIP, ArtnetHelpers.port);
  }

  Stream<int> _runServer() async* {
    int ticks = 0; //ticks
    while (true) {

      List<Mac> macs = List<Mac>.from(devices.keys);

      for(Mac key in macs){
        int strength = devices[key]!.strength;
        if(strength <= 0){
          devices.remove(key);
        } else {
          devices[key] = ArtnetStateData.fromState(devices[key]!, withStrength: strength - 1);
        }
      }

      _updateIP();
      sendPacket(_poll.udpPacket, InternetAddress(ArtnetHelpers.broadcast), description: "Poll");

      yield ticks++;
      await Future.delayed(Duration(milliseconds: pollIntervalMS));
    }
  }

  Stream<Map<Mac, ArtnetStateData>> listenToServer() async* {
    while (true) {
      await Future.delayed(Duration(milliseconds: 100));

      yield Map.from(devices);
    }
  }

  void _updateIP() async{

    await wifiInfo.requestLocationServiceAuthorization();
    ssid = await wifiInfo.getWifiName() ?? "";
    bssid = await wifiInfo.getWifiBSSID() ?? "";
    if(ssid.isNotEmpty){
      ip = InternetAddress(await wifiInfo.getWifiIP() ?? "");
    } else {
      ip = InternetAddress.anyIPv4;
    }
  }
}


