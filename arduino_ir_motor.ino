#include <SPI.h>
#include <Ethernet.h>

//Ethernet shield stuff
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "thesethings.000webhostapp.com";    // name address for Google (using DNS)

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Variables to measure the speed
unsigned long beginMicros, endMicros;
unsigned long byteCount = 0;
bool printWebData = true;  // set to false for better speed measurement


//Motor definitions
#define IN1  2
#define IN2  3
#define IN3  4
#define IN4  5
int Steps = 1024; //4096 or 768
int reverseSteps = 1024;
int cstep = 0;
int isRotate = 0;
int acstep = 0;

//IR Sensor definitions
int LED = 13; // Use the onboard Uno LED

//This is for closing gate sensor (i.e. when train first passes)
int isObstaclePinFirst = 7;  // This is our input pin
int isObstacleFirst = HIGH;  // HIGH MEANS NO OBSTACLE

//This is for opening gate sensor (i.e. after train has passed)
int isObstaclePinSecond = 6;  // This is our input pin
int isObstacleSecond = HIGH;  // HIGH MEANS NO OBSTACLE

bool hasTrainCome = false;

void setup()
{
  Serial.begin(9600);

  //Motor setup
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 

  //IR sensor setup
  pinMode(LED, OUTPUT);
  pinMode(isObstaclePinFirst, INPUT);
  pinMode(isObstaclePinSecond, INPUT);

  //Ethernet shield setup
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{

  //Check if obstacle is detected. If yes, spin motor in some direction.
  if(!hasTrainCome) {
    isObstacleFirst = digitalRead(isObstaclePinFirst);
    if (isObstacleFirst == LOW)
    {
      Serial.println("OBSTACLE!!, OBSTACLE!! Train has come....");
      hasTrainCome = true;

      //Send ON status to firebase database
      sendDataToFirebase(true);

      //digitalWrite(LED, HIGH);
      //Now spin motor (close the gate)..
      for(int x=0;x<Steps;x++)
      {
        step1();
        //delay(1);
        delayMicroseconds(2500);
      }
      //Serial.println("Boom!!");
      delay(100);
    
      
    }
    else
    {
      //Serial.println("clear");
      digitalWrite(LED, LOW);
    }
  
    delay(200);
  }
  

  //Now if hasTrainCome flag is true, that means train has arrived and we have to check the second sensor.
  if(hasTrainCome) {
    isObstacleSecond = digitalRead(isObstaclePinSecond);
    if (isObstacleSecond == LOW)
    {
      Serial.println("OBSTACLE!!, OBSTACLE!! Train is about to leave....");

      //Send OFF status to firebase database
      sendDataToFirebase(false);
      
      //digitalWrite(LED, HIGH);
      //Now spin the motor in opposite direction (open the gate)..
      for(int x=0;x<reverseSteps;x++)
      {
        stepReverse();
        //delay(1);
        delayMicroseconds(2500);
      }
      //Serial.println("Boom parat!!");
      delay(100);

      hasTrainCome = false;
      
    }
    else
    {
      //Serial.println("clear");
      digitalWrite(LED, LOW);
    }
    
    delay(200);
  }
  
}


//Method to communciate with Firebase
void sendDataToFirebase(bool trainStatus){
  
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    //client.println("GET /firebaseTest.php?crop_name=off HTTP/1.1");
    client.print("GET /firebaseTest.php?");
    
    if(trainStatus) {
      //Send ON
      client.print("crop_name=on");
    }
    if(!trainStatus) {
      client.print("crop_name=off");
    }
    //client.print(trainStatus);
    client.println(" HTTP/1.1");
    client.println("Host: thesethings.000webhostapp.com");
    client.println("Connection: close");
    client.println();

    Serial.println("AFter connections...");
    
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  //beginMicros = micros();
  delay(200);
}

void step1()
{
  //stepp
  switch(cstep)
  {
   case 0:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   case 1:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, HIGH);
   break; 
   case 2:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 3:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 4:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 5:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
     case 6:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 7:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   default:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
  }
   
   cstep=cstep+1;
   if(cstep==8)
     {cstep=0;}
}


//Ata Reverse firnar...
void stepReverse()
{
  //stepp
  switch(acstep)
  {
   case 0:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, HIGH);
   break; 
   case 1:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, HIGH);
     digitalWrite(IN1, HIGH);
   break; 
   case 2:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, HIGH);
     digitalWrite(IN1, LOW);
   break; 
   case 3:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, HIGH);
     digitalWrite(IN2, HIGH);
     digitalWrite(IN1, LOW);
   break; 
   case 4:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, HIGH);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, LOW);
   break; 
   case 5:
     digitalWrite(IN4, HIGH); 
     digitalWrite(IN3, HIGH);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, LOW);
   break; 
     case 6:
     digitalWrite(IN4, HIGH); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, LOW);
   break; 
   case 7:
     digitalWrite(IN4, HIGH); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, HIGH);
   break; 
   default:
     digitalWrite(IN4, LOW); 
     digitalWrite(IN3, LOW);
     digitalWrite(IN2, LOW);
     digitalWrite(IN1, LOW);
   break; 
  }
   
   acstep=acstep+1;
   if(acstep==8)
     {acstep=0;}
}
