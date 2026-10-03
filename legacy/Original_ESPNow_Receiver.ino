/************************************************************************
  Dylan Bollone
  IEEE Tetris Project
  ESP32 on Tetris-Machine board
  Last Updated: 11/19/2025
************************************************************************/

/************************************************************************
  Following code is for the ESP32 connected to the micros from an old 
  Micros-II project (LED Coffee Table Featuring Tetris). The purpose of
  This ESP32 is to recieve data from another ESP32 board, and decode
  information into information that the micros from the orignial Tertis
  Table can use.

  Quick note: Tetris Table and Tetris Machine refer to the same
              inanimate object
************************************************************************/

/************************************************************************
  Give credit to: Rui Santos
  Used their resources to set up working ESPNOW communication
  Complete project details at https://randomnerdtutorials.com
************************************************************************/

// load in wifi and esp now libraries
#include <WiFi.h>
#include <esp_now.h>

// Initialize vars to gpio pins
// These are the pins used to output data to the Tetris Table
#define A 13
#define B 12
#define Select 32
#define Start 33
#define Up 25
#define Down 26
#define Left 27
#define Right 14

// time delay const amount among input delays
//   There is a level of uncertainty on how fast the two ESP32s can communicate to change button data
//   because I am unsure how long the Tetris Table needs before button press data can be changed
const int d_time = 50;

// struct for incoming data - same as outgoing data from other esp32
//   the examples from Santos used structs, I am unsure if structs are neccessary for this use-case
//   but I did not want to break anything. Also if I ever wish to send more data, a struct would have 
//   to be implemented anyway
typedef struct data
{
  char button;
} data;
// name a var of that type - to store the incoming data
data inData;

/************************************************************************
  Function: OnDataRecieve
  Purpose: This function gets called whenever the ESP32 recognizes that
            another ESP32 is trying to send data.
            Copies the data into inData, and decodes inData.
  Returns: Pulses output pin related to specific values of inData
            Communicates button presses to Tetris Machine micros
  Last Modified: 11/14/2025
************************************************************************/
void OnDataRecieve(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  memcpy(&inData, incomingData, sizeof(inData));
  // Serial.print("Bytes received: ");
  // Serial.println(len);
  Serial.print("button: ");
  Serial.println(inData.button);

  // code for output to tetris machine logic
  if(inData.button == 'A')
  {
    // if A was pressed, pulse A output pin low
    digitalWrite(A, LOW);
    delay(d_time);
    digitalWrite(A, HIGH);
  }
  if(inData.button == 'B')
  {
    // if B was pressed, pulse B output pin low
    digitalWrite(B, LOW);
    delay(d_time);
    digitalWrite(B, HIGH);  
  }
  if(inData.button == 's')
  {
    // if select was pressed, pulse start output pin low
    digitalWrite(Select, LOW);
    delay(d_time);
    digitalWrite(Select, HIGH);
  }
  // I may have mixed up the start and select buttons, could possibly have to swap 's' <--> 'S'
  if(inData.button == 'S')
  {
    // if start was pressed, pulse select output pin low
    digitalWrite(Start, LOW);
    delay(d_time);
    digitalWrite(Start, HIGH);
  }
  if(inData.button == 'U')
  {
    // if up was pressed, pulse up output pin low
    digitalWrite(Up, LOW);
    delay(d_time);
    digitalWrite(Up, HIGH);
  }
  if(inData.button == 'D')
  {
    // if down was pressed, pulse down output pin low
    digitalWrite(Down, LOW);
    delay(d_time);
    digitalWrite(Down, HIGH);
  }
  if(inData.button == 'L')
  {
    // if left was pressed, pulse left output pin low
    digitalWrite(Left, LOW);
    delay(d_time);
    digitalWrite(Left, HIGH);
  }
  if(inData.button == 'R')
  {
    // if right was pressed, pulse right output pin low
    digitalWrite(Right, LOW);
    delay(d_time);
    digitalWrite(Right, HIGH);
  }
}

/************************************************************************
  Function: setup
  Purpose: setup and begin serial communication, ESPNOW, callback
            function, and interface to other micros
  Returns: All digital output pins are set to high
            This ESP32 is set up to only recieve data from another ESP32
  Last Modified: 11/19/2025
************************************************************************/
void setup()
{
  // begin serial communication
  Serial.begin(115200);

// relay setup
  delay(10000); // delay for a long time to give the other micros and LED matrix time to setup
  pinMode(19,OUTPUT); // set pins that are output to relays
  pinMode(5,OUTPUT);

  // Initialize pin outputs
  // set pin modes and default values (high)
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(Select, OUTPUT);
  pinMode(Start, OUTPUT);
  pinMode(Up, OUTPUT);
  pinMode(Down, OUTPUT);
  pinMode(Left, OUTPUT);
  pinMode(Right, OUTPUT); 
  
  digitalWrite(19, HIGH); // set the relay output pins high
  digitalWrite(5, HIGH);

  // this tells the Tetris Table micro that no buttons are pressed (buttons are active low)
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(Select, HIGH);
  digitalWrite(Start, HIGH);  
  digitalWrite(Up, HIGH);
  digitalWrite(Down, HIGH);
  digitalWrite(Left, HIGH);
  digitalWrite(Right, HIGH);

  delay(1000);  // delay for a lil while and set relay output pins to low
  digitalWrite(19, LOW);
  delay(1000);
  digitalWrite(5, LOW);

  // set up wifi mode
  WiFi.mode(WIFI_STA);
  //Init ESP-NOW, send error to serial port if init fails and quit program
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecieve));

// code for getting reciever board mac addr
  // Serial.print("ESP Board MAC Address: ");

  // WiFi.mode(WIFI_STA);
  // WiFi.STA.begin();
  // uint8_t baseMac[6];
  // esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  // if (ret == ESP_OK) {
  //  Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
  //                 baseMac[0], baseMac[1], baseMac[2],
  //                 baseMac[3], baseMac[4], baseMac[5]);
  // } else {
  //   Serial.println("Failed to read MAC address");
  // }
}

/************************************************************************
  Function: loop
  Purpose: has to be here or else
  Returns: whenever it feels like it
  Last Modified: never
************************************************************************/
void loop()
{

}
