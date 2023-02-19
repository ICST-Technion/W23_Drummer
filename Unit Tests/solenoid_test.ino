int in1 = 32;
int in2 = 33;
int in3 = 25; 
int in4 = 26;  

void setup() 
{
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

void loop() 
{
//The same in, set at low, then moved to high then low
  digitalWrite(in1, HIGH);
  delay(50);
  digitalWrite(in1, LOW);
  delay(2000);
  
////different ins for the same solenoid
//  digitalWrite(in1, HIGH);
//  delay(50);
//  digitalWrite(in1, LOW);
//  delay(1000);
//  digitalWrite(in2, HIGH);
//  delay(50);
//  digitalWrite(in2, LOW);

////two at a time:
//  digitalWrite(in1, HIGH);
//  digitalWrite(in3, HIGH);
//  delay(50);
//  digitalWrite(in1, LOW);
//  digitalWrite(in3, LOW);
//  delay(1000);
//  digitalWrite(in2, HIGH);
//  digitalWrite(in3, HIGH); //can use in4 here too
//  delay(50);
//  digitalWrite(in2, LOW);
//  digitalWrite(in3, LOW); //can use in4 here too
}
