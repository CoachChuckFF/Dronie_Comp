/*
* Christian Krueger Health LLC.
* All Rights Reserved.
*
* Author: Christian Krueger
* Date: 4.15.20
*
*/
import 'package:flutter/widgets.dart';
import 'package:flutter_bloc/flutter_bloc.dart';


//--------------- Offset --------------------------------------

abstract class OffsetEvent{}
class OffsetUpdateEvent implements OffsetEvent{
  final Offset value;

  OffsetUpdateEvent(this.value);
} 

class OffsetBLoC extends Bloc<OffsetEvent, Offset>{
  final Offset _start;

  OffsetBLoC([this._start = Offset.zero]) : super(Offset.zero);

  Offset get initialState => _start;


  @override
  Stream<Offset> mapEventToState(OffsetEvent event) async* {
    if(event is OffsetUpdateEvent){
      yield event.value;
    }
  }

  dispose()  async{
    await close();
  }
}

//--------------- Bool --------------------------------------

abstract class BoolEvent{}
class BoolUpdateEvent implements BoolEvent{
  final bool value;

  BoolUpdateEvent(this.value);
} 

class BoolToggleEvent implements BoolEvent{

  BoolToggleEvent();
} 

class BoolBLoC extends Bloc<BoolEvent, bool>{
  final bool _start;

  BoolBLoC([this._start = false]) : super(false);

  bool get initialState => _start;


  @override
  Stream<bool> mapEventToState(BoolEvent event) async* {
    if(event is BoolUpdateEvent){

      yield event.value;
    } else if(event is BoolToggleEvent){
      yield !state;
    }
  }

  dispose()  async{
    await close();
  }
}

//----------------- Enum ---------------------------------------

abstract class EnumEvent{}
class EnumUpdateEvent implements EnumEvent{
  final value;

  EnumUpdateEvent(this.value);
} 

class EnumBLoC extends Bloc<EnumEvent, dynamic>{
  final dynamic _start;

  EnumBLoC([this._start = 0]) : super(_start ?? 0);

  int get initialState => _start;

  @override
  Stream<dynamic> mapEventToState(EnumEvent event) async* {
    if(event is EnumUpdateEvent){
      yield event.value;
    }
  }

  dispose() async{
    await close();
  }
}

//----------------- Int ---------------------------------------

abstract class IntEvent{}
class IntUpdateEvent implements IntEvent{
  final int value;

  IntUpdateEvent(this.value);
} 

class IntIncrementEvent implements IntEvent{

  IntIncrementEvent();
} 

class IntDecrementEvent implements IntEvent{

  IntDecrementEvent();
} 

class IntBLoC extends Bloc<IntEvent, int>{
  final int _start;

  IntBLoC([this._start = 0]) : super(0);

  int get initialState => _start;

  @override
  Stream<int> mapEventToState(IntEvent event) async* {
    if(event is IntUpdateEvent){
      yield event.value;
    } else if(event is IntIncrementEvent){
      yield state + 1;
    } else if(event is IntDecrementEvent){
      yield state - 1;
    }
  }

  dispose()  async{
    await close();
  }
}

//----------------- Double ---------------------------------------

abstract class DoubleEvent{}
class DoubleUpdateEvent implements DoubleEvent{
  final double value;

  DoubleUpdateEvent(this.value);
} 

class DoubleBLoC extends Bloc<DoubleEvent, double>{
  final double _start;

  DoubleBLoC([this._start = 0.0]) : super(0.0);

  double get initialState => _start;

  @override
  Stream<double> mapEventToState(DoubleEvent event) async* {
    if(event is DoubleUpdateEvent){
      yield event.value;
    }
  }

  dispose()  async{
    await close();
  }
}

//----------------- String ---------------------------------------

abstract class StringEvent{}
class StringUpdateEvent implements StringEvent{
  final String value;

  StringUpdateEvent(this.value);
} 

class StringBLoC extends Bloc<StringEvent, String>{
  final String _start;

  StringBLoC([this._start = ""]) : super('');

  String get initialState => _start;

  @override
  Stream<String> mapEventToState(StringEvent event) async* {
    if(event is StringUpdateEvent){
      yield event.value;
    }
  }

  dispose()  async{
    await close();
  }
}

//--------------- Search State --------------------------------------

abstract class SearchStateEvent{}
class SearchStateSetEvent implements SearchStateEvent{
  final String search;

  SearchStateSetEvent(
    this.search,
  );
} 

class SearchStateSetTagsEvent implements SearchStateEvent{
  final List<String> tags;

  SearchStateSetTagsEvent(
    this.tags,
  );
} 

class SearchStateToggleEvent implements SearchStateEvent{
  final String tag;

  SearchStateToggleEvent(
    this.tag,
  );
} 

class SearchClearStateEvent implements SearchStateEvent{} 

class SearchStateBLoC extends Bloc<SearchStateEvent, Map<String, dynamic>>{
  SearchStateBLoC() : super({
    "Search" : "All",
  });


  
  Map<String, dynamic> get initialState => {
    "Search" : "All",
  };


  @override
  Stream<Map<String, dynamic>> mapEventToState(SearchStateEvent event) async* {
    if(event is SearchStateSetEvent){
      state["Search"] = event.search;
      yield Map.from(state);
    } else if(event is SearchStateToggleEvent){
      if(state.keys.contains(event.tag)){
        state[event.tag] = !state[event.tag];
      }
      yield Map.from(state);
    } else if(event is SearchStateSetTagsEvent){
      Map<String, dynamic> map = Map<String, dynamic>();
      state.forEach((k, v){
        map[k] = v;
      });
      event.tags.forEach((k){
        map[k] = false;
      });
      yield Map.from(map);
    } else if(event is SearchClearStateEvent){
      state.keys.forEach((key){
        if(key != "Search"){
          state[key] = false;
        } else {
          state[key] = "All";
        }
      });
      yield Map.from(state);
    }
  }

  void dispose()  async{
    await close();
  }

  bool canOpen(){
    return !(state.containsValue(true));
  }

}
