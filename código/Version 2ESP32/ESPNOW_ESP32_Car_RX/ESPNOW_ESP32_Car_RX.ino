/*
  MODDER: @RED9030
*/
/*
  Title: ESPNOW ESP32 CAR RX
  Este sketch permite la recepción de los datos mediante la comunicación ESP-NOW
  Carro control remoto usando protocolo espNOW 2.4GHz, 4 motores tracción trasera/delantera y giros.
  
  HardWare: ESP32
*/

/*
 *****************************************************
 *    LIBRERIAS
 *****************************************************
*/
#include <esp_now.h>
#include <WiFi.h>

/*
 *****************************************************
 *    VARIABLES
 *****************************************************
*/
//Right motor
int enableRightMotor=15;    //Enable Motor B 
int rightMotorPin1=19;      //In 3
int rightMotorPin2=18;      //In 4
//Left motor
int enableLeftMotor=5;     //Enable Motor A
int leftMotorPin1=22;      //In 1
int leftMotorPin2=21;      //In 2

#define MAX_MOTOR_SPEED 200

const int PWMFreq = 1000;                   // 1 KHz 
const int PWMResolution = 8;                // Resolutions bits "8bits"
const int rightMotorPWMSpeedChannel = 4;    //PWM channel Right motor
const int leftMotorPWMSpeedChannel = 5;     //PWM channel Left motor

#define SIGNAL_TIMEOUT 1000  // This is signal timeout in milli seconds. We will reset the data if no signal
unsigned long lastRecvTime = 0;

struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchPressed;
};
PacketData receiverData;

bool throttleAndSteeringMode = false;

//Lights
#define LDright 4  //Directional right
#define LDleft  2  //Directional Left
#define LStop   6  //Stop Light
#define LNight  7  //Diurn Light
#define Horn 34    //Horn

/*
 *****************************************************
 *    FUNCIONES
 *****************************************************
*/
// Callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  if (len == 0)
  {
    return;
  }
  memcpy(&receiverData, incomingData, sizeof(receiverData));
  String inputData ;
  inputData = inputData + "values " + receiverData.xAxisValue + "  " + receiverData.yAxisValue + "  " + receiverData.switchPressed;
  Serial.println(inputData);
  if (receiverData.switchPressed == true)
  {
    if (throttleAndSteeringMode == false)
    {
      throttleAndSteeringMode = true;
    }
    else
    {
      throttleAndSteeringMode = false;
    }
  }

  if (throttleAndSteeringMode)
  {
    throttleAndSteeringMovements();
  }
  else
  {
    simpleMovements();
  }

  lastRecvTime = millis();   
}

//Simple motor movements
void simpleMovements()
{
  if (receiverData.yAxisValue <= 75)       //Move car Forward
  {
    rotateMotor(MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    digitalWrite(LDright,LOW);
    digitalWrite(LDleft,LOW);
    digitalWrite(LStop,LOW);
  }
  else if (receiverData.yAxisValue >= 175)   //Move car Backward
  {
    rotateMotor(-MAX_MOTOR_SPEED, -MAX_MOTOR_SPEED);
    digitalWrite(LStop,HIGH);
    digitalWrite(LDright,LOW);
    digitalWrite(LDleft,LOW);
  }
  else if (receiverData.xAxisValue >= 175)  //Move car Right
  {
    rotateMotor(-MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    digitalWrite(LStop,LOW);
    blinkingLed(LDright);
  }
  else if (receiverData.xAxisValue <= 75)   //Move car Left
  {
    rotateMotor(MAX_MOTOR_SPEED, -MAX_MOTOR_SPEED);
    digitalWrite(LStop,LOW);
    blinkingLed(LDleft);
  }
  else                                      //Stop the car
  {
    rotateMotor(0, 0);
    digitalWrite(LDright,LOW);
    digitalWrite(LDleft,LOW);
    digitalWrite(LStop,LOW);
    digitalWrite(LNight,LOW);
    
  }   
}

//Direction and aceleration Movements
void throttleAndSteeringMovements()
{
  int throttle = map( receiverData.yAxisValue, 254, 0, -255, 255);
  int steering = map( receiverData.xAxisValue, 0, 254, -255, 255);  
  int motorDirection = 1;
  
  if (throttle < 0)       //Move car backward
  {
    motorDirection = -1;    
  }

  int rightMotorSpeed, leftMotorSpeed;
  rightMotorSpeed =  abs(throttle) - steering;
  leftMotorSpeed =  abs(throttle) + steering;
  rightMotorSpeed = constrain(rightMotorSpeed, 0, 255);
  leftMotorSpeed = constrain(leftMotorSpeed, 0, 255);

  rotateMotor(rightMotorSpeed * motorDirection, leftMotorSpeed * motorDirection);
}

//Rotación del vehículo
void rotateMotor(int rightMotorSpeed, int leftMotorSpeed)
{
  if (rightMotorSpeed < 0)
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,HIGH);    
  }
  else if (rightMotorSpeed > 0)
  {
    digitalWrite(rightMotorPin1,HIGH);
    digitalWrite(rightMotorPin2,LOW);      
  }
  else
  {
    digitalWrite(rightMotorPin1,LOW);
    digitalWrite(rightMotorPin2,LOW);      
  }
  
  if (leftMotorSpeed < 0)
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,HIGH);    
  }
  else if (leftMotorSpeed > 0)
  {
    digitalWrite(leftMotorPin1,HIGH);
    digitalWrite(leftMotorPin2,LOW);      
  }
  else
  {
    digitalWrite(leftMotorPin1,LOW);
    digitalWrite(leftMotorPin2,LOW);      
  } 

  ledcWrite(rightMotorPWMSpeedChannel, abs(rightMotorSpeed));
  ledcWrite(leftMotorPWMSpeedChannel, abs(leftMotorSpeed));    
}

//Configuraciones de los pines
void setUpPinModes()
{
  //Right Motors
  pinMode(enableRightMotor,OUTPUT);
  pinMode(rightMotorPin1,OUTPUT);
  pinMode(rightMotorPin2,OUTPUT);
  //Left Motors  
  pinMode(enableLeftMotor,OUTPUT);
  pinMode(leftMotorPin1,OUTPUT);
  pinMode(leftMotorPin2,OUTPUT);
  //Lights & Horn
  pinMode(LDright,OUTPUT);
  pinMode(Left,OUTPUT);
  pinMode(LStop,OUTPUT);
  pinMode(LNight,OUTPUT);
  pinMode(Horn,OUTPUT);

  //Set up PWM for motor speed
  ledcSetup(rightMotorPWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(leftMotorPWMSpeedChannel, PWMFreq, PWMResolution);  
  ledcAttachPin(enableRightMotor, rightMotorPWMSpeedChannel);
  ledcAttachPin(enableLeftMotor, leftMotorPWMSpeedChannel); 
  
  rotateMotor(0, 0);
}

//Función para hacer brillar un led
blinkingLed(const int Pin)
{
    digitalWrite(Pin,HIGH);
    delay(500);
    digitalWrite(Pin,LOW);
    delay(500);
  }

/*
 *****************************************************
 *    INICIO
 *****************************************************
*/
void setup() 
{
  setUpPinModes();
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}

/*
 *****************************************************
 *    REPETICIÓN
 *****************************************************
*/
void loop() 
{
  //Check Signal lost.
  unsigned long now = millis();
  if ( now - lastRecvTime > SIGNAL_TIMEOUT ) 
  {
    rotateMotor(0, 0);
  }
}
