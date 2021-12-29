/*
* Christian Krueger Health LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 4.15.20
*
*/
import 'dart:math' as math;
import 'package:doxadronie/Models/roads.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_bloc/flutter_bloc.dart';


class FadeRoute<T> extends MaterialPageRoute<T> {
  // @override
  // Duration get transitionDuration => const Duration(milliseconds: 1000);

  FadeRoute(Widget widget, {RouteSettings? settings})
      : super(
            builder: (_) => widget,
            settings: settings);

  @override
  Widget buildTransitions(BuildContext context, Animation<double> animation,
      Animation<double> secondaryAnimation, Widget child) {
    return FadeTransition(
        opacity: Tween<double>(
          begin: 0,
          end: 1
        ).animate(animation),
        child: child);
  }
}
