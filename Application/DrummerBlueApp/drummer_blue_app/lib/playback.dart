import 'package:flutter/material.dart';

Widget _buildPopupKey(BuildContext context) {
  return AlertDialog(
    title: const Text('Icon Key'),
    content: Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Icon(Icons.wb_sunny, color: Colors.black),
            const Text('  - OFF'),
            Container(),
          ],
        ),
        Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            const Icon(Icons.wb_sunny, color: Colors.green),
            const Text('  - ON '),
            Container(),
          ],
        ),
      ],
    ),
    actions: <Widget>[
      TextButton(
        onPressed: () {
          Navigator.of(context).pop();
        },
        child: const Text('Close'),
      ),
    ],
  );
}

class ChangingColorIcon extends StatefulWidget {
  const ChangingColorIcon({super.key, required this.isOn});

  final bool isOn;

  @override
  State<ChangingColorIcon> createState() => _ChangingColorIconState();
}

class _ChangingColorIconState extends State<ChangingColorIcon> {
  @override
  Widget build(BuildContext context) {
    return IconTheme(
      data: IconThemeData(
        color: widget.isOn ? Colors.green : Colors.black,
      ),
      child: const Icon(Icons.wb_sunny),
    );
  }
}

class Playback extends StatefulWidget {
  const Playback({super.key});

  @override
  State<Playback> createState() => _PlaybackState();
}

class _PlaybackState extends State<Playback> {
  final bool _recording = false;
  final bool _playing = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: <Widget>[
            const Text('Playback Mode'),
            TextButton(
              onPressed: () {
                showDialog(
                  context: context,
                  builder: (BuildContext context) => _buildPopupKey(context),
                );
              },
              child: const Icon(Icons.view_list_rounded, color: Colors.white),
            ),
          ],
        ),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                ChangingColorIcon(
                  isOn: _recording,
                ),
                const SizedBox(
                  width: 20,
                ),
                const Text(
                  'Recording',
                  style: TextStyle(fontSize: 25),
                ),
              ],
            ),
            const SizedBox(
              height: 10,
            ),
            const Text(
              '(Hold down the pedal to record)',
              style: TextStyle(fontSize: 15),
            ),
            const SizedBox(
              height: 40,
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                ChangingColorIcon(
                  isOn: _playing,
                ),
                const SizedBox(
                  width: 20,
                ),
                const Text(
                  'Playing',
                  style: TextStyle(fontSize: 25),
                ),
              ],
            ),
            Container(),
          ],
        ),
      ),
    );
  }
}