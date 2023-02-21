import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

class Metronome extends StatefulWidget {
  const Metronome({super.key});

  @override
  State<Metronome> createState() => _MetronomeState();
}

class _MetronomeState extends State<Metronome> {
  double _tempoSliderValue = 0;
  double _beatsSliderValue = 1;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: const Text('Metronome Mode'),
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
                value: _tempoSliderValue,
                max: 240,
                divisions: 24,
                label: _tempoSliderValue.round().toString(),
                onChanged: (double value) {
                  setState(() {
                    _tempoSliderValue = value;
                  });
                },
              ),
              const SizedBox(
                height: 40,
              ),
              const Text(
                'Beats per Measure',
                style: TextStyle(fontSize: 25),
              ),
              Slider(
                value: _beatsSliderValue,
                max: 12,
                divisions: 12,
                label: _beatsSliderValue.round().toString(),
                onChanged: (double value) {
                  setState(() {
                    _beatsSliderValue = value;
                  });
                },
              ),
            ],
          ),
        ));
  }
}
