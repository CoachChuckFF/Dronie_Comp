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
import 'Models/roads.dart';

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
        initialRoute: Roads.loading,
        onGenerateRoute: (RouteSettings settings) => Roads.getRoutes(settings),
      ),
    );
  }
}
