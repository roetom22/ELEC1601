
#include <SoftwareSerial.h>   //Software Serial Port
#include <Servo.h>



#define ConnStatus A1
#define DEBUG_ENABLED  1
#define RxD         7
#define TxD         6

#define SERV_RIGHT_PIN 12
#define SERV_LEFT_PIN 13
#define R_SENS 2
#define L_SENS 3

#define R_FAST_F 1700
#define R_FAST_B 1300
#define R_SLOW_B 1400
#define R_SLOW_F 1600
#define R_STOP 1500

#define L_FAST_B 1300
#define L_FAST_F 1700
#define L_SLOW_B 1400
#define L_SLOW_F 1600
#define L_STOP 1500

// start in manual: 1 or auto mode: 0
#define START_MODE 1
int mode = START_MODE;


int shieldPairNumber = 20;
boolean ConnStatusSupported = true;
String slaveNameCmd = "\r\n+STNA=Slave";




SoftwareSerial blueToothSerial(RxD, TxD);

Servo servoLeft;
Servo servoRight;


void setup()
{
  Serial.begin(9600);
  blueToothSerial.begin(38400);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  pinMode(ConnStatus, INPUT);

  if (ConnStatusSupported) Serial.println("Checking Slave-Master connection status.");

  if (ConnStatusSupported && digitalRead(ConnStatus) == 1)
  {
    Serial.println("Already connected to Master - remove USB cable if reboot of Master Bluetooth required.");
  }
  else
  {
    Serial.println("Not connected to Master.");

    setupBlueToothConnection();   // Set up the local (slave) Bluetooth module

    delay(1000);                  // Wait one second and flush the serial buffers
    Serial.flush();
    blueToothSerial.flush();
  }

  servoLeft.attach(SERV_LEFT_PIN);
  servoRight.attach(SERV_RIGHT_PIN);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);

}

void forward() {
  servoLeft.writeMicroseconds(L_FAST_B);
  servoRight.writeMicroseconds(R_FAST_F);
  delay(360);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);
}

void slow_forward() {
  servoLeft.writeMicroseconds(L_SLOW_F);
  servoRight.writeMicroseconds(R_SLOW_B);
  delay(500);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);
}

void back() {
  servoLeft.writeMicroseconds(L_FAST_F);
  servoRight.writeMicroseconds(R_FAST_B);
  delay(250);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);
}

void left() {
  servoLeft.writeMicroseconds(L_FAST_B);
  servoRight.writeMicroseconds(R_FAST_B);
  delay(250);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);
}

void right() {
  servoLeft.writeMicroseconds(L_FAST_F);
  servoRight.writeMicroseconds(R_FAST_F);
  delay(250);
  servoLeft.writeMicroseconds(L_STOP);
  servoRight.writeMicroseconds(R_STOP);
}




void joystick_recv(char direc)
{
  // Cases are ascii values for f,b,l,r
  switch (direc)
  {
    case 'f':
      Serial.println("forward");
      forward();
      break;

    case 'b':
      Serial.println("backward");
      back();
      break;

    case 'l':
      Serial.println("left");
      left();
      break;

    case 'r':
      Serial.println("right");
      right();
      break;

    default:
      //Serial.println("dont move");
      break;
  }
}







void loop()
{

  char recvChar;

  if (mode) {
    //manual

    if (blueToothSerial.available())
    { //check if there's any data sent from the remote bluetooth shield
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);
      joystick_recv(recvChar);
      delay(100);
      //if space character recieved switch to auto mode ie line following
      if (recvChar == ' ') {
        mode = 0;
      }
    }
  }
  else {
    int l_detect, r_detect;
    for (;;) {


      r_detect = digitalRead(R_SENS);
      l_detect = digitalRead(L_SENS);

      if (!l_detect && r_detect) {
        //turn right
        servoLeft.writeMicroseconds(1600);
        servoRight.writeMicroseconds(1600);
        delay(5);
        servoLeft.writeMicroseconds(L_STOP);
        servoRight.writeMicroseconds(R_STOP);
        continue;
      }
      if (l_detect && !r_detect) {
        //turn left
        servoLeft.writeMicroseconds(1400);
        servoRight.writeMicroseconds(1400);
        delay(5);
        servoLeft.writeMicroseconds(L_STOP);
        servoRight.writeMicroseconds(R_STOP);
        continue;
      }
      if (!l_detect && !r_detect) {
        //forward
        servoLeft.writeMicroseconds(L_SLOW_B);
        servoRight.writeMicroseconds(R_SLOW_F);
        delay(10);
        servoLeft.writeMicroseconds(L_STOP);
        servoRight.writeMicroseconds(R_STOP);
        continue;
      }
      if (l_detect && r_detect) {
        //stop
 
        servoLeft.writeMicroseconds(L_FAST_B);
        servoRight.writeMicroseconds(R_FAST_F);
        delay(250);
        servoLeft.writeMicroseconds(L_STOP);
        servoRight.writeMicroseconds(R_STOP);
        //delay(500);
        
        if (digitalRead(L_SENS) && digitalRead(R_SENS)) {
          delay(1000);
  
        servoLeft.writeMicroseconds(1450);
        servoRight.writeMicroseconds(1550);
        delay(400);
        servoLeft.writeMicroseconds(L_STOP);
        servoRight.writeMicroseconds(R_STOP);

              mode = 1;
              blueToothSerial.println("Object reached.");
              break;
        } else {
          continue;
        }
        // swap to manual
        //mode = 1;
        //blueToothSerial.println("End of line reached.");
        //break;
      }

    }



    //assuming line is in middle of two sensors.


  }

}




void setupBlueToothConnection()
{
  Serial.println("Setting up the local (slave) Bluetooth module.");

  slaveNameCmd += shieldPairNumber;
  slaveNameCmd += "\r\n";

  blueToothSerial.print("\r\n+STWMOD=0\r\n");      // Set the Bluetooth to work in slave mode
  blueToothSerial.print(slaveNameCmd);             // Set the Bluetooth name using slaveNameCmd
  blueToothSerial.print("\r\n+STAUTO=0\r\n");      // Auto-connection should be forbidden here
  blueToothSerial.print("\r\n+STOAUT=1\r\n");      // Permit paired device to connect me

  //  print() sets up a transmit/outgoing buffer for the string which is then transmitted via interrupts one character at a time.
  //  This allows the program to keep running, with the transmitting happening in the background.
  //  Serial.flush() does not empty this buffer, instead it pauses the program until all Serial.print()ing is done.
  //  This is useful if there is critical timing mixed in with Serial.print()s.
  //  To clear an "incoming" serial buffer, use while(Serial.available()){Serial.read();}

  blueToothSerial.flush();
  delay(2000);                                     // This delay is required

  blueToothSerial.print("\r\n+INQ=1\r\n");         // Make the slave Bluetooth inquirable

  blueToothSerial.flush();
  delay(2000);                                     // This delay is required

  Serial.println("The slave bluetooth is inquirable!");
}
