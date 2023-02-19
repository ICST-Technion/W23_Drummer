int MicVolume = 0;
int micPin = 14;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  MicVolume = (analogRead(micPin));
  Serial.println(MicVolume);
  delay(5);
}
