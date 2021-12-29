/*
* Blizzard Pro. LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 7/26/19
*
*/
//Extras
//At Full

import 'dart:typed_data';

class Mac{
  ByteData mac = ByteData(6);

  Mac(List<int> mac){
    int index = 0;
    this.mac = ByteData(6);
    mac.forEach((byte){
      if(index < 6){
        this.mac.setUint8(index++, byte);
      } else {
        return;
      }
    });
  }

  static Mac broadcast = Mac([0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]);

  @override
  int get hashCode => 3;

  static Mac fromString(String macString){
    List<int> list = [];
    list.add(int.parse("0x" + macString.substring(0, 2)));
    list.add(int.parse("0x" + macString.substring(3, 5)));
    list.add(int.parse("0x" + macString.substring(6, 8)));
    list.add(int.parse("0x" + macString.substring(9, 11)));
    list.add(int.parse("0x" + macString.substring(12, 14)));
    list.add(int.parse("0x" + macString.substring(15, 17)));

    return Mac(list);
  }

  static Mac fromBSSID(String macString){
    List<int> list = [];
    list.add(int.parse("0x" + macString.substring(0, 2)));
    list.add(int.parse("0x" + macString.substring(2, 4)));
    list.add(int.parse("0x" + macString.substring(4, 6)));
    list.add(int.parse("0x" + macString.substring(6, 8)));
    list.add(int.parse("0x" + macString.substring(8, 10)));
    list.add(int.parse("0x" + macString.substring(10, 12)));

    return Mac(list);
  }

  @override
  bool operator == (Object other) =>
    other is Mac &&
    other.mac.lengthInBytes == 6 &&
    other.mac.getUint8(0) == mac.getUint8(0) &&
    other.mac.getUint8(1) == mac.getUint8(1) &&
    other.mac.getUint8(2) == mac.getUint8(2) &&
    other.mac.getUint8(3) == mac.getUint8(3) &&
    other.mac.getUint8(4) == mac.getUint8(4) &&
    other.mac.getUint8(5) == mac.getUint8(5);

  @override
  String toString(){
    String zero = (mac.getUint8(0).toRadixString(16).length == 1) ? "0${mac.getUint8(0).toRadixString(16).toUpperCase()}" : "${mac.getUint8(0).toRadixString(16).toUpperCase()}";
    String one = (mac.getUint8(1).toRadixString(16).length == 1) ? "0${mac.getUint8(1).toRadixString(16).toUpperCase()}" : "${mac.getUint8(1).toRadixString(16).toUpperCase()}";
    String two = (mac.getUint8(2).toRadixString(16).length == 1) ? "0${mac.getUint8(2).toRadixString(16).toUpperCase()}" : "${mac.getUint8(2).toRadixString(16).toUpperCase()}";
    String three = (mac.getUint8(3).toRadixString(16).length == 1) ? "0${mac.getUint8(3).toRadixString(16).toUpperCase()}" : "${mac.getUint8(3).toRadixString(16).toUpperCase()}";
    String four = (mac.getUint8(4).toRadixString(16).length == 1) ? "0${mac.getUint8(4).toRadixString(16).toUpperCase()}" : "${mac.getUint8(4).toRadixString(16).toUpperCase()}";
    String five = (mac.getUint8(5).toRadixString(16).length == 1) ? "0${mac.getUint8(5).toRadixString(16).toUpperCase()}" : "${mac.getUint8(5).toRadixString(16).toUpperCase()}";


    return "$zero:$one:$two:$three:$four:$five";
  }
}