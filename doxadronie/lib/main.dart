import 'dart:io';
import 'dart:math';

import 'package:doxadronie/Controllers/artnet_server.dart';
import 'package:doxadronie/Models/dronie_colors.dart';
import 'package:doxadronie/Models/dronie_state.dart';
import 'package:doxadronie/Views/ast.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:provider/provider.dart';

import 'Models/artnet.dart';
import 'Models/mac.dart';

ArtnetServer server = ArtnetServer();

void main() {

  WidgetsFlutterBinding.ensureInitialized();
  SystemChrome.setPreferredOrientations([DeviceOrientation.portraitUp]);
  server.startServer();

  runApp(DroxADronie());
}

class DroxADronie extends StatefulWidget {
  DroxADronie({Key ?key }) : super(key: key);

  @override
  _DroxADronieState createState() => _DroxADronieState();
}

class _DroxADronieState extends State<DroxADronie> {
  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        StreamProvider<Map<Mac, ArtnetStateData>>.value(value: server.listenToServer(), initialData: {}),
      ],
      child: MaterialApp(
        title: 'Dox A Dronie',
        debugShowCheckedModeBanner: false,
        theme: ThemeData(
          primarySwatch: Colors.green,
        ),
        home: Scaffold(
          body: Container(
            color: DColors.black,
            child: Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  TestName(),
                  TestCount(),
                  Row(
                    children: [
                      ConnectButton(title: "Incog", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetMode(DronieMode.incognito));}),
                      ConnectButton(title: "Debug", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetMode(DronieMode.debug));}),
                      ConnectButton(title: "Spoon", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetMode(DronieMode.spoon));}),
                    ],
                  ),
                  Row(
                    children: [
                      ConnectButton(title: "BLK", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetLed(Colors.black, 0xFF));}),
                      ConnectButton(title: "WHT", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetLed(Colors.white, 0xFF));}),
                      ConnectButton(title: "RNG", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetLed(Color(0xFF000000 | Random().nextInt(0xFFFFFFFF)), 0xFF));}),
                    ],
                  ),
                  Row(
                    children: [
                      ConnectButton(title: "Dot", onTap: (){server.sendToAllDevices(DronieCommandPacket.createDot());}),
                      ConnectButton(title: "Dash", onTap: (){server.sendToAllDevices(DronieCommandPacket.createDash());}),
                      ConnectButton(title: "Motor", onTap: (){server.sendToAllDevices(DronieCommandPacket.createTouchMotor());}),
                    ],
                  ),
                  Row(
                    children: [
                      ConnectButton(title: "OWN", onTap: (){server.sendToAllDevices(DronieCommandPacket.createSetOwner("Coach Chuck"));}),
                      ConnectButton(title: "RST", onTap: (){server.sendToAllDevices(DronieCommandPacket.createFactoryReset());}),
                      ConnectButton(title: "CON", onTap: (){server.connectNodes("destroyer");}),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class AnimationScreen extends StatefulWidget {
  AnimationScreen({Key? key}) : super(key: key);

  @override
  _AnimationScreenState createState() => _AnimationScreenState();
}

class _AnimationScreenState extends State<AnimationScreen> {
  @override
  Widget build(BuildContext context) {
    return Container(
    );
  }
}

class ConnectButton extends StatelessWidget {
  final void Function()? onTap;
  final String title;
  
  const ConnectButton({Key? key, this.onTap, required this.title}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: onTap,
      child: Container(
        width: 100,
        height: 100,
        color: DColors.darkGreen,
        child: Container(
          padding: EdgeInsets.all(5),
          child: Center(
            child: AST(
              title,
            ),
          ),
        ),
      ),
    );
  }
}

class TestName extends StatelessWidget {
  const TestName({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Map<Mac, ArtnetStateData> devices = Provider.of<Map<Mac, ArtnetStateData>>(context);

    String title = "NA";
    if(devices.isNotEmpty){
      title = "${devices.values.first.state.owner} (${devices.values.first.strength})";
    }

    return Container(
      height: 200,
      child: Center(
         child: AST(
           title,
           color: DColors.white,
           size: 55,
         ),
      ),
    );
  }
}

class TestCount extends StatelessWidget {
  const TestCount({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    Map<Mac, ArtnetStateData> devices = Provider.of<Map<Mac, ArtnetStateData>>(context);

    return Container(
      height: 200,
      child: Center(
         child: AST(
           devices.length.toString(),
           color: DColors.white,
           size: 55,
         ),
      ),
    );
  }
}
