/************************************************************************
  Dylan Bollone
  IEEE Tetris Project
  ESP32 connected to NES Controller
  Last Updated: 11/15/2025
************************************************************************/

/************************************************************************
  Following code is for the ESP32 that reads in button pressed from an
  offbrand NES controller, and communcates the data to another ESP32 that
  is connect to an old Micros-II project (LED Coffee Table Featuring 
  Tetris). The purpose of this ESP32 is to find out which buttons are
  currently being pressed on the controller, and send the data to a 
  "slave" ESP32.

  Quick note: Tetris Table and Tetris Machine refer to the same
              inanimate object
************************************************************************/

/************************************************************************
  Give credit to: Rui Santos
  Used their resources to set up working ESPNOW communication
  Complete project details at https://randomnerdtutorials.com
************************************************************************/

// load in wifi and esp now libraries
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// define what gpio pins are being used to interface to the NES Controller
#define Latch 26
#define Clk 27
#define Data 14

// define which bit in controller data the buttons map to
#define A_button 0
#define B_button 1
#define Select_button 2
#define Start_button 3
#define Up_button 4
#define Down_button 5
#define Left_button 6
#define Right_button 7

// Variables used to keep track of all buttons current states
uint8_t previous_button_states = 0, curr_button_states = 0;

/************************************************************************
  This next section of code is a mixture of the stuff from Santos's 
  examples on how to communicate between ESP32's. I modified the structs
  internal data to match our case, and am using an array to store button
  presses.
************************************************************************/
// reciever mac address
uint8_t recieverMAC[] = {0xCC,0x50,0xE3,0xAF,0x22,0xC0};
// struct for incoming data - same as outgoing data from other esp32
typedef struct data
{
  char button;
} data;
// name a var of that type
//   This program uses a buffer to store button presses, so that we can wait
//   to send data, giving the other micro time to communicate with Tetris Table
const int buff_size = 20;
data outData[buff_size];
// buff_index keeps track of where we are in the input/output buffer
int buff_index = 0, send_index=0, j=0;
// store info about peer
esp_now_peer_info_t peerInfo;

/************************************************************************
  Function: OnDataSent
  Purpose: Whenever data is sent to another ESP32, output if delivery
            was successful
  Returns: Updata on status of data sent
  Last Modified: 11/15/2025
************************************************************************/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

/************************************************************************
  Function: setup
  Purpose: setup and begin serial communication, ESPNOW, callback
            function, and interface to NES controller
  Returns: Latch and Clk pins are set to High, Data pin is ready to read
            data from NES controller, this ESP32 is set up to only send
            data to ESP32 connected to Tetris Table
  Last Modified: 11/15/2025
************************************************************************/
void setup()
{
  // begin serial communication
  Serial.begin(115200);
  delay(2000);  // Give serial time to initialize
  Serial.println("=== STARTING SETUP ===");

  // set pins to correct type (io)
  pinMode(Latch, OUTPUT);
  pinMode(Clk, OUTPUT);
  pinMode(Data, INPUT);

  // set initial states for clock and latch output pins
  digitalWrite(Latch, LOW);
  digitalWrite(Clk, LOW);
  
  Serial.println("GPIO pins configured");
  int i;

  // wifi sender status setup
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  // Scan to find what channel the receiver is on
  Serial.println("Scanning for receiver's channel...");
  int n = WiFi.scanNetworks();
  int32_t channel = 0;

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    // Check if this network's BSSID matches our receiver's MAC
    if (ssid == "LSSU") {
      channel = WiFi.channel(i);
      Serial.print("Found receiver on channel: ");
      Serial.println(channel);
      break;
    }
  }

  if (channel == 0) {
    Serial.println("ERROR: Could not find receiver! Make sure it's powered on and connected to WiFi.");
    while(1) delay(1000);
  }

  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  Serial.println("WiFi mode set to STA and channel matched");
  
  // initialize espNOW
  Serial.println("Attempting ESP-NOW init...");
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    while(1) delay(1000);  // Stop here if it fails
  }
  Serial.println("ESP-NOW initialized successfully");
  
  // register callback function
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  Serial.println("Callback registered");
  
  // register peer
  memcpy(peerInfo.peer_addr, recieverMAC, 6);

  peerInfo.channel = channel;
  peerInfo.encrypt = false;
  
  Serial.println("Attempting to add peer...");
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    while(1) delay(1000);  // Stop here if it fails
  }
  Serial.println("Peer added successfully");

  Serial.print("Sender MAC: ");
  Serial.println(WiFi.macAddress());
  
  Serial.println("=== SETUP COMPLETE ===");
}

/************************************************************************
  Function: loop
  Purpose: Poll NES controller for button presses, manage io buffer, 
            send data to other ESP32 when neccessary
  Returns: NEVER. JK, it prints out to serial monitor whenever a new
            button press is detected.
  Last Modified: 11/19/2025
************************************************************************/
void loop()
{

  previous_button_states = curr_button_states;
  curr_button_states = get_NES_byte();

  // checks if current button states are the same as previous
  // if the bit we want to look at is set, and in the previous states it was not clear, the output the button press
  // note: does not output multiple times when buttons are held
  if (bitRead(curr_button_states, A_button) == 1 && !bitRead(previous_button_states, A_button) == 1)
  {
    // print button read to serial port
    Serial.println("A");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'A';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, B_button) == 1 && !bitRead(previous_button_states, B_button) == 1)
  {
    Serial.println("B");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'B';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Select_button) == 1 && !bitRead(previous_button_states, Select_button) == 1)
  {
    Serial.println("Select");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 's';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Start_button) == 1 && !bitRead(previous_button_states, Start_button) == 1)
  {
    Serial.println("Start");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'S';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Up_button) == 1 && !bitRead(previous_button_states, Up_button) == 1)
  {
    Serial.println("Up");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'U';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Down_button) == 1 && !bitRead(previous_button_states, Down_button) == 1)
  {
    Serial.println("Down");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'D';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Left_button) == 1 && !bitRead(previous_button_states, Left_button) == 1)
  {
    Serial.println("Left");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'L';
    // increment index
    buff_index+=1;
  }
  if (bitRead(curr_button_states, Right_button) == 1 && !bitRead(previous_button_states, Right_button) == 1)
  {
    Serial.println("Right");
    // add button input to buffer of chars to be output to other esp32
    outData[buff_index].button = 'R';
    // increment index
    buff_index+=1;
  }
  // if we've went past the max buffer index, reset index
  if(buff_index>=buff_size)
    buff_index = 0;

  // delay a small amount before getting next button press
  delay(15);

  if(j==2)
  {
    // reset wait condition for sending data
    // it takes 21 times of running void loop() to send one char to other esp32
    //   This is done because the reciever board needs time to communicate the information
    //   that it is given to the micros on the Tetris Machine
    j = 0;

    // send data if new data has been put into buffer
    if(buff_index!=send_index)
    {
      esp_err_t result1 = esp_now_send(
          recieverMAC, 
          (uint8_t *) &outData[send_index],
          sizeof(data));
      
      // check if sending data worked (any erros in communication?)
      if (result1 == ESP_OK)
      {
        Serial.println("Sent with success");
      }
      else
      {
        Serial.println("Error sending the data");
      }
      // move send_index to the next char, and if we've overrun the buffer reset index to 0
      send_index+=1;
      if(send_index>=buff_size)
       send_index=0;
    }
  }

  j++;
}

/************************************************************************
  Function: get_NES_byte
  Purpose: Poll the NES controller for the state of each button on the
            controller.
  Returns: 8-bit integer
            Each bit in the int maps to a specific button on the
            controller (only 8 buttons that can be pressed)
            Buttons are active-low. Bits in the int will be high if the
            button is not being pressed, and low if the button is pressed
  Last Modified: 11/15/2025
************************************************************************/
// this function is copy and pasted straight from chatgpt, 
// I will do my best to comment on what it is doing
uint8_t get_NES_byte() {
  uint8_t result = 0;

  // Latch pulse to capture button states
  //   Controller hardware uses a shift register to capture button states
  digitalWrite(Latch, HIGH);
  delayMicroseconds(12);
  digitalWrite(Latch, LOW);

  // Read 8 bits from the controller
  for (int i = 0; i < 8; i++)
  {
    int bitVal = !digitalRead(Data);  // NES outputs 0 for pressed
    result |= (bitVal << i);

    // Pulse the clock, to read in next button state
    digitalWrite(Clk, HIGH);
    delayMicroseconds(20);
    digitalWrite(Clk, LOW);
    delayMicroseconds(20);
  }

  // results bits => A->B->Select->Start->Up->Down->Left->Right
  return result;
}
