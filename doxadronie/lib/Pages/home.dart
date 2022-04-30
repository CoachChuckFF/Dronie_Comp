
import 'dart:async';
import 'dart:math';
import 'dart:ui';

import 'package:doxadronie/Controllers/artnet_server.dart';
import 'package:doxadronie/Controllers/bloc_controller.dart';
import 'package:doxadronie/Controllers/password_controller.dart';
import 'package:doxadronie/Models/artnet.dart';
import 'package:doxadronie/Models/dronie_colors.dart';
import 'package:doxadronie/Models/dronie_state.dart';
import 'package:doxadronie/Models/mac.dart';
import 'package:doxadronie/Views/ast.dart';
import 'package:doxadronie/Views/box.dart';
import 'package:doxadronie/Views/dronie_logo.dart';
import 'package:doxadronie/Views/password_dialog.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:provider/provider.dart';

class HomePage extends StatefulWidget {
  HomePage({Key? key}) : super(key: key);

  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage>  with TickerProviderStateMixin {
  ArtnetServer _server = ArtnetServer();
  AnimationController? _circleController;
  DoubleBLoC _circleUpdate = DoubleBLoC();

  IntBLoC _directionsBloc = IntBLoC();
  IntBLoC _searchingBloc = IntBLoC();
  BoolBLoC _startSearchingBloc = BoolBLoC();

  BoolBLoC _messageBloc = BoolBLoC();
  String _message = "";
  
  Timer? _timer;

  bool _hadDronies = false;

  @override
  void initState() {
    _setupAnimation();
    WidgetsBinding.instance?.addPostFrameCallback((timeStamp) {
      Future.delayed(Duration(milliseconds: 1300), (){
        _circleController!.forward();
        _startSearchingBloc.add(BoolUpdateEvent(true));
      });
    });
    super.initState();
  }

  @override
  void dispose() { 
    _searchingBloc.dispose();
    _startSearchingBloc.dispose();
    _messageBloc.dispose();
    _circleUpdate.dispose();
    _circleController?.dispose();
    super.dispose();
  }

  void _setupAnimation(){
    _circleController = AnimationController(
      vsync: this, 
      duration: Duration(milliseconds: 1300),
    )..addStatusListener((status) { 
      switch(status){
        case AnimationStatus.dismissed: 
          _circleController?.forward();
          break;
        case AnimationStatus.completed:
          _circleController?.reset();
          _circleController?.forward();
          _directionsBloc.add(IntUpdateEvent(_directionsBloc.state + 1));
          break;
        case AnimationStatus.forward: break;
        case AnimationStatus.reverse: break;
      }
    })..addListener(() {
      _circleUpdate.add(DoubleUpdateEvent(_circleController!.value));
      _searchingBloc.add(IntUpdateEvent(lerpDouble(0, 4, _circleController!.value)!.round()));
    });    
  }

  void _setMessage(String message){
    _message = message;
    _messageBloc.add(BoolUpdateEvent(true));

    _timer?.cancel();
    _timer = Timer(Duration(seconds: 3), (){
      _messageBloc.add(BoolUpdateEvent(false));
    });
    
  }

  Widget _buildMessage(){
    double height = MediaQuery.of(context).size.height * 0.05;
    return SafeArea(
      top: false,
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 10),
        height: height,
        child: BlocBuilder(
          bloc: _messageBloc,
          builder: (context, bool visable) {
            return AnimatedCrossFade(
              crossFadeState: (visable) ? CrossFadeState.showSecond : CrossFadeState.showFirst,
              duration: Duration(milliseconds: 200),
              firstChild: Container(
                height: height,
                child: Center(
                  child: BlocBuilder(
                    bloc: _startSearchingBloc,
                    builder: (context, bool isSearching) {
                      return BlocBuilder(
                        bloc: _directionsBloc,
                        builder: (context, int directionsCount) {
                          return BlocBuilder(
                            bloc: _searchingBloc,
                            builder: (context, int dots) {
                              String searching = "Searching";
                              for(int i = 0; i < dots % 4; i++) searching+=".";
                      
                              directionsCount %= 8;
                              if(directionsCount == 0 && _server.devices.isEmpty){
                                searching = "Tap Emblem to...";
                              }
                              if(directionsCount == 1 && _server.devices.isEmpty){
                                searching = "Execute Capture Protocol";
                              }

                              return AST(
                                isSearching ? searching.padRight(13) : "",
                                color: DColors.white,
                              );
                            }
                          );
                        }
                      );
                    }
                  ),
                ),
              ),
              secondChild: Container(
                height: height,
                child: Center(
                  child: AST(
                    _message,
                    color: DColors.white,
                  ),
                ),
              ),
            );
          }
        ),
      ),
    );
  }

  Widget _buildDronieCard(ArtnetStateData data){
    return GestureDetector(
      onTap: (){
        HapticFeedback.heavyImpact();
        DBox.showBottomDrawer(
          context,
          child: DronieDrawer(data.mac),
          heroTag: data.state.sn,
          fp: 'assets/images/dronie_icon.jpeg',
          onComplete: (){
            _setMessage("Dronie ${data.state.sn.substring(0, 8)}... Configured");
          }
        );
      },
      onLongPress: (){
        HapticFeedback.heavyImpact();
        _setMessage(data.state.sn);
      },
      child: Container(
        color: Colors.transparent,
        margin: EdgeInsets.all(8),
        padding: EdgeInsets.all(8),
        child: LayoutBuilder(
          builder: (context, size) {
            return Column(
              crossAxisAlignment: CrossAxisAlignment.center,
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Container(
                  width: size.maxWidth * 0.64,
                  height: size.maxWidth * 0.64,
                  decoration: BoxDecoration(
                    shape: BoxShape.circle,
                    boxShadow: DBox.shadow,
                    image: new DecorationImage(
                      fit: BoxFit.fill,
                      image: new AssetImage('assets/images/dronie_icon.jpeg')
                    )
                  )
                ),
                Container(height: 8,),
                AST(
                  data.state.sn.substring(0, 8),
                  color: DColors.white,
                )
              ],
            );
          }
        )
      ),
    );
  }

  Widget _buildDronieGrid(){
    Map<Mac, ArtnetStateData> devices = Provider.of<Map<Mac, ArtnetStateData>>(context);

    if(!_hadDronies){
      if(devices.isNotEmpty){
        _hadDronies = true;
        Future.delayed(Duration(milliseconds: 10), (){_setMessage("Found a Dronie");});
      }
    }

    return Container(
      child: GridView.builder(
        padding: EdgeInsets.zero,
        gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
          crossAxisCount: 2,
          crossAxisSpacing: 10
        ),
        itemCount: devices.length,
        itemBuilder: (BuildContext ctx, index) {
          return _buildDronieCard(devices.values.toList()[index]);
        }
      ),
    );
  }

  void _connect(String password) {
    _setMessage("Executing Dronie Capture Protocol.");
    _server.connectNodes(
      password,
      onSuccess: (){
        _setMessage("Captured a Dronie");
        HapticFeedback.heavyImpact();
        PasswordController.setPassword(password);
      },
      onTimeout: (){
        _setMessage("Capture Protocol Timeout.");
        HapticFeedback.lightImpact();
        PasswordController.clearPassword();
      },
    );
  }

  void _searchOrSet() {
    HapticFeedback.heavyImpact();
    PasswordController.getPassword().then((pass){
      if(pass.isEmpty){
        PasswordPopup.getPassword(
          context,
          (String password) async {
            _connect(password);
          }
        );
      } else {
        _connect(pass);
      }
    });
  }

  void _clearPassword() {
    PasswordController.clearPassword();
    _setMessage("WiFi Password Cleared.");
  }

  Widget _buildLogo(){
    double width = MediaQuery.of(context).size.width;
    double bigCircle = width * 0.89;

    return GestureDetector(
      onTap: _searchOrSet,
      onDoubleTap: _clearPassword,
      child: Container(
        height: bigCircle,
        child: Stack(
          children: [
            Center(
              child: Container(
                child: BlocBuilder(
                  bloc: _circleUpdate,
                  builder: (context, double value) {
                    value = Curves.decelerate.transform(value);
                    return Container(
                      width: bigCircle * value,
                      height: bigCircle * value,
                      decoration: BoxDecoration(
                        color: DColors.darkGreen.withOpacity(1.0 - value),
                        shape: BoxShape.circle
                      ),
                    );
                  }
                ),
              ),
            ),
            Center(child: DronieIcon()),
          ]
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      resizeToAvoidBottomInset: false,
      body: Container(
        color: DColors.black,
        child: Column(
          children: [
            Expanded(child: Container(),),
            _buildLogo(),
            Expanded(flex: 3, child: _buildDronieGrid(),),
            _buildMessage(),
          ]
        ),
      ),
    );
  }
}

class DronieDrawer extends StatefulWidget {
  final Mac mac;
  DronieDrawer(this.mac, {Key? key}) : super(key: key);

  @override
  _DronieDrawerState createState() => _DronieDrawerState();
}

class _DronieDrawerState extends State<DronieDrawer> {
  ArtnetServer _server = ArtnetServer();
  int _colorTick = 0;

  static List<Color> _colorList = [
    Colors.red,
    Colors.orange,
    Colors.yellow,
    Colors.green,
    Colors.blue,
    Colors.purple,
    Colors.white,
  ];

  Widget _infoLine(IconData icon, String text){
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 13, vertical: 8),
      child: Row(
        children: [
          Icon(
            icon,
            color: DColors.brightGreen,
            size: 34,
          ),
          Container(width: 5,),
          Expanded(
            child: AST(
              text,
              maxLines: 2,
              color: DColors.white,
              textAlign: TextAlign.right,
            ),
          )
        ],
      ),
    );
  }

  Widget _buildButton({required Color active, required Color disabled, required Color iconColor, required Color disabledIconColor, required bool enabled, required IconData icon, String? message, required Function onTap}){
    return GestureDetector(
      onTap: (){
        
        HapticFeedback.heavyImpact();
        onTap();
      },
      child: Container(
        color: Colors.transparent,
        margin: EdgeInsets.all(13),
        child: DBox.ffBox(
          padding: EdgeInsets.all(5),
          color: (enabled) ? active : disabled,
          child: Center(
            
            child: (message != null) ? 
            Row(
              children: [
                Icon(
                  icon,
                  color: (enabled) ? iconColor : disabledIconColor,
                  size: 34,
                ),
                Container(width: 10),
                Expanded(
                  child: AST(
                    message,
                    textAlign: TextAlign.center,
                    color: (enabled) ? active : disabled,
                  ),
                ),
              ],
            ) :
            Icon(
              icon,
              color: (enabled) ? iconColor : disabledIconColor,
              size: 34,
            ),
          )
        ),
      ),
    );
  }


  Widget _buildModes(ArtnetStateData data){
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 13, vertical: 8),
      child: Row(
        children: [
          Expanded(
            child: _buildButton(
              active: DColors.brightGreen,
              disabled: DColors.darkGreen,
              iconColor: DColors.darkGreen,
              disabledIconColor: DColors.black,
              enabled: data.state.mode == DronieMode.incognito,
              icon: Icons.airplanemode_active,
              onTap: (){
                _server.sendPacket(
                  DronieCommandPacket.createSetMode(DronieMode.incognito).udpPacket, 
                  data.ip
                );
              }
            ),
          ),
          Expanded(
            child: _buildButton(
              active: DColors.brightGreen,
              disabled: DColors.darkGreen,
              iconColor: DColors.darkGreen,
              disabledIconColor: DColors.black,
              enabled: data.state.mode == DronieMode.debug,
              icon: Icons.bug_report,
              onTap: (){
                _server.sendPacket(
                  DronieCommandPacket.createSetMode(DronieMode.debug).udpPacket, 
                  data.ip
                );
              }
            )
          ),
          Expanded(
            child: _buildButton(
              active: DColors.brightGreen,
              disabled: DColors.darkGreen,
              iconColor: DColors.darkGreen,
              disabledIconColor: DColors.black,
              enabled: data.state.mode == DronieMode.spoon,
              icon: Icons.audiotrack,
              onTap: (){
                _server.sendPacket(
                  DronieCommandPacket.createSetMode(DronieMode.spoon).udpPacket, 
                  data.ip
                );
              }
            )
          ),
        ],
      ),
    );
  }

  Widget _buildActions(ArtnetStateData data){
    return Container(
      padding: EdgeInsets.symmetric(horizontal: 13, vertical: 8),
      child: Row(
        children: [
          Expanded(
            child: Container(
              color: Colors.transparent,
              child: _buildButton(
                active: DColors.brightGreen,
                disabled: DColors.darkGreen,
                iconColor: DColors.darkGreen,
                disabledIconColor: DColors.black,
                enabled: data.state.mode == DronieMode.debug,
                icon: Icons.settings,
                onTap: (){
                  if(data.state.mode == DronieMode.debug){
                    HapticFeedback.heavyImpact();
                    _server.sendPacket(
                      DronieCommandPacket.createTouchMotor().udpPacket, 
                      data.ip
                    );
                  }
                }
              ),
            ),
          ),
          Expanded(
            child: _buildButton(
              active: DColors.brightGreen,
              disabled: DColors.darkGreen,
              iconColor: DColors.darkGreen,
              disabledIconColor: DColors.black,
              enabled: data.state.mode == DronieMode.debug,
              icon: Icons.color_lens,
              onTap: (){
                if(data.state.mode == DronieMode.debug){
                  HapticFeedback.heavyImpact();
                  _server.sendPacket(
                    DronieCommandPacket.createSetLed(_colorList[_colorTick++ % _colorList.length], 0xFF).udpPacket, 
                    data.ip
                  );
                }
              }
            )
          ),
          Expanded(
            child: _buildButton(
              active: DColors.red,
              disabled: DColors.darkGreen,
              iconColor: DColors.darkGreen,
              disabledIconColor: DColors.black,
              enabled: data.state.mode == DronieMode.debug,
              icon: Icons.clear_rounded,
              onTap: (){
                if(data.state.mode == DronieMode.debug){
                  HapticFeedback.heavyImpact();
                  FactoryResetPopup.factoryReset(
                    context, 
                    () async {
                      _server.sendPacket(
                        DronieCommandPacket.createFactoryReset().udpPacket, 
                        data.ip
                      );
                      Future.delayed(Duration(milliseconds: 89), (){
                        _server.clearDronies();
                        Navigator.pop(context);
                      });
                      HapticFeedback.heavyImpact();
                    }
                  );
                }
              }
            )
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    Map<Mac, ArtnetStateData> devices = Provider.of<Map<Mac, ArtnetStateData>>(context);
    ArtnetStateData ?data = devices[widget.mac];

    if(data == null){
      return Container(
        child: Center(
          child: AST(
            "The Dronie has Disconnected",
            color: DColors.white,
          ),
        ),
      );
    }

    String mode = "";
    switch(data.state.mode){
      case DronieMode.incognito: mode = "Incognito"; break;
      case DronieMode.debug: mode = "Debug"; break;
      case DronieMode.spoon: mode = "???"; break;
    }

    return Container(
      child: LayoutBuilder(
        builder: (context, size) {
          return SafeArea(
            top: false,
            child: Container(
              height: size.maxHeight,
              child: ListView(
                padding: EdgeInsets.zero,
                children: [
                  _infoLine(Icons.assignment_ind, "${data.state.sn}"),
                  _infoLine(Icons.business, "${data.state.man}"),
                  _infoLine(Icons.info, "$mode Mode"),
                  _buildModes(data),
                  _buildActions(data),
                ]
              ),
            ),
          );
        }
      ),
    );
  }
}