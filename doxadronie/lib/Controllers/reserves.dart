/*
* Blizzard Pro. LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 7/26/19
*
*/
//Extras
import 'dart:math' as math;
import 'dart:io';

/*Generates a 32 bit uuid, (so not a real uuid)*/
int blizzardGenerateUUID(int seed){
  var rnJesus = math.Random(seed);
  int uuid = (rnJesus.nextInt(0xFF) << 24) & 0xFF000000;
  uuid |= (rnJesus.nextInt(0xFF) << 16) & 0x00FF0000;
  uuid |= (rnJesus.nextInt(0xFF) << 8) & 0x0000FF00;
  uuid |= (rnJesus.nextInt(0xFF)) & 0x000000FF;

  return uuid;
}

/* Choses a random index within a given List */
int blizzardRandomIndex(int length, [int seed = -1]){
  if(seed == -1){
    return (math.Random(seed).nextInt(length));
  } else {
    return blizzardGenerateUUID(DateTime.now().millisecondsSinceEpoch) % length;
  }
}

/*Removes excess NULL terminators from a given string and returns a cleaned string*/
String blizzardRemoveZeros(String string){
  List<int> codes = List<int>.from(string.codeUnits);

  codes.removeWhere((code) => code == 0);

  return String.fromCharCodes(codes);
}

/*Turns IP into an actual String*/
String blizzardIPToString(InternetAddress address){
  var temp = address.rawAddress;
  return temp[0].toString() + "." + temp[1].toString() + "." + temp[2].toString() + "." + temp[3].toString();
}

double blizzardDegreesToRadians(double degree){
  return math.pi / 180.0 * degree;
}

double blizzardRadiansToDegrees(double radians){
  return 180.0 / math.pi * radians;
}

String blizzardMillisecondsToNiceString(int milliseconds, {bool shorten = false}){
  int value = 0;
  String tempString;

  if(milliseconds == 0) {
    return "None!";
  } else if(milliseconds < 500) {
    return milliseconds.toString() + "ms";
  } else if(milliseconds < 60000) {
    value = (milliseconds / 1000).round();
    tempString = value.toString() + (shorten ? " s" : " second");
  } else if(milliseconds < 6000000) {
    value = (milliseconds / 60000).round();
    tempString = value.toString() + (shorten ? " m" : " minute");
  } else {
    value = (milliseconds / 6000000).round();
    tempString = value.toString() + (shorten ? " h" : " hour");
  }

  if(!shorten && value != 1) {
    tempString += "s";
  }

  return tempString;
}


String blizzardSecondsToNiceString(double seconds){
  int value = 0;
  String tempString;

  if(seconds == 0) {
    return "None!";
  } else if(seconds < 10) {
    return seconds.toStringAsFixed(1) + " s";
  } else if(seconds < 100) {
    tempString = seconds.toStringAsFixed(0) + " s";
  } else if(seconds < 3600) {
    value = (seconds / 60).round();
    tempString = value.toString() + " m";
  } else {
    value = (seconds / 3600).round();
    tempString = value.toString() + " h";
  }

  return tempString;
}