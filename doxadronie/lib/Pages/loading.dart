
import 'package:doxadronie/Models/dronie_colors.dart';
import 'package:doxadronie/Models/roads.dart';
import 'package:doxadronie/Views/dronie_logo.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';

class LoadingPage extends StatefulWidget {
  LoadingPage({Key? key}) : super(key: key);

  @override
  _LoadingPageState createState() => _LoadingPageState();
}

class _LoadingPageState extends State<LoadingPage> {
  bool _visable = false;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance?.addPostFrameCallback((timeStamp) {_getGoing();});
  }

  void _getGoing(){
    Future.delayed(Duration(milliseconds: 300), (){
      setState(() {
        _visable = true;
      });
      Future.delayed(Duration(milliseconds: 500), (){
        Navigator.of(context).pushNamed(Roads.main);
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Container(
        color: DColors.black,
        child: Container(
          child: Column(
            children: [
              Expanded(child: Container()),
              AnimatedOpacity(
                opacity: _visable ? 1.0 : 0.0,
                duration: const Duration(milliseconds: 1000),
                child: DronieIcon(),
              ),
              Expanded(child: Container())
            ]
          ),
        ),
      ),
    );
  }
}