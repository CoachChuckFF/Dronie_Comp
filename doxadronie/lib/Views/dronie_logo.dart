import 'package:flutter/widgets.dart';

class DronieIcon extends StatelessWidget {
  const DronieIcon({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    double size = MediaQuery.of(context).size.width * 0.55;

    return Hero(
      tag: "Dronie",
      child: Container(
        margin: EdgeInsets.all(34),
        width: size,
        height: size,
        child: Center(
          child: Image.asset('assets/images/icons/foreground_icon.png'),
        ),
      ),
    );
  }
}
