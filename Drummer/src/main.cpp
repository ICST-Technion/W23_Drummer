#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
#include <time.h>
#include "NeuralNetwork.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define CMD_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BUFFER_SIZE 100
#define MIC_PIN 14 
#define STRONG_SOLENOID_PIN 26
#define WEAK_SOLENOID_PIN 33
#define COMMAND 0
#define METRONOMESETTINGS 1
#define N 500
#define T 4000
#define TIME_INTERVAL 8

int MicVolume = 0;
int flag = 1;
int counter = 0;
int arrayLength = 200;
uint32_t initial_time;
int num_hits=0;
int in1 = 32;                    //This is the output pin on the Arduino
int in2 = 33;
int in3 = 25;                    //This is the output pin on the Arduino
int in4 = 26;
int softThreshold = 0;
int loudThreshold = 0;
unsigned long initial_hit_time = 0;
bool calibrationDone = false;
int mic = 14;
int pedal = 27;
bool record = true;
unsigned long music_started_playing = 0;
bool ever_started_recording = false;
unsigned long beats_per_minute = 60;//this is going to be input from the app
double doub_metronome_delay_between_hits = ((double)(60*1000)/beats_per_minute);
unsigned long metronome_delay_between_hits = (unsigned long)doub_metronome_delay_between_hits;
int beats_per_measure = 4;//also input from the app
unsigned long start_time_for_looper = 0;
NeuralNetwork *nn;
int arr_nn[N];

bool deviceConnected = false;
bool newValue = false;

unsigned long first_first_response[7][2] = {{0, 1},{495, 1},{1394, 1},{1874, 0},{2110, 0},{2346, 0},{2724, 1}};
unsigned long first_second_response[8][2] = {{0, 0},{335, 0},{632, 0},{1635, 1},{2168, 0},{2454, 0},{2751, 0},{3307, 1}};
unsigned long first_third_response[10][2] = {{0, 1},{230, 0},{463, 1},{885, 1},{1103, 0},{1341, 1},{1744, 1},{1986, 0},{2257, 1},{2609, 1}};
unsigned long second_first_response[7][2] = {{0, 1},{588, 0},{810, 1},{1166, 0},{1535, 1},{1942, 1},{2345, 0}};
unsigned long second_second_response[5][2] = {{0, 1},{726, 1},{1472, 1},{1833, 1},{2154, 0}};
unsigned long second_third_response[8][2] = {{0, 1},{218, 0},{460, 1},{870, 1},{1085, 0},{1324, 1},{1761, 1},{2410, 1}};
unsigned long third_first_response[7][2] = {{0, 0},{349, 0},{698, 0},{1027, 0},{1415, 1},{1787, 0},{2126, 1}};
unsigned long third_second_response[11][2] = {{0, 1},{396, 1},{853, 0},{1090, 1},{1352, 0},{1594, 1},{1841, 0},{2097, 1},{2352, 0},{2618, 1},{2855, 1}};
unsigned long third_third_response[8][2] = {{0, 1},{360, 0},{717, 1},{1073, 1},{1451, 1},{1811, 1},{1811, 0},{2189, 1}};
unsigned long fourth_first_response[11][2] = {{0, 1},{216, 0},{459, 1},{838, 1},{1066, 0},{1315, 1},{1699, 1},{1947, 0},{2197, 0},{2434, 0},{2703, 1}};
unsigned long fourth_second_response[8][2] = {{0, 0},{407, 1},{746, 0},{1132, 0},{1456, 1},{1814, 0},{2159, 0},{2512, 1}};
unsigned long fourth_third_response[6][2] = {{0, 1},{597, 0},{1165, 1},{1577, 0},{1945, 0},{2315, 1}};
unsigned long first_class[8][2] = {{0, 1},{575, 1},{1110, 0},{1379, 0},{1651, 0},{2193, 1},{2723, 1},{3290, 1}};
unsigned long second_class[9][2] = {{0, 1},{564, 0},{806, 1},{1161, 0},{1563, 1},{1807, 0},{2184, 0},{2404, 1},{2784, 0}};
unsigned long third_class[6][2] = {{0, 1},{386, 1},{758, 0},{1759, 1},{2154, 0},{2386, 1}};
unsigned long fourth_class[7][2] = {{0, 1},{364, 0},{698, 0},{1050, 1},{1269, 0},{1734, 0},{2105, 1}};

class Solenoids{
  public:
    void strongHit(){
        digitalWrite(STRONG_SOLENOID_PIN, HIGH);
        delay(50);
        digitalWrite(STRONG_SOLENOID_PIN, LOW);
    }
    void weakHit(){
        digitalWrite(WEAK_SOLENOID_PIN, HIGH);
        delay(50);
        digitalWrite(WEAK_SOLENOID_PIN, LOW);
    }
};

auto mySolenoid = new Solenoids();
/////////////////////////PLAYBACK FUNCS/////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////

class myDrummerBuff{
  int sample[BUFFER_SIZE];
  unsigned int sample_time_stamp[BUFFER_SIZE];
  int record_index = 0;
  public:
    void takeSample(){
      this->sample[this->record_index] = analogRead(MIC_PIN);
      this->sample_time_stamp[this->record_index] = millis();
      this->record_index++;
    }  
    void cleanBuffer(){this->record_index = 0;};
};

typedef enum {
  IDLE,
  CALIBRATION,
  METRONOME,
  LOOPER,
  PLAYBACK,
  INTERACTIVE,
  IDLE_NEW  
}command;


command cmd = IDLE;
unsigned long temp_time;
unsigned long metronome_interval = 2000;
auto drummer_buffer = new myDrummerBuff();


class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic){
    // cmd = command(*(pCharacteristic->getData()));
    uint8_t* input = pCharacteristic->getData();
    int lenght = pCharacteristic->getLength();
    if(*input==COMMAND){
      cmd = command(*(input+1));
    }else if (*input==METRONOMESETTINGS){
      beats_per_minute=*(input+1);
      beats_per_measure=*(input+2);
      Serial.print("beats per minute = ");
      Serial.print(beats_per_minute);
      Serial.print("beats per measure = ");
      Serial.print(beats_per_measure);
    }
    doub_metronome_delay_between_hits = ((double)(60)*1000/beats_per_minute);
    metronome_delay_between_hits = (unsigned long)doub_metronome_delay_between_hits;
  }
};


struct node {
    unsigned long hit_time;
    int strength;
    struct node *next;
};
struct node *listHead = NULL;

struct node* createNode(unsigned long hit_time, int strength) {
    struct node *newNode = (struct node*) malloc(sizeof(struct node));
    if (newNode == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    newNode->hit_time = hit_time;
    newNode->strength = strength;
    newNode->next = NULL;
    return newNode;
}

void appendNode(struct node **head, unsigned long hit_time, int strength) {
    struct node *newNode = createNode(hit_time, strength);
    if (*head == NULL) {
        *head = newNode;
        return;
    }
    struct node *lastNode = *head;
    while (lastNode->next != NULL) {
        lastNode = lastNode->next;
    }
    lastNode->next = newNode;
}

struct node* insert_to_end(struct node **head, unsigned long hit_time, int strength) {
    struct node *new_node = (struct node*)malloc(sizeof(struct node));
    new_node->hit_time = hit_time;
    new_node->strength = strength;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return new_node;
    }

    struct node *temp = *head;
    while (temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = new_node;
    return new_node;
}

struct node* insert_after_node(struct node *prev_node, unsigned long hit_time, int strength) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL");
        return NULL;
    }

    struct node *new_node = (struct node*)malloc(sizeof(struct node));
    new_node->hit_time = hit_time;
    new_node->strength = strength;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
    return new_node;
}

int hit_exists(struct node *head, unsigned long hit_time) {
  struct node *temp = head;
  while (temp != NULL) {
    Serial.println("my hit time:");
    Serial.println(hit_time);
    Serial.println("checking against");
    Serial.println(temp->hit_time);
    Serial.println("Result:");
    Serial.println(abs((int)temp->hit_time - (int)hit_time));
    if (abs((int)temp->hit_time - (int)hit_time)<300) {
      Serial.println("hit existsssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss");
      return 1;
    }
    temp = temp->next;
  }
  return 0;
}

struct node* insert_in_order(struct node **head, unsigned long hit_time, int strength) {
  struct node *new_node = (struct node*)malloc(sizeof(struct node));
  new_node->hit_time = hit_time;
  new_node->strength = strength;

  if (*head == NULL || (*head)->hit_time >= hit_time) {
    new_node->next = *head;
    *head = new_node;
    return new_node;
  }

  struct node *temp = *head;
  while (temp->next != NULL && temp->next->hit_time < hit_time) {
    temp = temp->next;
  }
  new_node->next = temp->next;
  temp->next = new_node;
  return new_node;
}

struct node* createResponseList(int beatClass){
    struct node *responseHead = NULL;
    int j=0;
    int k = 0;

    
    switch(beatClass){
        case 1:
            j=(rand() % 3) + 1;
            break;
        case 2:
            j=(rand() % 3) + 4;
            break;
        case 3:
            j=(rand() % 3) + 7;
            break;
        case 4:
            j=(rand() % 3) + 10;
            break;
        default:
            j=1000;
            break;
    }
    Serial.println("j value:");
    Serial.println(j);
    unsigned long (*response)[2] = NULL;
    switch(j){
        case 1:
            k = sizeof(first_first_response)/8;
            response = first_first_response;
            break;
        case 2:
            k = sizeof(first_second_response)/8;
            response = first_second_response;
            break;
        case 3:
            k = sizeof(first_third_response)/8;
            response = first_third_response;
            break;
        case 4:
            k = sizeof(second_first_response)/8;
            response = second_first_response;
            break;
        case 5:
            k = sizeof(second_second_response)/8;
            response = second_second_response;
            break;
        case 6:
            k = sizeof(second_third_response)/8;
            response = second_third_response;
            break;
        case 7:
            k = sizeof(third_first_response)/8;
            response = third_first_response;
            break;
        case 8:
            k = sizeof(third_second_response)/8;
            response = third_second_response;
            break;
        case 9:
            k = sizeof(third_third_response)/8;
            response = third_third_response;
            break;
        case 10:
            k = sizeof(fourth_first_response)/8;
            response = fourth_first_response;
            break;
        case 11:
            k = sizeof(fourth_second_response)/8;
            response = fourth_second_response;
            break;
        case 12:
            k = sizeof(fourth_third_response)/8;
            response = fourth_third_response;
            break;
    }
    for (int i = 0; i < k; i++) {
        unsigned long hit_time = response[i][0];
        int strength = response[i][1];
        appendNode(&responseHead, hit_time, strength);
    }
    return responseHead;

}

void print_list(struct node *head) {
    struct node *temp = head;
    while (temp != NULL) {
        Serial.println("hit time: ");
        Serial.println(temp->hit_time);
        Serial.println("strength: ");
        Serial.println(temp->strength);
        temp = temp->next;
    }
}

void clear_list(struct node **head) {
  struct node *current = *head;
  struct node *next;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  *head = NULL;
}

void loudHit(long hit_time, command myMode){
  Serial.println("num hits:");
  Serial.println(num_hits);
  if(num_hits == 0){
    initial_hit_time = hit_time;
    music_started_playing = initial_hit_time;
  }
  else if (myMode==LOOPER && music_started_playing!=0){
    initial_hit_time = music_started_playing;
  }
  
  counter = 0;
  flag = 0;
  Serial.println("time since first playback hit:");
  Serial.println(hit_time-initial_hit_time);
  //strengths[i]=1;
  if(myMode==PLAYBACK){
    appendNode(&listHead,hit_time-initial_hit_time,1); 
  }
  else{//if looper mode, we want to check if the hit already exists in the linkedlist
     Serial.println("inserting: ");
     Serial.println(hit_time-initial_hit_time);
     Serial.println("music started playing:");
     Serial.println(music_started_playing);
     if (!hit_exists(listHead,hit_time-initial_hit_time)){
        insert_in_order(&listHead,hit_time-initial_hit_time,1);
     }
  }
  num_hits++;
}

void softHit(long hit_time, command myMode){
  if(num_hits == 0){
    initial_hit_time = hit_time;
    music_started_playing = initial_hit_time;
  }
  else if (myMode==LOOPER && music_started_playing!=0){
    initial_hit_time = music_started_playing;
  }
  counter = 0;
  flag = 0;
  Serial.println("softest-------------------");
  //strengths[i]=1;
  if(myMode==PLAYBACK){
    appendNode(&listHead,hit_time-initial_hit_time,0); 
  }
  else{//if looper mode, we want to check if the hit already exists in the linkedlist
     if (!hit_exists(listHead,hit_time-initial_hit_time)){
        insert_in_order(&listHead,hit_time-initial_hit_time,0);
     }
  }
  num_hits++;
}

void hit_detection(command myMode){
  if (counter > 200){
    flag = 1;
  }
  if (counter > 2000000)
    counter = 0;
  if (MicVolume >loudThreshold &&  flag == 1){
    Serial.println("Loudest-------------------");
    Serial.println(MicVolume);
    loudHit(millis(),myMode);
  }
    
  else if ((MicVolume > softThreshold) &&  flag==1){
    Serial.println(MicVolume);
    //if we entered the soft if, sometimes it was just a soft sound before the loud thud. Therefore we want to check if a loud sounds
    //came immeadiately after the soft one
    bool loud_hit_happened = false;
    long hit_time = millis();
    for (int k=0; k<20; k++){
      delay(1);
      MicVolume = (analogRead(mic));
      //Serial.println(MicVolume);
      if (MicVolume>loudThreshold){
        //replace soft hit with loud hit
        Serial.println("Loudest----------------------");
        loudHit(millis(),myMode);
        loud_hit_happened=true;
  break;
      }
    }
    if (!loud_hit_happened){
    softHit(hit_time,myMode);
    }
  }
  //Serial.println(MicVolume);
  delay(1);
  counter++;
}


void solenoid_player() {
  Serial.println("................................................................................");
  music_started_playing = millis();
  digitalWrite(in1, HIGH);
  digitalWrite(in4, HIGH);
  delay(100);
  struct node *temp = listHead;
  Serial.println("here!!!!!");
  while (temp != NULL) {
    if(temp->strength == 0){
        Serial.println("Producing soft");
        digitalWrite(in4, LOW);
        delay(50);
        digitalWrite(in4, HIGH);
      }
    else{
        Serial.println("Producing loud");
        digitalWrite(in1, LOW);
        delay(50);
        digitalWrite(in1, HIGH);
      }
    record = !digitalRead(pedal);
    struct node* next_hit = temp->next;
    if(temp->next != NULL){
      //going to want to do hit detection for this duration
      unsigned long curr_time = millis();
      Serial.println("waiting this many seconds:");
      Serial.println(next_hit->hit_time - temp->hit_time);
      while((millis() - curr_time) < (next_hit->hit_time - temp->hit_time)){
        record = !digitalRead(pedal);
        if (cmd == LOOPER && record){ //then we want to keep recording while hitting
        MicVolume = (analogRead(mic));
        hit_detection(LOOPER);
        }
      }


      /*
      if (current_mode == LOOPER && record){ //then we want to keep recording while hitting
        Serial.println("waiting this many seconds:");
        Serial.println(next_hit->hit_time - temp->hit_time);
        while((millis() - curr_time) < (next_hit->hit_time - temp->hit_time)){
          
        }
      }
      else{//then we don't want to keep recording while hitting
        delay(next_hit->hit_time - temp->hit_time);  
      }*/
    }
    temp = next_hit;
  }
}


void calibrationMode(){
  int loudest1 = 0;
  int loudest2 = 0;
  int softest1 = 0;
  int softest2 = 0;
  Serial.println("Welcome to calibration mode!");
  Serial.println("Hit the drum loudly!");
  for(int k=0; k<3000; k++){//meaning we wait three seconds for intput
    MicVolume = (analogRead(mic));
    if (MicVolume > loudest1){
      loudest1 = MicVolume;
    }
    delay(1);
  }
  Serial.print("Max value: ");
  Serial.println(loudest1);
  Serial.println("Great! Hit the drum loudly again!");
  for(int k=0; k<3000; k++){//meaning we wait three seconds for intput
    MicVolume = (analogRead(mic));
    if (MicVolume > loudest2){
      loudest2 = MicVolume;
    }
    delay(1);
  }
  Serial.print("Max value: ");
  Serial.println(loudest2);
  Serial.println("Hit the drum softly!");
  for(int k=0; k<3000; k++){//meaning we wait three seconds for intput
    MicVolume = (analogRead(mic));
    if (MicVolume > softest1){
      softest1 = MicVolume;
    }
    delay(1);
  }
  Serial.print("Max value: ");
  Serial.println(softest1);
  Serial.println("Great! Hit the drum softly again!");
  for(int k=0; k<3000; k++){//meaning we wait three seconds for intput
    MicVolume = (analogRead(mic));
    if (MicVolume > softest2){
      softest2 = MicVolume;
    }
    delay(1);
  }
  Serial.print("Max value: ");
  Serial.println(softest2);
  loudThreshold = ((loudest1+loudest2)/2) - 50; 
  softThreshold = ((softest1+softest2)/2) - 75; 
  Serial.print("loud threshold: ");
  Serial.println(loudThreshold);
  Serial.print("soft threshold: ");
  Serial.println(softThreshold);
  
  
}



void metronomeMode(){
  int counter = 0;
  int static_beats_per_measure=0;
  while(cmd==METRONOME){
    Serial.println(cmd);
    digitalWrite(in1, LOW);
    delay(50);
    digitalWrite(in1, HIGH);
    delay(metronome_delay_between_hits);
    counter++;
    static_beats_per_measure = beats_per_measure;
    while(counter!=static_beats_per_measure){
      digitalWrite(in4, LOW);
      delay(50);
      digitalWrite(in4, HIGH);
      counter++;
      delay(metronome_delay_between_hits);
    }
    counter=0;    
  }
  Serial.println("here");
}




void fillArray(int* arr, unsigned long our_hit_time, int our_hit_strength){
    int element_index = our_hit_time/TIME_INTERVAL;
    for(int i=-2; i<=2; i++){
        if((element_index==0 && i==-2) || (element_index==0 && i==-1)|| (element_index==1 && i==-2)||
        (element_index==N-1 && i==1)||(element_index==N-1 && i==2)||(element_index==N-2 && i==2)){
          continue;
        }
        arr[element_index+i] = our_hit_strength+1;
    }
}





void setup() {
  srand(time(NULL));
  Serial.begin(115200);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, HIGH);
  initial_time = millis();
  Serial.println("Starting BLE work!");

  BLEDevice::init("Drummer  ");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(CMD_UUID);
  BLECharacteristic *cmdCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  cmdCharacteristic->setValue("Drummer mode selector");
  cmdCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(CMD_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
  nn = new NeuralNetwork();
}


void loop() {

  if(deviceConnected){
    //while (cmd==CALIBRATION&&!calibrationDone)
    Serial.println("IDLE");
    while (cmd==CALIBRATION)//need to change this back to CALIBRATION!!!!!
    {
      calibrationMode();
      cmd=IDLE;      
    }
    while (cmd==METRONOME){
      Serial.println("METRONOME mode");
      metronomeMode();
    }
    while (cmd==PLAYBACK)
    {
      //Serial.println("PLAYBACK mode");
      record = !digitalRead(pedal);
      MicVolume = (analogRead(mic)); //change the number in the analogRead(HERE) based on which pin in connected to OUT
      //Serial.println("PLAYBACK mode");
      if(record){
        //clear_list(&listHead);//every time record button is hit, we record a whole new beat
        //Serial.println("hit detection");
        hit_detection(PLAYBACK); // the difference betweeen the hit detection codes is whether to check before inserting into linkedlist or not
        if(num_hits>=1){
          ever_started_recording = true;
        }
      }
      else{
        delay(1000);
        //print_list(listHead);
        if (ever_started_recording){
          Serial.println("Calling solenoid player----------------------------------------------");
          solenoid_player();
          clear_list(&listHead);
          num_hits=0;
          ever_started_recording=false;
          initial_hit_time=0;//not sure about this one
          
        }
      }

    }
    while (cmd==LOOPER)
    {
      //Serial.println("ever started recording:");
      //Serial.println(ever_started_recording);
      if(ever_started_recording==false){
        num_hits=0;
        music_started_playing=1;
      }
      MicVolume = (analogRead(mic)); //change the number in the analogRead(HERE) based on which pin in connected to OUT
      record = !digitalRead(pedal);
      //Serial.println("LOOPER mode");
      if(record){
        //Serial.println("music started playing:");
        //Serial.println(music_started_playing);
        hit_detection(LOOPER); // the difference betweeen the hit detection codes is whether to check before inserting into linkedlist or not
        ever_started_recording = true;
      }
      else{
        if (ever_started_recording){
          while(cmd==LOOPER){
            delay(1000);
            Serial.println("Calling solenoid player----------------------------------------------");
            solenoid_player();
          }
          clear_list(&listHead);
          num_hits=0;
          ever_started_recording=false;
          initial_hit_time=0;//not sure about this one

        }
      }
    }
    while (cmd==INTERACTIVE)
    {
      Serial.println("INTERACTIVE mode");
      record = !digitalRead(pedal);
      MicVolume = (analogRead(mic)); //change the number in the analogRead(HERE) based on which pin in connected to OUT
      if(record){
        hit_detection(PLAYBACK); // the difference betweeen the hit detection codes is whether to check before inserting into linkedlist or not
        if(num_hits>=1){
          ever_started_recording = true;
        }
      }
      else{
        delay(1000);
        //print_list(listHead);
        if (ever_started_recording){
          //here we send the linkedlist to the model
          struct node* temp = listHead;
          for (int index=0; index<N; index++){
            arr_nn[index]=0;
          }
          while(temp!=NULL){
            fillArray(arr_nn, temp->hit_time,temp->strength);
            Serial.println(temp->hit_time);
            Serial.println(temp->strength);
            temp = temp->next;
          }
          for (int index=0; index<N; index++){
            nn->getInputBuffer()[index] = arr_nn[index];
          }
          clear_list(&listHead);
          temp=NULL;
          //and then we get back a class number, and clear the currect list
          int result = nn->predict();
          Serial.println("result");
          Serial.println(result);
          
          //now we create the response, and then call the solenoid player
          listHead = createResponseList(result);
          solenoid_player();
          clear_list(&listHead);
          num_hits=0;
          ever_started_recording=false;
          initial_hit_time=0;//not sure about this one
        }
      }
    }  
  }
  //delay(1000);
}