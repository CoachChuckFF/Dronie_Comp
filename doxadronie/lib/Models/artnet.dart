/*
* Blizzard Pro. LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 7/26/19
*
*/
//Extras
import 'dart:math';
import 'dart:typed_data';
import 'dart:io';
import 'dart:ui';
//At Full
import 'package:doxadronie/Controllers/reserves.dart';

import 'dronie_state.dart';

class ArtnetHelpers {
  static const int protVersion = 14;
  static const int opCodeIndex = 8;
  static const int port = 6454;
  static const String broadcast = "255.255.255.255";
  static const String espIp = "192.168.4.255";
  static const String genericIp = "192.168.1.1";

  static void artnetCopyIdtoBuffer(ByteData buffer, int opCode){
    List<int> id = [0x41, 0x72, 0x74, 0x2D, 0x4E, 0x65, 0x74, 0x00];
    for(var i = 0; i < id.length; i++){
      buffer.setUint8(i, id[i]);
    }
    buffer.setUint16(opCodeIndex, opCode, Endian.little);
  }

  static bool artnetCheckPacket(List<int> packet){
    List<int> id = [0x41, 0x72, 0x74, 0x2D, 0x4E, 0x65, 0x74, 0x00];
    for(var i = 0; i < id.length; i++){
      if(packet[i] != id[i]){
        return false;
      }
    } 

    return true;
  }

  static int artnetGetOpCode(List<int> packet){
    if(packet.length <= opCodeIndex + 1) return -1;
    return packet[opCodeIndex + 1] << 8 | packet[opCodeIndex];
  }

  static String artnetOpCodeToString(int code){
    switch(code){
      case ArtnetDataPacket.opCode:
        return "Data";
      case ArtnetPollPacket.opCode:
        return "Poll";
      case ArtnetPollReplyPacket.opCode:
        return "Poll Reply";
      default:
        return "Unkown";
    }
  }
}

class DronieCommandPacket implements ArtnetPacket{
  static const type = "Dronie Command";
  static const size = 26+47;
  static const opCode = 0xCC03;

  /* Indexes */
  static const int commandIndex = ArtnetHelpers.opCodeIndex + 2;
  static const int valueIndex = commandIndex + 1;
  static const int led0Index = valueIndex + 1;
  static const int led1Index = led0Index + DronieState.ledSize;
  static const int led2Index = led1Index + DronieState.ledSize;
  static const int led3Index = led2Index + DronieState.ledSize;
  static const int led4Index = led3Index + DronieState.ledSize;
  static const int ownerIndex = led4Index + DronieState.ledSize;

  /* Commands */
  static const int nop = 0x00;
  static const int setMode = 0x01;
  static const int setLed = 0x02;
  static const int setOwner = 0x03;
  static const int dot = 0x10;
  static const int dash = 0x11;
  static const int touchMotor = 0x20;
  static const int factoryReset = 0xFF;

  ByteData packet = ByteData(size);

  DronieCommandPacket([List<int> raw = const []]){
    if(raw.isNotEmpty){
      for(var i = 0; i < min(raw.length, packet.lengthInBytes); i++){
        this.packet.setUint8(i, raw[i]);
      }
      return;
    }

    //set id
    ArtnetHelpers.artnetCopyIdtoBuffer(this.packet, opCode);
  }

  factory DronieCommandPacket.createSetMode(DronieMode mode){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = setMode
      ..commandValue = mode.index;

    return cmd;
  }

  factory DronieCommandPacket.createSetLed(Color color, int index){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = setLed
      ..commandValue = index
      ..led0 = color
      ..led1 = color
      ..led2 = color
      ..led3 = color
      ..led4 = color;

    return cmd;
  }

  factory DronieCommandPacket.createSetOwner(String newOwner){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = setOwner
      ..owner = newOwner;
    return cmd;
  }


  factory DronieCommandPacket.createDot(){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = dot;
    return cmd;
  }

  factory DronieCommandPacket.createDash(){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = dash;
    return cmd;
  }

  factory DronieCommandPacket.createTouchMotor(){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = touchMotor;
    return cmd;
  }

  factory DronieCommandPacket.createFactoryReset(){
    DronieCommandPacket cmd = DronieCommandPacket()
      ..command = factoryReset;
    return cmd;
  }

  int get command => this.packet.getUint8(commandIndex);
  set command(int value) => this.packet.setUint8(commandIndex, value);

  int get commandValue => this.packet.getUint8(valueIndex);
  set commandValue(int value) => this.packet.setUint8(valueIndex, value);

  Color get led0 => DronieState.getLed(this.packet, led0Index, 0);
  set led0(Color color) => DronieState.setLed(this.packet, led0Index, 0, color);

  Color get led1 => DronieState.getLed(this.packet, led0Index, 0);
  set led1(Color color) => DronieState.setLed(this.packet, led0Index, 1, color);

  Color get led2 => DronieState.getLed(this.packet, led0Index, 0);
  set led2(Color color) => DronieState.setLed(this.packet, led0Index, 2, color);

  Color get led3 => DronieState.getLed(this.packet, led0Index, 0);
  set led3(Color color) => DronieState.setLed(this.packet, led0Index, 3, color);

  Color get led4 => DronieState.getLed(this.packet, led0Index, 0);
  set led4(Color color) => DronieState.setLed(this.packet, led0Index, 4, color);

  String get owner => blizzardRemoveZeros(String.fromCharCodes(this.packet.buffer.asUint8List(ownerIndex, DronieState.ownerSize)));
  set owner(String value){
    for(var i = 0; i < DronieState.ownerSize - 1; i++){
      if(value.length <= i){
        this.packet.setUint8(ownerIndex + i, 0);
      } else {
        this.packet.setInt8(ownerIndex + i, value.codeUnitAt(i));
      }
    }
    this.packet.setUint8(ownerIndex + DronieState.ownerSize - 2, 0); //Null terminate just in case
  }


  List<int> get udpPacket => this.packet.buffer.asUint8List();

  @override
  String toString() {
    return toHexString();
  }

  @override
  String toHexString(){
    String string = "";
    String tempString = "";
    for(var i = 0; i < this.udpPacket.length; i++){
      tempString = this.udpPacket[i].toRadixString(16).toUpperCase();
      if(tempString.length < 2) tempString = "0" + tempString;
      string += tempString;
    }
    return string;
  }
}

// ===================== ARTNET ======================================

abstract class ArtnetPacket {
  static var type;
  static var size;
  static var opCode;

  List<int> get udpPacket;

  String toHexString();
}


class ArtnetDataPacket implements ArtnetPacket{
  static const type = "Artnet Data Packet";
  static const size = 18;
  static const opCode = 0x5000;
  static const defaultDataLength = 512;

  /* Indexes */
  static const protVersionHiIndex = ArtnetHelpers.opCodeIndex + 2;
  static const protVersionLoIndex = protVersionHiIndex + 1;
  static const sequenceIndex = protVersionLoIndex + 1;
  static const physicalIndex = sequenceIndex + 1;
  static const subUniIndex = physicalIndex + 1;
  static const netIndex = subUniIndex + 1;
  static const lengthHiIndex = netIndex + 1;
  static const lengthIndex = lengthHiIndex + 1;
  static const dataIndex = lengthIndex + 1;

  ByteData packet = ByteData(0);

  ArtnetDataPacket([List<int> packet = const [], int dmxLength = defaultDataLength]){
    this.packet = new ByteData(size + dmxLength);
    if(packet.isNotEmpty){
      for(var i = 0; i < size + dmxLength; i++){
        if(packet.length <= i) return;
        this.packet.setUint8(i, packet[i]);
      }
      return;
    }

    //set id
    ArtnetHelpers.artnetCopyIdtoBuffer(this.packet, opCode);

    //set protocol version
    this.artnetProtVersion = ArtnetHelpers.protVersion;

    //set length
    this.dmxLength = dmxLength;
    
  }

  int get artnetProtVerHi => this.packet.getUint8(protVersionHiIndex);
  set artnetProtVerHi(int value) => this.packet.setUint8(protVersionHiIndex, value);

  int get artnetProtVerLo => this.packet.getUint8(protVersionLoIndex);
  set artnetProtVerLo(int value) => this.packet.setUint8(protVersionLoIndex, value);

  int get artnetProtVersion => this.packet.getUint16(protVersionHiIndex);
  set artnetProtVersion(int value) => this.packet.setUint16(protVersionHiIndex, value);

  int get sequence => this.packet.getUint8(sequenceIndex);
  set sequence(int value) => this.packet.setUint8(sequenceIndex, value);

  int get physical => this.packet.getUint8(physicalIndex);
  set physical(int value) => this.packet.setUint8(physicalIndex, value);

  int get subUni => this.packet.getUint8(subUniIndex);
  set subUni(int value) => this.packet.setUint8(subUniIndex, value);

  int get net => this.packet.getUint8(netIndex);
  set net(int value) => this.packet.setUint8(netIndex, value);

  int get universe => ((this.net << 8) & 0x7F00) | this.subUni & 0xFF;
  set universe(int value){
    this.subUni = value & 0xFF;
    this.net = (value >> 8) & 0x7F;
  }

  int get lengthHi => this.packet.getUint8(lengthHiIndex);
  set lengthHi(int value) => this.packet.setUint8(lengthHiIndex, value);

  int get lengthLo => this.packet.getUint8(lengthIndex);
  set lengthLo(int value) => this.packet.setUint8(lengthIndex, value);

  int get dmxLength => this.packet.getUint16(lengthHiIndex);
  set dmxLength(int value){
    if(value > defaultDataLength || value < 0){
      return;
    }

    this.packet.setUint16(lengthHiIndex, value);
  }

  void blackout() {
    for(var i = 0; i < defaultDataLength; i++){
      this.packet.setUint8(dataIndex + i, 0x00);
    }
  }

  void whiteout() {
    for(var i = 0; i < defaultDataLength; i++){
      this.packet.setUint8(dataIndex + i, 0xFF);
    }
  }

  void copyDmxToPacket(ByteData data) {
    for(var i = 0; i < defaultDataLength; i++){
      this.packet.setUint8(dataIndex + i, data.getUint8(i));
    }
  }

  void copyDmxFromPacket(ByteData data) {
    for(var i = 0; i < defaultDataLength; i++){
      data.setUint8(i, this.packet.getUint8(dataIndex + i));
    }
  }

  void setDmxValue(int address, int value){
    if(address > defaultDataLength || address < 1){
      return;
    }

    this.packet.setUint8(dataIndex + address - 1, value);
  }

  void setDmxValues(List<int> addresses, int value){

    addresses.forEach((address){
      if(address > defaultDataLength || address < 1){
        return;
      }
      this.packet.setUint8(dataIndex + address - 1, value);
    });
  }

  List<int> get dmx => this.packet.buffer.asUint8List(dataIndex, this.dmxLength);
  set dmx(List<int> dmx){
    for(var i = 0; i < defaultDataLength; i++){
      this.packet.setUint8(dataIndex + i, dmx[i] & 0xFF);
    }
  }

  List<int> get udpPacket => this.packet.buffer.asUint8List(0, size + this.dmxLength);

  @override
  String toString() {
    String string = "***$type***\n";
    string += "Id: " + String.fromCharCodes(this.packet.buffer.asUint8List(0, 7)) + "\n";
    string += "Opcode: 0x" + this.packet.getUint16(ArtnetHelpers.opCodeIndex).toRadixString(16) + "\n";
    string += "Protocol Version: " + this.artnetProtVersion.toString() + "\n";
    string += "Sequence: " + this.sequence.toString() + "\n";
    string += "Physical: " + this.physical.toString() + "\n";
    string += "Universe: " + this.universe.toString() + "\n";
    string += "*** Net: " + this.net.toString() + "\n";
    string += "*** Sub Universe: " + this.subUni.toString() + "\n";
    string += "Data Length: " + this.dmxLength.toString() + "\n";
    string += "Data:";
    for(var i = 1; i <= this.dmxLength; i++){
      if(((i-1) % 16) == 0){
        string +="\n*** ";
      }
      String tempString = " " + (i).toString() + ":" + this.dmx[i-1].toString() + " ";
      while(tempString.length < 9){
        tempString += " ";
      }
      string += tempString;
    }
    string += "\n**********************\n";

    return string;
  }

  String toHexString(){
    String string = "";
    String tempString = "";
    for(var i = 0; i < this.udpPacket.length; i++){
      tempString = this.udpPacket[i].toRadixString(16).toUpperCase();
      if(tempString.length < 2) tempString = "0" + tempString;
      string += tempString;
    }
    return string;
  }

}

class ArtnetPollPacket implements ArtnetPacket{
  static const type = "Artnet Poll Packet";
  static const size = 14;
  static const opCode = 0xCC01;

  /* Indexes */
  static const protVersionHiIndex = ArtnetHelpers.opCodeIndex + 2;
  static const protVersionLoIndex = protVersionHiIndex + 1;
  static const talkToMeIndex = protVersionLoIndex + 1;
  static const priorityIndex = talkToMeIndex + 1;

  /* Masks */
  static const talkToMeVlcTransmissionEnableMask = 0x10;
  static const talkToMeDiagnosticsTransmissionMask = 0x08;
  static const talkToMeDiagnosticsEnableMask = 0x04;
  static const talkToMePollReplyOptionMask = 0x02;

  /* Options */
  static const talkToMeDiagnosticsTransmissionOptionBroadcast = 0;
  static const talkToMeDiagnosticsTransmissionOptionUnicast = 1;
  static const talkToMePollReplyOptionOnlyInResponse = 0;
  static const talkToMePollReplyOptionOnChange = 1;

  ByteData packet = ByteData(0);

  ArtnetPollPacket([List<int> packet = const []]){
    this.packet = new ByteData(size);
    if(packet.isNotEmpty){
      for(var i = 0; i < size; i++){
        if(packet.length <= i) return;
        this.packet.setUint8(i, packet[i]);
      }
      return;
    }

    //set id
    ArtnetHelpers.artnetCopyIdtoBuffer(this.packet, opCode);

    //set protocol version
    this.artnetProtVersion = ArtnetHelpers.protVersion;
  }

  int get artnetProtVerHi => this.packet.getUint8(protVersionHiIndex);
  set artnetProtVerHi(int value) => this.packet.setUint8(protVersionHiIndex, value);

  int get artnetProtVerLo => this.packet.getUint8(protVersionLoIndex);
  set artnetProtVerLo(int value) => this.packet.setUint8(protVersionLoIndex, value);

  int get artnetProtVersion => this.packet.getUint16(protVersionHiIndex);
  set artnetProtVersion(int value) => this.packet.setUint16(protVersionHiIndex, value);

  int get talkToMe => this.packet.getUint8(talkToMeIndex);
  set talkToMe(int value) => this.packet.setUint8(talkToMeIndex, value);

  bool get talkToMeVlcTransmissionEnable => ((this.talkToMe & talkToMeVlcTransmissionEnableMask) == 0);
  set talkToMeVlcTransmissionEnable(bool value) => (value) ? this.talkToMe &= ~talkToMeVlcTransmissionEnableMask : this.talkToMe |= talkToMeVlcTransmissionEnableMask;

  bool get talkToMeDiagnosticsEnable => ((this.talkToMe & talkToMeDiagnosticsEnableMask) != 0);
  set talkToMeDiagnosticsEnable(bool value) => (value) ? this.talkToMe |= talkToMeDiagnosticsEnableMask : this.talkToMe &= ~talkToMeDiagnosticsEnableMask;

  int get talkToMeDiagnosticsTransmission => (this.talkToMe & talkToMeDiagnosticsTransmissionMask) >> 3;
  set talkToMeDiagnosticsTransmission(int value) => (value == talkToMeDiagnosticsTransmissionOptionUnicast) ? this.talkToMe |= talkToMeDiagnosticsTransmissionMask : this.talkToMe &= ~talkToMeDiagnosticsTransmissionMask;

  int get talkToMePollReplyOption => (this.talkToMe & talkToMePollReplyOptionMask) >> 1;
  set talkToMePollReplyOption(int value) => (value == talkToMePollReplyOptionOnChange) ? this.talkToMe |= talkToMePollReplyOptionMask : this.talkToMe &= ~talkToMePollReplyOptionMask;

  int get priority => this.packet.getUint8(priorityIndex);
  set priority(int value) => this.packet.setUint8(priorityIndex, value);

  List<int> get udpPacket => this.packet.buffer.asUint8List();

  @override
  String toString() {
    String string = "***$type***\n";
    string += "Id: " + String.fromCharCodes(this.packet.buffer.asUint8List(0, 7)) + "\n";
    string += "Opcode: 0x" + this.packet.getUint16(ArtnetHelpers.opCodeIndex).toRadixString(16) + "\n";
    string += "Protocol Version: " + this.artnetProtVersion.toString() + "\n";
    string += "Talk to me: 0x" + this.talkToMe.toRadixString(16) + "\n";
    string += "*** VLC Transmission: " + ((this.talkToMeVlcTransmissionEnable) ? "Enabled" : "Disabled") + "\n";
    string += "*** Diagnostics: " + ((this.talkToMeDiagnosticsEnable) ? "Enabled" : "Disabled") + "\n";
    string += "*** Diagnostics are " + ((this.talkToMeDiagnosticsTransmission == talkToMeDiagnosticsTransmissionOptionBroadcast) ? "broadcast" : "unicast") + "\n";
    string += "*** Send art poll reply " + ((this.talkToMePollReplyOption == talkToMePollReplyOptionOnChange) ? "on config change" : "in response to art poll") + "\n";
    string += "Priority: " + this.priority.toString() + "\n";

    return string;
  }

  String toHexString(){
    String string = "";
    String tempString = "";
    for(var i = 0; i < size; i++){
      tempString = this.udpPacket[i].toRadixString(16).toUpperCase();
      if(tempString.length < 2) tempString = "0" + tempString;
      string += tempString;
    }
    return string;
  }

}

class ArtnetPollReplyPacket implements ArtnetPacket {
  static const type = "Artnet Poll Reply Packet";
  static const size = 234;
  static const opCode = 0xCC02;
  static const magic = [0x42, 0x6C, 0x69, 0x7A, 0x7A, 0x61, 0x72, 0x64]; //Blizzard Pro Preamble

  /* Legacy SoC-It */
  static const legacySoCItMagic = [0x53, 0x6F, 0x43, 0x00];

  /* Sizes */
  static const ipAddressSize = 4;
  static const portSize = 2;
  static const shortNameSize = 18;
  static const longNameSize = 64;
  static const nodeReportSize = 64;

  static const blizzardMagicSize = 8;
  static const blizzardTypeSize = 2;
  static const blizzardShowNameSize = 16;
  static const blizzardShowCurrentFrameSize = 4;
  static const blizzardShowTotalFramesSize = 4;
  static const blizzardShowCurrentTimeSize = 4;
  static const blizzardShowTotalTimeSize = 4;
  static const blizzardSSIDSize = 16;

  static const portTypesSize = 4;
  static const goodInputSize = 4;
  static const goodOutputSize = 4;
  static const swInSize = 4;
  static const swOutSize = 4;
  static const spareSize = 3;
  static const bindIpSize = 4;
  static const fillerSize = 26;

  /* Indexes */
  static const ipAddressIndex = ArtnetHelpers.opCodeIndex + 2;
  static const portIndex = ipAddressIndex + ipAddressSize;
  static const versInfoHIndex = portIndex + portSize;
  static const versInfoLIndex = versInfoHIndex + 1;
  static const netSwitchIndex = versInfoLIndex + 1;
  static const subSwitchIndex = netSwitchIndex + 1;
  static const oemHiIndex = subSwitchIndex + 1;
  static const oemIndex = oemHiIndex + 1;
  static const ubeaVersionIndex = oemIndex + 1;
  static const status1Index = ubeaVersionIndex + 1;
  static const estaManLoIndex = status1Index + 1;
  static const estaManHiIndex = estaManLoIndex + 1;
  static const shortNameIndex = estaManHiIndex + 1;
  static const longNameIndex = shortNameIndex + shortNameSize;
  static const nodeReportIndex = longNameIndex + longNameSize;

  static const blizzardMagicIndex = nodeReportIndex; //Blizzard Pro Proprietary Info
  static const blizzardTypeIndex = blizzardMagicIndex + blizzardMagicSize; //Blizzard Pro Proprietary Info
  static const blizzardBatteryStateIndex = blizzardTypeIndex + blizzardTypeSize; //Blizzard Pro Proprietary Info
  static const blizzardBatteryLevelIndex = blizzardBatteryStateIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowInternalStateIndex = blizzardBatteryLevelIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowStateIndex = blizzardShowInternalStateIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowNameIndex = blizzardShowStateIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowOnLoopIndex = blizzardShowNameIndex + blizzardShowNameSize; //Blizzard Pro Proprietary Info
  static const blizzardShowOnStartIndex = blizzardShowOnLoopIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowCurrentFrameIndex = blizzardShowOnStartIndex + 1; //Blizzard Pro Proprietary Info
  static const blizzardShowTotalFramesIndex = blizzardShowCurrentFrameIndex + blizzardShowCurrentFrameSize; //Blizzard Pro Proprietary Info
  static const blizzardShowCurrentTimeIndex = blizzardShowTotalFramesIndex + blizzardShowTotalFramesSize; //Blizzard Pro Proprietary Info
  static const blizzardShowTotalTimeIndex = blizzardShowCurrentTimeIndex + blizzardShowCurrentTimeSize; //Blizzard Pro Proprietary Info
  static const blizzardSSIDIndex = blizzardShowTotalTimeIndex + blizzardShowTotalTimeSize;

  static const numPortsHiIndex = nodeReportIndex + nodeReportSize;
  static const numPortsLoIndex = numPortsHiIndex + 1;
  static const portTypesIndex = numPortsLoIndex + 1;
  static const goodInputIndex = portTypesIndex + portTypesSize;
  static const goodOutputIndex = goodInputIndex + goodInputSize;
  static const swInIndex = goodOutputIndex + goodOutputSize;
  static const swOutIndex = swInIndex + swInSize;
  static const swVideoIndex = swOutIndex + swOutSize;
  static const swMacroIndex = swVideoIndex + 1;
  static const swRemoteIndex = swMacroIndex + 1;
  static const spareIndex = swRemoteIndex + 1;

  static const blizzardUIStateIndex = swRemoteIndex + 1;
  static const blizzardProtocolStateIndex = blizzardUIStateIndex + 1;
  static const blizzardSpareIndex = blizzardProtocolStateIndex + 1;

  static const styleIndex = spareIndex + spareSize;
  static const macHiIndex = styleIndex + 1;
  static const mac4Index = macHiIndex + 1;
  static const mac3Index = mac4Index + 1;
  static const mac2Index = mac3Index + 1;
  static const mac1Index = mac2Index + 1;
  static const macLoIndex = mac1Index + 1;
  static const bindIpIndex = macLoIndex + 1;
  static const bindIndexIndex = bindIpIndex + bindIpSize;
  static const status2Index = bindIndexIndex + 1;
  static const fillerIndex = status2Index + 1;

  static const hemisphereDMXAddress = 122;
  static const hemisphereChannelIndex = 124;
  static const hemisphereWorkingModeIndex = 125;
  static const hemisphereBatteryLevelIndex = 131;
  static const hemisphereTempIndex = 132;
  static const hemisphereSSIDIndex = 134;
  static const hemisphereChargingIndex = 166;

  /* SoC-It Legacy Index */
  static const legacySoCItIndex = numPortsHiIndex - 4;

  /* Bitmasks */
  static const netSwitchMask = 0x7F;
  static const subSwitchMask = 0x0F;
  static const ioSwitchMask = 0x0F;
  static const status1IndicatorStateMask = 0xC0;
  static const status1ProgrammingAuthorityMask = 0x30;
  static const status1FirmwareBootMask = 0x04;
  static const status1RdmCapableMask = 0x02;
  static const status1UbeaPresentMask = 0x01;
  static const portTypesOutputArtnetAbleMask = 0x80;
  static const portTypesInputArtnetAbleMask = 0x40;
  static const portTypesProtocolMask = 0x3F;
  static const goodInputDataReceivedMask = 0x80;
  static const goodInputIncludesTestPackets1Mask = 0x40;
  static const goodInputIncludesSIPsMask = 0x20;
  static const goodInputIncludesTestPackets2Mask = 0x10;
  static const goodInputInputDisableMask = 0x08;
  static const goodInputReceiveErrorDetectedMask = 0x04;
  static const goodOutputDataTransmitingMask = 0x80;
  static const goodOutputIncludesTestPackets1Mask = 0x40;
  static const goodOutputIncludesSIPsMask = 0x20;
  static const goodOutputIncludesTestPackets2Mask = 0x10;
  static const goodOutputIsMergingMask = 0x08;
  static const goodOutputShortDetectedMask = 0x04;
  static const goodOutputMergeIsLTPMask = 0x02;
  static const goodOutputProtocolMask = 0x01;
  static const status2IsSquawkingMask = 0x20;
  static const status2ProtocolSwitchableMask = 0x10;
  static const status215BitSupportMask = 0x08;
  static const status2DHCPCapableMask = 0x04;
  static const status2IpIsSetManuallyMask = 0x02;
  static const status2HasWebConfigurationSupportMask = 0x01;

  /* Options */
  static const status1IndicatorStateOptionUnkown = 0x00;
  static const status1IndicatorStateOptionLocate = 0x01;
  static const status1IndicatorStateOptionMute = 0x02;
  static const status1IndicatorStateOptionNormal = 0x03;
  static const status1ProgrammingAuthorityOptionUnknown = 0x00;
  static const status1ProgrammingAuthorityOptionPanel = 0x01;
  static const status1ProgrammingAuthorityOptionNetwork = 0x02;
  static const status1FirmwareBootOptionNormal = 0x00;
  static const status1FirmwareBootOptionRom = 0x01;
  static const portTypesProtocolOptionDMX = 0x00;
  static const portTypesProtocolOptionMidi = 0x01;
  static const portTypesProtocolOptionAvab = 0x02;
  static const portTypesProtocolOptionColortranCMX = 0x03;
  static const portTypesProtocolOptionADB62_5 = 0x04;
  static const portTypesProtocolOptionArtnet = 0x05;
  static const goodOutputProtocolOptionArtnet = 0x00;
  static const goodOutputProtocolOptionSacn = 0x01;
  static const styleOptionStNode = 0x00;
  static const styleOptionStController = 0x01;
  static const styleOptionStMedia = 0x02;
  static const styleOptionStRoute = 0x03;
  static const styleOptionStBackup = 0x04;
  static const styleOptionStConfig = 0x05;
  static const styleOptionStVisual = 0x06;

  ByteData packet = ByteData(0);

  ArtnetPollReplyPacket([List<int> packet = const []]){
    this.packet = new ByteData(size);
    if(packet.isNotEmpty){
      for(var i = 0; i < size; i++){
        if(packet.length <= i) return;
        this.packet.setUint8(i, packet[i]);
      }
      return;
    }

    //set id
    ArtnetHelpers.artnetCopyIdtoBuffer(this.packet, opCode);
  }

  List<int> get ip => this.packet.buffer.asUint8List(ipAddressIndex, ipAddressSize);
  set ip(List<int> value){
    for(var i = 0; i < ipAddressSize; i++){
      if(value.length <= i){
        this.packet.setUint8(ipAddressIndex + i, 0);    
      } else {
        this.packet.setUint8(ipAddressIndex + i, value[i]);
      }
    }
  }

  InternetAddress get ipAddress{
    List<int> ip = this.ip;
    String ipString;

    ipString = ip[3].toString() + ".";
    ipString += ip[2].toString() + ".";
    ipString += ip[1].toString() + ".";
    ipString += ip[0].toString();

    return InternetAddress(ipString);
  }

  int get port => this.packet.getUint16(portIndex, Endian.little);
  set port(int value) => this.packet.setUint16(portIndex, value, Endian.little);

  int get versionInfoH => this.packet.getUint8(versInfoHIndex);
  set versionInfoH(int value) => this.packet.setUint8(versInfoHIndex, value);

  int get versionInfoL => this.packet.getUint8(versInfoLIndex);
  set versionInfoL(int value) => this.packet.setUint8(versInfoLIndex, value);

  int get netSwitch => this.packet.getUint8(netSwitchIndex);
  set netSwitch(int value) => this.packet.setUint8(netSwitchIndex, value & netSwitchMask);

  int get subSwitch => this.packet.getUint8(subSwitchIndex);
  set subSwitch(int value) => this.packet.setUint8(subSwitchIndex, value & subSwitchMask);

  int get universe => (this.netSwitch & 0x7F) << 8 | (this.subSwitch & 0x0F) << 4 | (this.swOut[0] & 0x0F);
  set universe(int value){
    this.netSwitch = ((value >> 8) & 0x7F);
    this.subSwitch = ((value >> 4) & 0x0F);
    this.swOut[0] = (value & 0x0F);
  }

  int get oemHi => this.packet.getUint8(oemHiIndex);
  set oemHi(int value) => this.packet.setUint8(oemHiIndex, value);

  int get oemLo => this.packet.getUint8(oemIndex);
  set oemLo(int value) => this.packet.setUint8(oemIndex, value);

  int get oem => (this.oemHi << 8) & 0xFF00 | this.oemLo & 0xFF;
  set oem(int value) {
    this.oemHi = (value >> 8);  
    this.oemLo = value & 0xFF;
  }

  int get ubeaVersion => this.packet.getUint8(ubeaVersionIndex);
  set ubeaVersion(int value) => this.packet.setUint8(ubeaVersionIndex, value);

  int get status1 => this.packet.getUint8(status1Index);
  set status1(int value) => this.packet.setUint8(status1Index, value);

  int get status1IndicatorState => (this.status1 & status1IndicatorStateMask) >> 6;
  set status1IndicatorState(int value){
    //clear value
    this.status1 &= ~status1IndicatorStateMask;
    //set value
    this.status1 |= ((value << 6) & status1IndicatorStateMask);
  }

  int get status1ProgrammingAuthority => (this.status1 & status1ProgrammingAuthorityMask) >> 4;
  set status1ProgrammingAuthority(int value){
    //clear value
    this.status1 &= ~status1ProgrammingAuthorityMask;
    //set value
    this.status1 |= ((value << 4) & status1ProgrammingAuthorityMask);
  }

  int get status1FirmwareBoot => (this.status1 & status1ProgrammingAuthorityMask) >> 2;
  set status1FirmwareBoot(int value) => (value == status1FirmwareBootOptionRom) ? this.status1 |= status1FirmwareBootMask : this.status1 &= ~status1FirmwareBootMask;

  bool get status1RdmCapable => ((this.status1 & status1RdmCapableMask) != 0);
  set status1RdmCapable(bool value) => (value) ? this.status1 |= status1RdmCapableMask : this.status1 &= ~status1RdmCapableMask;
  
  bool get status1UbeaPresent => ((this.status1 & status1UbeaPresentMask) != 0);
  set status1UbeaPresent(bool value) => (value) ? this.status1 |= status1UbeaPresentMask : this.status1 &= ~status1UbeaPresentMask;

  int get estaManLo => this.packet.getUint8(estaManLoIndex);
  set estaManLo(int value) => this.packet.setUint8(estaManLoIndex, value);

  int get estaManHi => this.packet.getUint8(estaManHiIndex);
  set estaManHi(int value) => this.packet.setUint8(estaManHiIndex, value);

  int get estaMan => this.estaManHi << 8 | this.estaManLo;
  set estaMan(int value){
    this.estaManHi = value >> 8;
    this.estaManLo = value & 0xFF;
  }

  String get shortName => blizzardRemoveZeros(String.fromCharCodes(this.packet.buffer.asUint8List(shortNameIndex, shortNameSize)));
  set shortName(String value){
    for(var i = 0; i < shortNameSize; i++){
      if(value.length <= i){
        this.packet.setUint8(shortNameIndex + i, 0);
      } else {
        this.packet.setInt8(shortNameIndex + i, value.codeUnitAt(i));
      }
    }
    this.packet.setUint8(shortNameIndex + shortNameSize - 1, 0); //Null terminate just in case
  }

  String get longName => blizzardRemoveZeros(String.fromCharCodes(this.packet.buffer.asUint8List(longNameIndex, longNameSize)));
  set longName(String value){
    for(var i = 0; i < longNameSize; i++){
      if(value.length <= i){
        this.packet.setUint8(longNameIndex + i, 0);
      } else {
        this.packet.setInt8(longNameIndex + i, value.codeUnitAt(i));
      }
    }
    this.packet.setUint8(longNameIndex + longNameSize - 1, 0); //Null terminate just in case
  }

  String get nodeReport => blizzardRemoveZeros(String.fromCharCodes(packet.buffer.asUint8List(nodeReportIndex, nodeReportSize)));
  set nodeReport(String value){
    for(var i = 0; i < nodeReportSize; i++){
      if(value.length <= i){
        this.packet.setUint8(nodeReportIndex + i, 0);
      } else {
        this.packet.setInt8(nodeReportIndex + i, value.codeUnitAt(i));
      }
    }
    this.packet.setUint8(nodeReportIndex + nodeReportSize - 1, 0); //Null terminate just in case
  }

  int get numPortsHi => this.packet.getUint8(numPortsHiIndex);
  set numPortsHi(int value) => this.packet.setUint8(numPortsHiIndex, value);

  int get numPortsLo => this.packet.getUint8(numPortsLoIndex);
  set numPortsLo(int value) => this.packet.setUint8(numPortsLoIndex, value);

  int get numPorts => this.packet.getUint16(numPortsHiIndex);
  set numPorts(int value) => this.packet.setUint16(numPortsHiIndex, value);

  List<int> get portTypes => this.packet.buffer.asUint8List(portTypesIndex, portTypesSize);
  void setPortType(int index, int value){
    if(index >= portTypesSize || index < 0){
      return;
    }
    this.packet.setUint8(portTypesIndex + index, value);
  }

  bool getPortTypesOutputArtnetAble(int index) => (index >= portTypesSize) ? false : ((this.portTypes[index] & portTypesOutputArtnetAbleMask) != 0x00);
  void setPortTypesOutputArtnetAble(int index, bool value){
    if(index >= portTypesSize) return;

    (value) ? this.portTypes[index] |= portTypesOutputArtnetAbleMask : this.portTypes[index] &= ~portTypesOutputArtnetAbleMask;
  } 

  bool getPortTypesInputArtnetAble(int index) => (index >= portTypesSize) ? false : ((this.portTypes[index] & portTypesInputArtnetAbleMask) != 0x00);
  void setPortTypesInputArtnetAble(int index, bool value){
    if(index >= portTypesSize) return;
    
    (value) ? this.portTypes[index] |= portTypesInputArtnetAbleMask : this.portTypes[index] &= ~portTypesInputArtnetAbleMask;
  } 

  int getPortTypesProtocol(int index) => (index >= portTypesSize) ? 0x00 : (this.portTypes[index] & portTypesProtocolMask);
  void setPortTypesProtocol(int index, int value){
    if(index >= portTypesSize) return;

    //clear value
    this.portTypes[index] &= ~portTypesProtocolMask;
    //set value
    this.portTypes[index] |= (value & portTypesProtocolMask);
  }

  List<int> get goodInput => this.packet.buffer.asUint8List(goodInputIndex, goodInputSize);
  void setGoodInput(int index, int value){
    if(index >= goodInputSize || index < 0){
      return;
    }
    this.packet.setUint8(goodInputIndex + index, value);
  }

  bool getGoodInputDataReceived(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputDataReceivedMask) != 0x00);
  void setGoodInputDataReceived(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputDataReceivedMask : this.goodInput[index] &= ~goodInputDataReceivedMask;
  } 

  bool getGoodInputIncludesTestPackets1(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputIncludesTestPackets1Mask) != 0x00);
  void setGoodInputIncludesTestPackets1(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputIncludesTestPackets1Mask : this.goodInput[index] &= ~goodInputIncludesTestPackets1Mask;
  } 

  bool getGoodInputIncludesSIPs(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputIncludesSIPsMask) != 0x00);
  void setGoodInputIncludesSIPs(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputIncludesSIPsMask : this.goodInput[index] &= ~goodInputIncludesSIPsMask;
  } 

  bool getGoodInputIncludesTestPackets2(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputIncludesTestPackets2Mask) != 0x00);
  void setGoodInputIncludesTestPackets2(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputIncludesTestPackets2Mask : this.goodInput[index] &= ~goodInputIncludesTestPackets2Mask;
  } 

  bool getGoodInputInputDisable(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputInputDisableMask) != 0x00);
  void setGoodInputInputDisable(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputInputDisableMask : this.goodInput[index] &= ~goodInputInputDisableMask;
  } 

  bool getGoodInputReceiveErrorDetected(int index) => (index >= goodInputSize) ? false : ((this.goodInput[index] & goodInputReceiveErrorDetectedMask) != 0x00);
  void setGoodInputReceiveErrorDetected(int index, bool value){
    if(index >= goodInputSize) return;

    (value) ? this.goodInput[index] |= goodInputReceiveErrorDetectedMask : this.goodInput[index] &= ~goodInputReceiveErrorDetectedMask;
  } 

  List<int> get goodOutput => this.packet.buffer.asUint8List(goodOutputIndex, goodOutputSize);
  void setGoodOutput(int index, int value){
    if(index >= goodOutputSize || index < 0){
      return;
    }
    this.packet.setUint8(goodOutputIndex + index, value);
  }

  bool getGoodOutputDataTransmiting(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputDataTransmitingMask) != 0x00);
  void setGoodOutputDataTransmiting(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputDataTransmitingMask : this.goodOutput[index] &= ~goodOutputDataTransmitingMask;
  } 

  bool getGoodOutputIncludesTestPackets1(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputIncludesTestPackets1Mask) != 0x00);
  void setGoodOutputIncludesTestPackets1(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputIncludesTestPackets1Mask : this.goodOutput[index] &= ~goodOutputIncludesTestPackets1Mask;
  } 

  bool getGoodOutputIncludesSIPs(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputIncludesSIPsMask) != 0x00);
  void setGoodOutputIncludesSIPs(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputIncludesSIPsMask : this.goodOutput[index] &= ~goodOutputIncludesSIPsMask;
  } 

  bool getGoodOutputIncludesTestPackets2(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputIncludesTestPackets2Mask) != 0x00);
  void setGoodOutputIncludesTestPackets2(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputIncludesTestPackets2Mask : this.goodOutput[index] &= ~goodOutputIncludesTestPackets2Mask;
  } 

  bool getGoodOutputIsMerging(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputIsMergingMask) != 0x00);
  void setGoodOutputIsMerging(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputIsMergingMask : this.goodOutput[index] &= ~goodOutputIsMergingMask;
  } 

  bool getGoodOutputShortDetected(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputShortDetectedMask) != 0x00);
  void setGoodOutputShortDetected(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputShortDetectedMask : this.goodOutput[index] &= ~goodOutputShortDetectedMask;
  } 

  bool getGoodOutputMergeIsLTP(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputMergeIsLTPMask) != 0x00);
  void setGoodOutputMergeIsLTP(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputMergeIsLTPMask : this.goodOutput[index] &= ~goodOutputMergeIsLTPMask;
  } 

  bool getGoodOutputProtocol(int index) => (index >= goodOutputSize) ? false : ((this.goodOutput[index] & goodOutputProtocolMask) != 0x00);
  void setGoodOutputProtocol(int index, bool value){
    if(index >= goodOutputSize) return;

    (value) ? this.goodOutput[index] |= goodOutputProtocolMask : this.goodOutput[index] &= ~goodOutputProtocolMask;
  } 

  bool getSACNEnabled ()=> getGoodOutputProtocol(0);

  List<int> get swIn => this.packet.buffer.asUint8List(swInIndex, swInSize);
  void setSwIn(int index, int value){
    if(index >= swInSize || index < 0){
      return;
    }
    this.packet.setUint8(swInIndex + index, value & ioSwitchMask);
  }

  List<int> get swOut => this.packet.buffer.asUint8List(swOutIndex, swOutSize);
  void setSwOut(int index, int value){
    if(index >= swOutSize || index < 0){
      return;
    }
    this.packet.setUint8(swOutIndex + index, value & ioSwitchMask);
  }

  int get swVideo => this.packet.getUint8(swVideoIndex);
  set swVideo(int value) => this.packet.setUint8(swVideoIndex, value);

  int get swMacro => this.packet.getUint8(swMacroIndex);
  set swMacro(int value) => this.packet.setUint8(swMacroIndex, value);

  int get swRemote => this.packet.getUint8(swRemoteIndex);
  set swRemote(int value) => this.packet.setUint8(swRemoteIndex, value);

  int get style => this.packet.getUint8(styleIndex);
  set style(int value) => this.packet.setUint8(styleIndex, value);

  int get macHi => this.packet.getUint8(macHiIndex);
  set macHi(int value) => this.packet.setUint8(macHiIndex, value);

  int get mac4 => this.packet.getUint8(mac4Index);
  set mac4(int value) => this.packet.setUint8(mac4Index, value);

  int get mac3 => this.packet.getUint8(mac3Index);
  set mac3(int value) => this.packet.setUint8(mac3Index, value);

  int get mac2 => this.packet.getUint8(mac2Index);
  set mac2(int value) => this.packet.setUint8(mac2Index, value);

  int get mac1 => this.packet.getUint8(mac1Index);
  set mac1(int value) => this.packet.setUint8(mac1Index, value);

  int get macLo => this.packet.getUint8(macLoIndex);
  set macLo(int value) => this.packet.setUint8(macLoIndex, value);

  List<int> get mac => this.packet.buffer.asUint8List(macHiIndex, 6);
  set mac(List<int> value){
    for(var i = 0; i < 6; i++){
      if(value.length <= i){
        this.packet.setUint8(macHiIndex + i, 0);    
      } else {
        this.packet.setUint8(macHiIndex + i, value[i]);
      }
    }
  }

  List<int> get bindIp => this.packet.buffer.asUint8List(bindIpIndex, bindIpSize);
  set bindIp(List<int> value){
    for(var i = 0; i < bindIpSize; i++){
      if(value.length <= i){
        this.packet.setUint8(bindIpIndex + i, 0);    
      } else {
        this.packet.setUint8(bindIpIndex + i, value[i]);
      }
    }
  }

  int get bindIndex => this.packet.getUint8(bindIndexIndex);
  set bindIndex(int value) => this.packet.setUint8(bindIndexIndex, value);

  int get status2 => this.packet.getUint8(status2Index);
  set status2(int value) => this.packet.setUint8(status2Index, value);

  bool get status2IsSquawking => ((this.status2 & status2IsSquawkingMask) != 0x00);
  set status2IsSquawking(bool value) => (value) ? this.status2 |= status2IsSquawkingMask : this.status2 &= ~status2IsSquawkingMask;

  bool get status2ProtocolSwitchable => ((this.status2 & status2ProtocolSwitchableMask) != 0x00);
  set status2ProtocolSwitchable(bool value) => (value) ? this.status2 |= status2ProtocolSwitchableMask : this.status2 &= ~status2ProtocolSwitchableMask;

  bool get status215BitSupport => ((this.status2 & status215BitSupportMask) != 0x00);
  set status215BitSupport(bool value) => (value) ? this.status2 |= status215BitSupportMask : this.status2 &= ~status215BitSupportMask;

  bool get status2DHCPCapable => ((this.status2 & status2DHCPCapableMask) != 0x00);
  set status2DHCPCapable(bool value) => (value) ? this.status2 |= status2DHCPCapableMask : this.status2 &= ~status2DHCPCapableMask;

  bool get status2IpIsSetManually => ((this.status2 & status2IpIsSetManuallyMask) == 0x00);
  set status2IpIsSetManually(bool value) => (value) ? this.status2 &= ~status2IpIsSetManuallyMask : this.status2 |= status2IpIsSetManuallyMask;

  bool get status2HasWebConfigurationSupport => ((this.status2 & status2HasWebConfigurationSupportMask) != 0x00);
  set status2HasWebConfigurationSupport(bool value) => (value) ? this.status2 |= status2HasWebConfigurationSupportMask : this.status2 &= ~status2HasWebConfigurationSupportMask;

  List<int> get udpPacket => this.packet.buffer.asUint8List();

  @override
  String toString() {
    String string = "***$type***\n";
    string += "Id: " + String.fromCharCodes(this.packet.buffer.asUint8List(0, 7)) + "\n";
    string += "Opcode: 0x" + this.packet.getUint16(ArtnetHelpers.opCodeIndex).toRadixString(16) + "\n";
    string += "Ip Address: " + this.ip[0].toString() + "." + this.ip[1].toString() + "." + this.ip[2].toString() + "." + this.ip[3].toString() + "\n";
    string += "Port: " + this.port.toString() + "\n";
    string += "Version: " + this.versionInfoH.toString() + "." + this.versionInfoL.toString() + "\n";
    string += "Port-Address (Universe): " + (this.netSwitch << 16 | this.subSwitch << 8 | this.swOut[0]).toString() + "\n";
    string += "*** Net Switch: " + this.netSwitch.toString() + "\n";
    string += "*** Sub Switch: " + this.subSwitch.toString() + "\n";
    string += "*** Input Switch:\n";
    string += "*** *** 0: " + this.swIn[0].toString() + "\n";
    string += "*** *** 1: " + this.swIn[1].toString() + "\n";
    string += "*** *** 2: " + this.swIn[2].toString() + "\n";
    string += "*** *** 3: " + this.swIn[3].toString() + "\n";
    string += "*** Output Switch:\n";
    string += "*** *** 0: " + this.swOut[0].toString() + "\n";
    string += "*** *** 1: " + this.swOut[1].toString() + "\n";
    string += "*** *** 2: " + this.swOut[2].toString() + "\n";
    string += "*** *** 3: " + this.swOut[3].toString() + "\n";
    string += "Oem: 0x" + this.oem.toRadixString(16) + "\n";
    string += "Ubea Version: " + this.ubeaVersion.toString() + "\n";
    string += "Status 1: 0x" + this.status1.toRadixString(16) + "\n";
    string += "*** Indicator State: ";
    switch(this.status1IndicatorState){
      case status1IndicatorStateOptionLocate: string += "Locate Mode\n"; break;
      case status1IndicatorStateOptionMute: string += "Mute Mode\n"; break;
      case status1IndicatorStateOptionNormal: string += "Normal Mode\n"; break;
      default: string += "Unkown Mode\n"; break;
    }
    string += "*** Programming Authority: ";
    switch(this.status1ProgrammingAuthority){
      case status1ProgrammingAuthorityOptionNetwork: string += "All or part of Port-Address programmed by network or web browser\n"; break;
      case status1ProgrammingAuthorityOptionPanel: string += "All Port-Address set by front panel controls.\n"; break;
      default: string += "Port-Address programming authority unknown\n"; break;
    }
    string += "*** Firmware Boot: " + ((this.status1FirmwareBoot == status1FirmwareBootOptionRom) ? "ROM boot" : "Normal boot (from flash)") + "\n";
    string += "*** RDM Capable: " + ((this.status1RdmCapable) ? "Capable" : "Not Capable") + "\n";
    string += "*** UBEA Present: " + ((this.status1UbeaPresent) ? "Present" : "Not Present or Currupt") + "\n";
    string += "ESTA Manufacturer Code: 0x" + this.estaMan.toRadixString(16) + "\n";
    string += "Short Name: " + this.shortName + "\n";
    string += "Long Name: " + this.longName + "\n";
    string += "Node Report: " + this.nodeReport + "\n";
    string += "Number of Ports: " + this.numPorts.toString() + "\n";
    string += "Port Types:\n";
    for(var i = 0; i < portTypesSize; i++){
      string += "*** " + i.toString() + ":\n";
      string += "*** *** Artnet Output: " + ((this.getPortTypesInputArtnetAble(i)) ? "Enabled" : "Disabled") + "\n";
      string += "*** *** Artnet Input: " + ((this.getPortTypesOutputArtnetAble(i)) ? "Enabled" : "Disabled") + "\n";
      string += "*** *** Protocol: ";
      switch(this.getPortTypesProtocol(i)){
        case portTypesProtocolOptionDMX: string += "DMX 512\n"; break;
        case portTypesProtocolOptionDMX: string += "MIDI\n"; break;
        case portTypesProtocolOptionDMX: string += "Avab\n"; break;
        case portTypesProtocolOptionDMX: string += "Colortran CMX\n"; break;
        case portTypesProtocolOptionDMX: string += "ADB 62.5\n"; break;
        case portTypesProtocolOptionDMX: string += "Art-Net\n"; break;
        default: string += "Unkown Protocol\n"; break;
      }
    }
    string += "Good Input:\n";
    for(var i = 0; i < goodInputSize; i++){
      string += "*** " + i.toString() + ":\n";
      string += "*** *** Data Received: " + ((this.getGoodInputDataReceived(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX Test Packets 1: " + ((this.getGoodInputIncludesTestPackets1(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX SIP's: " + ((this.getGoodInputIncludesSIPs(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX Test Packets 2: " + ((this.getGoodInputIncludesTestPackets2(i)) ? "True" : "False") + "\n";
      string += "*** *** Input: " + ((this.getGoodInputInputDisable(i)) ? "Disabled" : "Enabled") + "\n";
      string += "*** *** Receive Errors: " + ((this.getGoodInputReceiveErrorDetected(i)) ? "Detected" : "Not Detected") + "\n";
    }
    string += "Good Ouput:\n";
    for(var i = 0; i < goodOutputSize; i++){
      string += "*** " + i.toString() + ":\n";
      string += "*** *** Data Transmitting: " + ((this.getGoodOutputDataTransmiting(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX Test Packets 1: " + ((this.getGoodOutputIncludesTestPackets1(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX SIP's: " + ((this.getGoodOutputIncludesSIPs(i)) ? "True" : "False") + "\n";
      string += "*** *** Channel Includes DMX Test Packets 2: " + ((this.getGoodOutputIncludesTestPackets2(i)) ? "True" : "False") + "\n";
      string += "*** *** Merge: " + ((this.getGoodOutputIsMerging(i)) ? "Disabled" : "Enabled") + "\n";
      string += "*** *** DMX Short: " + ((this.getGoodOutputShortDetected(i)) ? "Detected" : "Not Detected") + "\n";
      string += "*** *** Merge is LTP: " + ((this.getGoodOutputMergeIsLTP(i)) ? "True" : "False") + "\n";
      string += "*** *** Protocol: " + (this.getSACNEnabled() ? "sACN" : "Art-Net") + "\n";
    }
    string += "Video Switch: 0x" + this.swVideo.toRadixString(16) + "\n";
    string += "Macro Switch: 0x" + this.swMacro.toRadixString(16) + "\n";
    string += "Remote Switch: 0x" + this.swRemote.toRadixString(16) + "\n";
    string += "Mac: ";
    string += this.macHi.toRadixString(16) + ".";
    string += this.mac4.toRadixString(16) + ".";
    string += this.mac3.toRadixString(16) + ".";
    string += this.mac2.toRadixString(16) + ".";
    string += this.mac1.toRadixString(16) + ".";
    string += this.macLo.toRadixString(16) + "\n";
    string += "Bind Ip: " + this.bindIp[0].toString() + "." + this.bindIp[1].toString() + "." + this.bindIp[2].toString() + "." + this.bindIp[3].toString() + "\n";
    string += "Bind Index: " + this.bindIndex.toString() + "\n";
    string += "Status 2: 0x" + this.status2.toRadixString(16) + "\n";
    string += "*** Node is Squawking: " + ((this.status2IsSquawking) ? "True" : "False") + "\n";
    string += "*** Node is Protocol Switchable (Art-Net - sACN): " + ((this.status2ProtocolSwitchable) ? "True" : "False") + "\n";
    string += "*** Node Supports 15 Bit Port-Address (Art-Net 3 or 4): " + ((this.status215BitSupport) ? "True" : "False") + "\n";
    string += "*** Node Supports DHCP: " + ((this.status2DHCPCapable) ? "True" : "False") + "\n";
    string += "*** Node's ip is set " + ((this.status2IpIsSetManually) ? "manually" : "by DHCP") + "\n";
    string += "*** Node Supports Web Configurations: " + ((this.status2HasWebConfigurationSupport) ? "True" : "False") + "\n";
    return string;
  }

  String toHexString(){
    String string = "";
    String tempString = "";
    for(var i = 0; i < size; i++){
      tempString = this.udpPacket[i].toRadixString(16).toUpperCase();
      if(tempString.length < 2) tempString = "0" + tempString;
      string += tempString;
    }
    return string;
  }

}

