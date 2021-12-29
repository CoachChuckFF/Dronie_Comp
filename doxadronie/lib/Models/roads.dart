/*
* Christian Krueger Health LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 4.15.20
*
*/


import 'package:doxadronie/Pages/home.dart';
import 'package:doxadronie/Pages/loading.dart';
import 'package:doxadronie/Views/transitions.dart';
import 'package:flutter/material.dart';
import 'package:flutter/widgets.dart';

class Roads{
  
  static const String loading = '/Loading';
  static const String main = '/main';


  static MaterialPageRoute getRoutes(RouteSettings settings) {
    switch (settings.name) {
      case loading: return FadeRoute(LoadingPage(), settings: settings);
      case main: return FadeRoute(HomePage(), settings: settings);

      default:
        return FadeRoute(LoadingPage(), settings: settings);
    }
  }
}