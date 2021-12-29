  
  import 'dart:math';

import 'package:doxadronie/Models/dronie_colors.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class DBox{
  static Widget ffBox({
    Color? color, 
    EdgeInsets? padding, 
    EdgeInsets? margin, 
    Widget? child, 
    double? width, 
    double? height, 
    double? boxSize, 
    double radius = 8, 
    bool hasShadow = true,
    bool isReverseShadow = false
  }){
    return Container(
      width: boxSize ?? width,
      height: boxSize ?? height,
      padding: padding ?? EdgeInsets.all(0),
      margin: margin ?? EdgeInsets.all(0),
      decoration: BoxDecoration(
        color: color ?? DColors.red,
        borderRadius: BorderRadius.circular(radius),
        boxShadow: (hasShadow) ? (isReverseShadow ? reverseShadow : shadow) : [],
      ),
      child: child ?? Container(),
    );
  }

  static buildFloatingButton({String? heroTag, String? fp, Function ?onTap, IconData? icon}){
    return FloatingActionButton(
      heroTag: heroTag,
      onPressed: (){
        if(onTap != null) onTap();
      },
      backgroundColor: DColors.darkGreen,
      splashColor: DColors.white.withAlpha(100),
      elevation: 8,
      // shape: RoundedRectangleBorder(
      //   borderRadius: BorderRadius.circular(8.0),
      // ),
      child: Container(
        decoration: BoxDecoration(
          shape: BoxShape.circle,
          image: DecorationImage(
            fit: BoxFit.fill,
            image: AssetImage('assets/images/dronie_icon.jpeg')
          )
        )
      ),
    );
  }

  static showBottomDrawer(
    BuildContext context, 
    {Widget ?child, String? heroTag, String? fp, bool isScrollControlled = true, Function? onComplete}){
    Size screenSize = MediaQuery.of(context).size;

    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.transparent,
      isScrollControlled: isScrollControlled,
      builder: (BuildContext context) {
        
        return Container(
          child: ffBox(
            height: screenSize.height * 0.55,
            color: DColors.black,
            child: Column(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Transform.translate(
                  offset: Offset(0, -20),
                  child: buildFloatingButton(
                    heroTag: heroTag ?? Random().nextDouble().toString(),
                    
                    onTap: (){
                      Navigator.of(context).pop();
                    }
                  ),
                ),
                (child != null) ? Expanded(child: child) : Container(),
              ]
            )
          ),
        );
      }
    )..whenComplete((){
      HapticFeedback.heavyImpact();
      if(onComplete != null) onComplete();
    });
    
  }

    static const Color _kKeyUmbraOpacity = Color(0x33000000); // alpha = 0.2
    static const Color _kKeyPenumbraOpacity = Color(0x24000000); // alpha = 0.14
    static const Color _kAmbientShadowOpacity = Color(0x1F000000); // alpha = 0.12
    static const List<BoxShadow> shadow = [
      BoxShadow(offset: Offset(0.0, 0.0), blurRadius: 0.0, spreadRadius: 0.0, color: _kKeyUmbraOpacity),
      // BoxShadow(offset: Offset(0.0, 3.0), blurRadius: 3.0, spreadRadius: -2.0, color: _kKeyUmbraOpacity),
      BoxShadow(offset: Offset(0.0, 3.0), blurRadius: 4.0, spreadRadius: 0.0, color: _kKeyPenumbraOpacity),
      BoxShadow(offset: Offset(0.0, 1.0), blurRadius: 8.0, spreadRadius: 0.0, color: _kAmbientShadowOpacity),
    ];

    static const List<BoxShadow> reverseShadow = [
      BoxShadow(offset: Offset(0.0, 0.0), blurRadius: 0.0, spreadRadius: 0.0, color: _kKeyUmbraOpacity),
      // BoxShadow(offset: Offset(0.0, 3.0), blurRadius: 3.0, spreadRadius: -2.0, color: _kKeyUmbraOpacity),
      BoxShadow(offset: Offset(0.0, -3.0), blurRadius: 4.0, spreadRadius: 0.0, color: _kKeyPenumbraOpacity),
      BoxShadow(offset: Offset(0.0, -1.0), blurRadius: 8.0, spreadRadius: 0.0, color: _kAmbientShadowOpacity),
    ];
  }
  
