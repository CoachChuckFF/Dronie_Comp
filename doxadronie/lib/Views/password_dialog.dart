import 'package:doxadronie/Models/dronie_colors.dart';
import 'package:flutter/material.dart';

class FactoryResetPopup {
  static factoryReset( 
    BuildContext context, 
    Future<void> Function() onReset,
  ) async {
    await showDialog(
      context: context, 
      barrierDismissible: true,
      builder: (BuildContext context){
        return KeyboardPadding(
          child: AlertDialog(
            backgroundColor: DColors.black,
            contentPadding: const EdgeInsets.all(16.0),
            title: Text(
              'Factory Rest Dronie',
              style: TextStyle(
                color: DColors.white
              ),
            ),
            actions: <Widget>[
              new FlatButton(
                  child: const Text(
                    'RESET',
                    style: TextStyle(
                      color: DColors.red
                    ),
                  ),
                  onPressed: () {
                    onReset().then((_) => Navigator.pop(context));
                  })
            ],
          ),
        );
      },
    );
  }
}


class PasswordPopup {
  static getPassword( 
    BuildContext context, 
    Future<void> Function(String) onClose,
  ) async {
    TextEditingController _controller = TextEditingController();
    await showDialog(
      context: context, 
      barrierDismissible: true,
      builder: (BuildContext context){
        return KeyboardPadding(
          child: AlertDialog(
            backgroundColor: DColors.black,
            contentPadding: const EdgeInsets.all(16.0),
            content: new Row(
              children: <Widget>[
                new Expanded(
                  child: new TextField(
                    controller: _controller,
                    obscureText: true,
                    autofocus: true,
                    style: TextStyle(
                      color: DColors.brightGreen,
                    ),
                    decoration: new InputDecoration(
                      labelText: 'WiFi Password',
                      iconColor: DColors.darkGreen,
                    ),
                  ),
                )
              ],
            ),
            actions: <Widget>[
              new FlatButton(
                  child: const Text(
                    'SET',
                    style: TextStyle(
                      color: DColors.white
                    ),
                  ),
                  onPressed: () {
                    onClose(_controller.text).then((_) => Navigator.pop(context));
                  })
            ],
          ),
        );
      },
    );
  }
}

class KeyboardPadding extends StatelessWidget {
  final Widget? child;

  KeyboardPadding({@required this.child});

  @override
  Widget build(BuildContext context) {
    return child ?? Container();
    var mediaQuery = MediaQuery.of(context);
    return new AnimatedContainer(
        padding: mediaQuery.viewInsets,
        duration: const Duration(milliseconds: 300),
        child: child
      );
  }
}