//green connected to pin 2 of the digital pins
//red connected to 3.3v
//black connected to GND
int pedalPin = 14;

void setup() {
  Serial.begin(115200);
  pinMode(pedalPin, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  int pedalState = digitalRead(pedalPin); //is 1 when pedal is not pressed, is 0 when pedal is pressed
  Serial.println(pedalState);
  delay(1);  // delay in between reads for stability
}
