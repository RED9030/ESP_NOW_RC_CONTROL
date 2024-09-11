/*
  MODDER: @RED9030
*/
/*
  Title: ESPNOW ESP8266 CAR TX
  Este sketch permite la transmición de dos datos mediante la comunicación ESP-NOW
  Carro control remoto usando protocolo espNOW, 4 motores tracción trasera/delantera y giros.

  HardWare: ESP8266
*/

/*
 *****************************************************
 *    LIBRERIAS
 *****************************************************
*/
#include <espnow.h>
#include <ESP8266WiFi.h>

/*
 *****************************************************
 *    VARIABLES
 *****************************************************
*/
#define X_AXIS_PIN 32
#define Y_AXIS_PIN 33
#define SWITCH_PIN 25

// RECEIVER MAC Address
uint8_t receiverMacAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};  //Here mac for Receptor (RX)

struct PacketData
{
  byte xAxisValue;
  byte yAxisValue;
  byte switchPressed;
};
PacketData data;

/*
 *****************************************************
 *    FUNCIONES
 *****************************************************
*/
//This function is used to map 0-4095 joystick value to 0-254. hence 127 is the center value which we send.
//It also adjust the deadband in joystick.
//Jotstick values range from 0-4095. But its center value is not always 2047. It is little different.
//So we need to add some deadband to center value. in our case 1800-2200. Any value in this deadband range is mapped to center 127.
int mapAndAdjustJoystickDeadBandValues(int value, bool reverse)
{
  if (value >= 2200)
  {
    value = map(value, 2200, 4095, 127, 254);
  }
  else if (value <= 1800)
  {
    value = map(value, 1800, 0, 127, 0);  
  }
  else
  {
    value = 127;
  }

  if (reverse)
  {
    value = 254 - value;
  }
  return value;
}

// callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.print("\r\nLast Packet Send Status:\t ");
  if(sendStatus == 0){
    Serial.println("Message sent");
  }else{
    Serial.println("Message failed");
    }
}

/*
 *****************************************************
 *    INICIO
 *****************************************************
*/
void setup() 
{
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else
  {
    Serial.println("Succes: Initialized ESP-NOW");
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);  //ESP_NOW_ROLE_CONTROLLER is 6
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  //ESP8266 esp_now_add_peer(uint8 mac_addr, uint8 role, uint8 channel, uint8 key, uint8 key_len)
  bool peerInfo = esp_now_add_peer(receiverMacAddress,ESP_NOW_ROLE_CONTROLLER,0,NULL,0);
  
  // Add peer        
  if (peerInfo != 0)
  {
    Serial.println("Failed to add peer");
    return;
  }
  else
  {
    Serial.println("Succes: Added peer");
  } 

  pinMode(SWITCH_PIN, INPUT_PULLUP);   
}

/*
 *****************************************************
 *    REPETICIÓN
 *****************************************************
*/
void loop() 
{
  data.xAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(X_AXIS_PIN), false);
  data.yAxisValue = mapAndAdjustJoystickDeadBandValues(analogRead(Y_AXIS_PIN), false);  
  data.switchPressed = false; 

  if (digitalRead(SWITCH_PIN) == LOW)
  {
    data.switchPressed = true;
  }

  bool result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  //esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  if (result == 0) 
  {
    Serial.println("Sent with success");
  }else{
    Serial.println("Error sending the data");
  }
  
  if (data.switchPressed == true)
  {
    delay(500);
  }
  else
  {
    delay(50);
  }
}
