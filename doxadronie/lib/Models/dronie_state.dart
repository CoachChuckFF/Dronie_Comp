

import 'dart:math';
import 'dart:typed_data';
import 'dart:ui';
import 'package:doxadronie/Controllers/reserves.dart';
import 'package:doxadronie/Models/artnet.dart';
import 'package:doxadronie/Models/mac.dart';
import 'artnet.dart';

enum DronieMode {
  incognito,
  debug,
  spoon,
}

class DronieCommand {

}

class DronieState {

  static const int ledCount = 5;

  static const int manSize = ArtnetPollReplyPacket.shortNameSize;
  static const int snSize = ArtnetPollReplyPacket.longNameSize;
  static const int ledSize = 3;
  static const int ownerSize = 47;

  static const int convertFromPollReplyIndex = ArtnetPollReplyPacket.shortNameIndex;
  static const int dronieManIndex = 0;
  static const int dronieSNIndex = dronieManIndex + manSize;
  static const int dronieModeIndex = dronieSNIndex +snSize;
  static const int dronieMotorIndex = dronieModeIndex + 1;
  static const int dronieLed0Index = dronieMotorIndex + 1;
  static const int dronieLed1Index = dronieLed0Index + ledSize;
  static const int dronieLed2Index = dronieLed1Index + ledSize;
  static const int dronieLed3Index = dronieLed2Index + ledSize;
  static const int dronieLed4Index = dronieLed3Index + ledSize;
  static const int dronieOwnerIndex = dronieLed4Index + ledSize;

  static const packetSize = (
    ArtnetPollReplyPacket.shortNameSize + 
    ArtnetPollReplyPacket.longNameSize + 
    ArtnetPollReplyPacket.nodeReportSize
  );

  final ByteData rawData = ByteData(packetSize);
  final Mac mac;

  DronieState(this.mac, [List<int> packet = const []]){
    if(packet.isNotEmpty){
      for(var i = 0; i < min(rawData.lengthInBytes, packet.length); i++){
        this.rawData.setUint8(i, packet[i]);
      }
    }
  }

  factory DronieState.fromPoll(Mac mac, ArtnetPollReplyPacket packet){
    return DronieState(
      mac,
      packet.udpPacket.sublist(convertFromPollReplyIndex)
    );
  }

  String get man => blizzardRemoveZeros(
    String.fromCharCodes(
      this.rawData.buffer.asUint8List(
        dronieManIndex, 
        manSize
      )
    )
  );

  String get sn => blizzardRemoveZeros(
    String.fromCharCodes(
      this.rawData.buffer.asUint8List(
        dronieSNIndex, 
        snSize
      )
    )
  );

  String get owner => blizzardRemoveZeros(
    String.fromCharCodes(
      this.rawData.buffer.asUint8List(
        dronieOwnerIndex, 
        ownerSize
      )
    )
  );

  DronieMode get mode => DronieMode.values[min(rawData.getUint8(dronieModeIndex), DronieMode.values.length)];

  bool get motorRunning => rawData.getUint8(dronieModeIndex) != 0x00;

  List<Color> get leds => List.generate(ledCount, (index) => getLed(rawData, dronieLed0Index, index));
  
  Color get led0 => getLed(rawData, dronieLed0Index, 0);
  Color get led1 => getLed(rawData, dronieLed0Index, 1);
  Color get led2 => getLed(rawData, dronieLed0Index, 2);
  Color get led3 => getLed(rawData, dronieLed0Index, 3);
  Color get led4 => getLed(rawData, dronieLed0Index, 4);

  static Color getLed(ByteData data, int startingIndex, int ledIndex){
    ledIndex = min(ledIndex, ledCount - 1);
    int red = data.getUint8(startingIndex + 0 + (ledIndex * ledSize));
    int green = data.getUint8(startingIndex + 1 + (ledIndex * ledSize));
    int blue = data.getUint8(startingIndex + 2 + (ledIndex * ledSize));
    return Color.fromARGB(0xFF, red, green, blue);
  }

  static void setLed(ByteData data, int startingIndex, int ledIndex, Color color){
    ledIndex = min(ledIndex, ledCount - 1);

    data.setUint8(startingIndex + 0 + (ledIndex * ledSize), color.red);
    data.setUint8(startingIndex + 1 + (ledIndex * ledSize), color.green);
    data.setUint8(startingIndex + 2 + (ledIndex * ledSize), color.blue);
  }
}

// // -------------------- DRONIE -----------------

// #define DRONIE_OP_POLL 0xCC01
// #define DRONIE_OP_POLL_REPLY 0xCC02
// #define DRONIE_OP_COMMAND 0xCC03

// #define DRONIE_NOP 0x00

// #define DRONIE_SET_MODE 0x01 
// #define DRONIE_MODE_INCOGNITO 0
// #define DRONIE_MODE_DEBUG 1
// #define DRONIE_MODE_SPOON 2

// #define DRONIE_SET_LED 0x02

// #define DRONIE_SET_OWNER 0x03

// #define DRONIE_DOT 0x10
// #define DRONIE_DASH 0x11

// #define DRONIE_TOUCH_MOTOR 0x20

// #define DRONIE_FACTORY_RESET 0xFF

// typedef struct DronieCommandPacket {
//   uint8_t _id[8];  
//   uint16_t _opcode;
//   uint8_t _command;
//   uint8_t _value;
//   color_t _color_0;
//   color_t _color_1;
//   color_t _color_2;
//   color_t _color_3;
//   color_t _color_4;
//   uint8_t _owner[47];
// }__attribute__((packed)) DronieCommandPacket;

// typedef struct DroniePollPacket {
//   uint8_t _id[8];  
//   uint16_t _opcode;
//   uint16_t _protocol_version; //14
//   uint8_t _talk_to_me;
//   uint8_t _priority;
// }__attribute__((packed)) DroniePollPacket;

//   uint8_t _man[18]; //Talon Corp
//   uint8_t _sn[64]; //SN
//   union {
//     struct {
//       uint8_t _mode; //Ingocnito, //Debug, //Spoon
//       uint8_t _motor; //on/off
//       color_t _led0;
//       color_t _led1;
//       color_t _led2;
//       color_t _led3;
//       color_t _led4;
//       uint8_t _owner[47];
//     };
//     uint8_t val[64];
//   } _node_report;


 

// /*----------------------- Functions ------------------------------------------*/

// uint8_t init_artnet_manager(void);
// uint8_t start_artnet(void);
// void stop_artnet(void);

// void tick_artnet(void);

// uint8_t change_artnet_merge(uint8_t merge);
// uint8_t get_artnet_merge(void);
// uint8_t change_artnet_net(uint8_t net);
// uint8_t get_artnet_net(void);
// uint8_t change_artnet_subnet(uint8_t subnet);
// uint8_t get_artnet_subnet(void);
// uint8_t change_artnet_universe(uint8_t universe);
// uint8_t get_artnet_universe(void);
// uint16_t build_artnet_universe(void);

// /*Internal*/
// void receiveArtnet(void *arg,
//                   struct udp_pcb *pcb,
//                   struct pbuf *p,
//                   const ip_addr_t *addr,
//                   u16_t port);
                
// uint8_t pushToQue(struct pbuf *packet, const ip_addr_t *address);
// bool emptyQue();
// ArtQueType* popFromQue();

// void handlePackets();

// uint8_t parseDronieCommandPacket(DronieCommandPacket* packet);
// uint8_t createDronieReplyPacket(void);


// uint8_t parseArtnetDataPacket(ArtnetDataPacket* packet);
// uint8_t parseArtnetAddressPacket(ArtnetAddressPacket* packet, const ip_addr_t *address);
// uint8_t parseArtnetProgPacket(ArtnetProgPacket* packet);
// uint8_t parseArtnetCommandPacket(ArtnetCommandPacket* packet);
// uint8_t parseArtnetFirmwareMasterPacket(ArtnetFirmwareMasterPacket* packet);

// uint8_t sendPacket(const ip_addr_t *address, uint16_t size, uint8_t* data);

// uint8_t createArtnetPollReplyPacket(void);
// uint8_t createArtnetProgReplyPacket(void);
// void createArtnetTODReplyPacket();
// void createArtnetRDMReplyPacket();
// void createArtnetFirmwareReplyPacket(void);
// void createArtnetCommandReplyPacket(void);
// //command and firmware packets are populated in the parsing of them

// uint8_t checkMergeNSync(const ip_addr_t* address, uint8_t* dmx);

// uint8_t artToDMX(uint16_t slots, uint8_t* dmx);
// const char * opCodeToString(uint16_t opCode);

// #ifdef __cplusplus
// }
// #endif

// #endif