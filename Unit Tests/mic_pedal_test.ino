int MicVolume = 0;
int pedalPin = 27;
int micPin = 14;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  int pedalState = digitalRead(pedalPin);
  if(pedalState == 0){
    MicVolume = (analogRead(micPin)); //change the number in the analogRead(HERE) based on which pin in connected to OUT
    Serial.println(MicVolume);
  }
  delay(5);
}
