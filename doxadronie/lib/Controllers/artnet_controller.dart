// /*
// * Blizzard Pro. LLC.
// * All Rights Reserved.
// *
// * Author: Christian Krueger
// * Date: 7/26/19
// *
// */
// //Extras
// import 'dart:async';
// import 'dart:io';
// import 'package:at_full/Controllers/Helpers/at_full_helpers.dart';
// import 'package:at_full/Controllers/Helpers/reserves.dart';
// import 'package:at_full/Models/Configuration/globals.dart';
// import 'package:at_full/Models/Data/af_state.dart';
// import 'package:at_full/Models/Data/artnet.dart';
// import 'package:at_full/Models/Data/mac.dart';
// import 'package:at_full/Models/Data/reducer_actions.dart';
// import 'package:at_full/Models/Data/request_packet.dart';
// import 'package:redux/redux.dart';

// import 'artnet_server.dart';

// class ArtnetController{

//   ArtnetServer server;

//   ArtnetController(this.store){
//     server = ArtnetServer(_onPoll, _handlePacket);
//   }

//   Future<int> startController(){
//     return server.startServer();
//   }

//   void dispose(){
//     requestList.clear();
//     this.server.stopServer();
//   }

//   //decrement all devices, if a device is almost lost try to get it back
//   void _onPoll(){
//     bool getThePaddlesOut = false;
//     Mac dyingDevice;
    
//     store.state.deviceStrength.forEach((device, strength){
//       if(strength < 1){
//         getThePaddlesOut = true;
//         dyingDevice = device;
//       }
//     });

//     if(getThePaddlesOut){
//       server.sendBurst(
//         13, 
//         ArtnetPollPacket().udpPacket,
//         ip: store.state.deviceAddress[dyingDevice],
//         description: "Resuscitation"
//       );
//     } 
    
//     store.dispatch(DecrementAllDevices());
//   }

//   void _handlePacket(Datagram gram){
//     Mac device;

//     if(requestList.length > 0){
//       device = _checkRequest(gram);
//     }

//     LogHelper.log(
//       LogHelper.controller,
//        "Incoming ${artnetOpCodeToString(artnetGetOpCode(gram.data))} Packet from ${gram.address.toString()}",
//        LogHelper.verbose,
//     );

//     switch(artnetGetOpCode(gram.data)){
//       case ArtnetPollReplyPacket.opCode:
//         ArtnetPollReplyPacket info = ArtnetPollReplyPacket(gram.data);
//         if(device == null){
//           device = Mac(info.mac);
//         }

//         if(store.state.devices.contains(
//           device
//         )){
//           store.dispatch(
//             UpdateCurrentDevice(
//               device,
//               gram.address,
//               info
//             )
//           );
//         } else {
//           store.dispatch(
//             AddNewDevice(
//               device,
//               gram.address,
//               info
//             )
//           );
//         }
//       break;
//       case ArtnetIpProgReplyPacket.opCode:
//         if(device == null){
//           device = _getMac(store, gram.address);
//           if(device == null) return;
//         }

//         store.dispatch(
//           UpdateCurrentDeviceNet(
//             device,
//             gram.address,
//             ArtnetIpProgReplyPacket(gram.data)
//           )
//         );
//       break;
//       case ArtnetCommandPacket.opCode:
//         if(device == null){
//           device = _getMac(store, gram.address);
//           if(device == null) return;
//         }

//         ArtnetCommandPacket packet = ArtnetCommandPacket(gram.data);
//         // Check if if is Player File Update
//         if(packet.estaMan != 0xFFFF){
//           store.dispatch(
//             RecievedShowPlaybackFileInfo(device, packet.data)
//           );
//         } else if(store.state.deviceInfo[device] != null && store.state.deviceInfo[device].isBlizzardDevice && store.state.deviceInfo[device].blizzardType == BlizzardFixtureTypeIds.hemisphere) {
//           // is Hemisphere
//           // print(packet.toHemisphereString());
//         }
//       break;
//       case ArtnetFirmwareReplyPacket.opCode:
//         otaPacketCallback(gram.data);
//       break;
//       case ArtnetFirmwareMasterPacket.opCode:
//         ArtnetFirmwareMasterPacket packet = ArtnetFirmwareMasterPacket(gram.data);

//         if(packet.blockType == ArtnetFirmwareMasterPacket.blockTypeOptionReadDMX){
//           Mac device;

//           store.state.deviceAddress.forEach((k, v){
//             if(v == gram.address){
//               device = k;
//             }
//           });

//           if(device != null){
//             store.dispatch(
//               UpdateCurrentDeviceData(device, ArtnetDataPacket()..dmx = packet.dmxData)
//             );
//           }
//         }
//       break;
//       default:
//         return; //unknown packet
//     }
//   }

//   Mac _getMac(Store<AppState> store, InternetAddress gramAddress){
//     int i = -1;
//     if(store.state.deviceAddress.values.firstWhere(
//       (address){
//         i++;
//         return address == gramAddress;
//       }, 
//       orElse: ()=> null
//     ) == null){
//       LogHelper.log(LogHelper.error, "No mac found with address $gramAddress", LogHelper.crash);
//       return null;
//     }

//     if(i >= store.state.deviceAddress.length){
//       LogHelper.log(LogHelper.error, "Index was too high [$i] for device length "
//         "${store.state.deviceAddress.length} - trying to get Mac", LogHelper.crash);
//       return null;
//     }

//     return store.state.deviceAddress.keys.toList()[i];
//   }

//   Mac _checkRequest(Datagram gram){
//     RequestPacket packet;
    
//     packet = requestList.firstWhere((pack){

//       if(pack.expectedOpCode == ArtnetPollReplyPacket.opCode && 
//         artnetGetOpCode(gram.data) == ArtnetPollReplyPacket.opCode)
//       { //contains the Mac
//         if(pack.device == null){
//           LogHelper.log(LogHelper.error, "If expecting a Poll Reply - mac cannot be null", LogHelper.crash);
//           return null;
//         }
//         return Mac(ArtnetPollReplyPacket(gram.data).mac) == pack.device;
//       } else if(pack.expectedOpCode == ArtnetRDMPacket.opCode && 
//         artnetGetOpCode(gram.data) == ArtnetRDMPacket.opCode){
        
//         if(pack.expectedPid != null){
//           return ArtnetRDMPacket(gram.data).rdm.pid == pack.expectedPid;
//         } else {
//           return artnetGetOpCode(gram.data) == pack.expectedOpCode;
//         }

//       } else {
//         return artnetGetOpCode(gram.data) == pack.expectedOpCode;
//       }
//     }, orElse: (){
//       return null;
//     });

//     if(packet != null){
//       Mac returnVal = packet.device;
//       packet.onSuccess(gram);
//       requestList.remove(packet);
//       return returnVal;
//     }
//     return null;
//   }

//   void _tickRequest(RequestPacket packet){

//     if(requestList.contains(packet)){
//       if(packet.resendCount-- <= 0){

//         packet.onFailure();
//         requestList.remove(packet);
//         return;
//       }

//       server.sendPacket(
//         packet.packetNeedingResponse, 
//         ip: packet.address, 
//         description: packet.description
//       );
//       Timer(packet.resendFrequency, () => _tickRequest(packet));
//     }
//   } 

//   void addRequest(RequestPacket packet){
//     requestList.add(packet);
//     _tickRequest(packet);
//   }
// }