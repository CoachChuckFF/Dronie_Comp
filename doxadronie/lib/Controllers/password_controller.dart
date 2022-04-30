/*
* Blizzard Pro. LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 7/26/19
*
*/
//Extras
import 'dart:async';
import 'package:shared_preferences/shared_preferences.dart';

class PasswordController{
  static const PASSWORD_KEY = "password";
  static Future<String> getPassword() async { return (await SharedPreferences.getInstance()).getString(PASSWORD_KEY) ?? ""; }
  static Future<bool> setPassword(String password) async { return (await SharedPreferences.getInstance()).setString(PASSWORD_KEY, password); }
  static Future<bool> clearPassword() async { return setPassword(""); }
}


