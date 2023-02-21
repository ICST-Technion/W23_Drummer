import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

enum Mode{
  idle, calibration, metronome, looper, playback, interactive
}

const command = 0;
const metronomesettings = 1;

const idle = 0;
const calibration = 1;
const metronome = 2;
const looper = 3;
const playback = 4;
const interactive = 5;

const serviceuuid = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const cmduuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple)
      ),
      home: StreamBuilder<BluetoothState>(
          stream: FlutterBlue.instance.state,
          initialData: BluetoothState.unknown,
          builder: (c, snapshot) {
            final state = snapshot.data;
            if (state == BluetoothState.on) {
              return const MyHomePage(title: "Drummer");
            }else{
            return BluetoothOffScreen(state: state);
            }
          }),
    );
  }
}

class BluetoothOffScreen extends StatelessWidget {
  const BluetoothOffScreen({Key? key, this.state}) : super(key: key);

  final BluetoothState? state;

  @override
  Widget build(BuildContext context) {
    var theme = Theme.of(context);
    return Scaffold(
      backgroundColor: theme.backgroundColor,
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: <Widget>[
            const Icon(
              Icons.bluetooth_disabled,
              size: 200.0,
              color: Colors.white54,
            ),
            Text(
              'Bluetooth Adapter is ${state != null ? state.toString().substring(15) : 'not available'}.',
            ),
          ],
        ),
      ),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  Mode modePage = Mode.idle;
  double _metronomeTempoSliderValue = 60;
  double _metronomeBeatsSliderValue = 4 ;
  BluetoothDevice? device;
  BluetoothCharacteristic? Characteristic;
  bool _calibrationDone = false;
  Stream<String> _callibrationstream = (() {
    late final StreamController<String> controller;
    controller = StreamController<String>(
      onListen: () async {
        await Future<void>.delayed(const Duration(seconds: 3));
        controller.add("Hit the drum loudly again!"); 
        await Future<void>.delayed(const Duration(seconds: 3));
        controller.add("Hit the drum softly!");
        await Future<void>.delayed(const Duration(seconds: 3));
        controller.add("Hit the drum soflty again!");
        await Future<void>.delayed(const Duration(seconds: 3));
        await controller.close();
      },
    );
    return controller.stream;
  })();
  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    var theme = Theme.of(context);
    var buttonStyle = theme.textTheme.displayLarge!.copyWith(
      color: theme.colorScheme.primary,
      );

    switch(modePage) {
/*-------------------------------------IDLE-----------------------------------------*/
      case Mode.idle: 
        return Scaffold(
          appBar: AppBar(
            title: Text(widget.title),
          ),
          body: Center(
            // Center is a layout widget. It takes a single child and positions it
            // in the middle of the parent.
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        modePage = Mode.calibration;
                      });
                      Characteristic?.write([command,calibration]);
                    },
                    child: Text('Calibration',style: buttonStyle),              
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        modePage = Mode.metronome;
                      });
                      Characteristic?.write([command,metronome]);
                    },
                    child: Text('Metronome',style: buttonStyle),              
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        modePage = Mode.looper;
                      });
                      Characteristic?.write([command,looper]);
                    },
                    child: Text('Looper',style: buttonStyle),              
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        modePage = Mode.playback;
                      });
                      Characteristic?.write([command,playback]);
                    },
                    child: Text('Playback',style: buttonStyle),              
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(
                    onPressed: () {
                      setState(() {
                        modePage = Mode.interactive;
                      });
                      Characteristic?.write([command,interactive]);
                    },
                    child: Text('Interactive',style: buttonStyle),              
                  ),
                ),
              ],
            ),
          ),
          floatingActionButton: StreamBuilder<bool>(
            stream: FlutterBlue.instance.isScanning,
            initialData: false,
            builder: (c, snapshot) {
              if (snapshot.data!) {
                return FloatingActionButton(
                  onPressed: () => FlutterBlue.instance.stopScan(),
                  child: const Icon(Icons.search),
                );    
              }
              else{
                return device==null ? FloatingActionButton(
                  onPressed:() {
                  FlutterBlue.instance.startScan(timeout: const Duration(seconds: 4));
                  showDialog<String>(
                    context: context,
                    builder: (BuildContext context) => SimpleDialog(
                      title: const Text('Choose device'),
                      children: <Widget>[
                        StreamBuilder<List<ScanResult>>(
                          stream: FlutterBlue.instance.scanResults,
                          initialData: [],
                          builder: (c, snapshot) => Column(
                            children: snapshot.data!
                              .map((result) { 
                                if(result.device.name != ""){
                                return SimpleDialogOption(
                                  child: Text(result.device.name == "" ? "Unknown " : result.device.name),
                                  onPressed: () => Navigator.of(context).push(MaterialPageRoute(builder: (context) {
                                    result.device.connect().whenComplete(() => result.device.discoverServices().then((value) => { 
                                            Characteristic = value.firstWhere((element) => element.uuid.toString().contains(serviceuuid))
                                            .characteristics.firstWhere((element) => element.uuid.toString().contains(cmduuid))
                                          }));
                                  return AlertDialog(
                                    title: Text("Connected to ${result.device.name}"),
                                    actions: <Widget>[
                                      TextButton(
                                        onPressed: () {
                                          Navigator.pop(context, 'Ok');
                                          setState(() {
                                            device = result.device;
                                          });
                                        },
                                        child: const Text('ok'),
                                      ),
                                    ],
                                  );
                                })),
                              );
                              }else{ return const SizedBox.shrink();
                              }
                              }
                            )
                            .toList(),
                          ),
                        ),
                      ],
                    ),
                  );
                },
                tooltip: 'Press to connect a device',
                child: const Icon(Icons.bluetooth),)
                : FloatingActionButton(onPressed: () => Navigator.of(context).push(MaterialPageRoute(builder: (context) {
                                  return AlertDialog(
                                    title: const Text("Device is already connected"),
                                    actions: <Widget>[
                                      TextButton(
                                        onPressed: () {
                                          Navigator.pop(context, 'Ok');
                                        },
                                        child: const Text('ok'),
                                      ),
                                    ],
                                  );
                                })),
                tooltip: 'The device is connected',
                child: const Icon(Icons.bluetooth_connected),
                );
              }
            }
          ),
        );
      case Mode.calibration:
        if(device==null){
          return AlertDialog(
            title: const Text("Device is not connected"),
            content: const Text("please connect to the device and try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else{
        return Scaffold(
          appBar: AppBar(
            title: const Text("Calibration"),
          ),
          body: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                StreamBuilder<String>(
                  stream: _callibrationstream,
                  builder: (c, AsyncSnapshot<String> snapshot) {
                    List<Widget> children;
                    if (snapshot.hasError) {
                      children = <Widget>[
                        const Icon(
                          Icons.error_outline,
                          color: Colors.red,
                          size: 60,
                        ),
                        Padding(
                          padding: const EdgeInsets.only(top: 16),
                          child: Text('Error: ${snapshot.error}'),
                        ),
                        Padding(
                          padding: const EdgeInsets.only(top: 8),
                          child: Text('Stack trace: ${snapshot.stackTrace}'),
                        ),
                      ];
                    } else {
                      switch (snapshot.connectionState) {
                        case ConnectionState.none:
                          children = const <Widget>[
                            Icon(
                              Icons.info,
                              color: Colors.blue,
                              size: 60,
                            ),
                            Padding(
                              padding: EdgeInsets.only(top: 16),
                              child: Text('how did we got here'),
                            ),
                          ];
                          break;
                        case ConnectionState.waiting:
                          children =  <Widget>[
                            const SizedBox(
                              width: 60,
                              height: 60,
                              child: CircularProgressIndicator(),
                            ),
                            Padding(
                              padding: EdgeInsets.only(top: 16),
                              child: Text("Hit the drum loudly!",style: buttonStyle.copyWith(fontSize: 25)),
                            ),
                          ];
                          break;
                        case ConnectionState.active:
                          children = <Widget>[
                            const SizedBox(
                              width: 60,
                              height: 60,
                              child: CircularProgressIndicator(),
                            ),
                            Padding(
                              padding: const EdgeInsets.only(top: 16),
                              child: Text('${snapshot.data}',style: buttonStyle.copyWith(fontSize: 25)),
                            ),
                          ];
                          break;
                        case ConnectionState.done:
                          children = <Widget>[
                            const Icon(
                              Icons.done,
                              color: Colors.blue,
                              size: 60,
                            ),
                            Padding(
                              padding: EdgeInsets.only(top: 16),
                              child: Text("The drummer was calibrated!",style: buttonStyle.copyWith(fontSize: 25)),
                            ),
                          ];
                          break;
                      }
                    }
                    return Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      children: children,
                    );
                  }
                ),
              ],
            ),
          ),          
          floatingActionButtonLocation: FloatingActionButtonLocation.startFloat,
          floatingActionButton: FloatingActionButton(onPressed: (){
            setState(() {
              modePage = Mode.idle;
              _calibrationDone = true;
              _callibrationstream = (() {
                late final StreamController<String> controller;
                controller =  StreamController<String>(
                    onListen: () async {
                      await Future<void>.delayed(const Duration(seconds: 3));
                      controller.add("Hit the drum loudly again!"); 
                      await Future<void>.delayed(const Duration(seconds: 3));
                      controller.add("Hit the drum softly!");
                      await Future<void>.delayed(const Duration(seconds: 3));
                      controller.add("Hit the drum soflty again!");
                      await Future<void>.delayed(const Duration(seconds: 3));
                      await controller.close();
                    },
                  );
                  return controller.stream;
                })();
              });
            Characteristic?.write([command,idle]);
            },
          child: const Icon(Icons.arrow_back_ios_new)
          )
        );  
        }  
      case Mode.metronome:
        if(device==null){
          return AlertDialog(
            title: const Text("Device is not connected"),
            content: const Text("please connect to the device and try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else{
        return Scaffold(
          appBar: AppBar(
            title: const Text('Metronome Mode'),
          ),
          floatingActionButtonLocation: FloatingActionButtonLocation.startFloat,
          floatingActionButton: FloatingActionButton(onPressed: (){
            setState(() {
              modePage = Mode.idle;
            });
            Characteristic?.write([command,idle]);
            },
            child: const Icon(Icons.arrow_back_ios_new)
          ),
          body: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                const Text(
                  'Choose a Tempo:', //should be in beats per minute
                  style: TextStyle(fontSize: 25),
                ),
                Slider(
                  value: _metronomeTempoSliderValue,
                  max: 240,
                  min: 10,
                  divisions: 23,
                  label: _metronomeTempoSliderValue.round().toString()+" BPM".toString(),
                  onChanged: (double value) {
                    setState(() {                      
                      _metronomeTempoSliderValue = value;
                    });
                  },
                ),
                const SizedBox(height: 40,),
                const Text('Beats per Measure',style: TextStyle(fontSize: 25)),
                Slider(
                  value: _metronomeBeatsSliderValue,
                  max: 8,
                  min: 2,
                  divisions: 6,
                  label: _metronomeBeatsSliderValue.round().toString(),
                  onChanged: (value) {
                    setState(() {
                      _metronomeBeatsSliderValue = value;
                    });
                  },
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: ElevatedButton(onPressed: () {
                    Characteristic?.write([metronomesettings,_metronomeTempoSliderValue.toInt(),_metronomeBeatsSliderValue.toInt()]);
                  }, child: const Text("set")),
                )
              ],
            ),
          )
        );
        }
      case Mode.looper:
        if(device==null){
          return AlertDialog(
            title: const Text("Device is not connected"),
            content: const Text("please connect to the device and try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else if(!_calibrationDone){
          return AlertDialog(
            title: const Text("Device hasn't been calibrated"),
            content: const Text("please go to calibration, follow the instructions and then try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else{
        return Scaffold(
          appBar: AppBar(
            title: const Text("Looper mode")
          ),
          floatingActionButtonLocation: FloatingActionButtonLocation.startFloat,
          floatingActionButton: FloatingActionButton(onPressed: (){
            setState(() {
              modePage = Mode.idle;
            });
            Characteristic?.write([command,idle]);
            },
          child: const Icon(Icons.arrow_back_ios_new)
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  const SizedBox(
                    width: 20,
                  ),
                  Container(),
                ],
              ),
              const SizedBox(
                height: 10,
              ),
              Text(
                'Hold down the pedal to record',
                style: buttonStyle.copyWith(fontSize: 25),
              ),
              const Padding(
                padding: const EdgeInsets.all(8.0),
                child: const LinearProgressIndicator(),
              ),
            ],
          ),
        ),
      );
        }
      case Mode.playback:
        if(device==null){
          return AlertDialog(
            title: const Text("Device is not connected"),
            content: const Text("please connect to the device and try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else if(!_calibrationDone){
          return AlertDialog(
            title: const Text("Device hasn't been calibrated"),
            content: const Text("please go to calibration, follow the instructions and then try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else{
        return Scaffold(
          appBar: AppBar(
            title: const Text("Playback")
          ),
          floatingActionButtonLocation: FloatingActionButtonLocation.startFloat,
          floatingActionButton: FloatingActionButton(onPressed: (){
            setState(() {
              modePage = Mode.idle;
            });
            Characteristic?.write([command,idle]);
          },
          child: const Icon(Icons.arrow_back_ios_new)
          ),
          body: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
              const SizedBox(height: 10),
                            Text(
                'Hold down the pedal to record',
                style: buttonStyle.copyWith(fontSize: 25),
              ),
              const Padding(
                padding: EdgeInsets.all(8.0),
                child: LinearProgressIndicator(),
              ),
              const SizedBox(height: 40),
            Container(),
          ],
        ),
      ),
    );
        }
      case Mode.interactive:
        if(device==null){
          return AlertDialog(
            title: const Text("Device is not connected"),
            content: const Text("please connect to the device and try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else if(!_calibrationDone){
          return AlertDialog(
            title: const Text("Device hasn't been calibrated"),
            content: const Text("please go to calibration, follow the instructions and then try again"),
            actions: <Widget>[
              TextButton(
                onPressed: () { 
                  setState(() {
                    modePage = Mode.idle;
                  });
                  },
                child: const Text('ok'),
              ),
            ],
          );
        }else{
        return Scaffold(
          appBar: AppBar(
            title: const Text("Interactive mode"),
          ),
           body: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
              const SizedBox(height: 10),
                            Text(
                'Hold down the pedal to record',
                style: buttonStyle.copyWith(fontSize: 25),
              ),
              const Padding(
                padding: EdgeInsets.all(8.0),
                child: LinearProgressIndicator(),
              ),
              const SizedBox(height: 40),
            Container(),
          ],
        ),
      ),
        floatingActionButtonLocation: FloatingActionButtonLocation.startFloat,
        floatingActionButton: FloatingActionButton(onPressed: (){
          setState(() {
            modePage = Mode.idle;
          });
          Characteristic?.write([command,idle]);
          },
          child: const Icon(Icons.arrow_back_ios_new)
        ),
      );
      }
      default: 
        throw UnimplementedError('not implemented yet');   
    }
  }
}

class BigCard extends StatelessWidget {
  const BigCard({
    Key? key,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var theme = Theme.of(context);
    var style = theme.textTheme.displaySmall!.copyWith(
      color: theme.colorScheme.onPrimary,
      );

    return Card(
      color: theme.primaryColor,
      child: Padding(
        padding: const EdgeInsets.all(20),
        child: Text(
          'You have pushed the button this many times:',
          style: style,
        ),
      ),
    );
  }
}
