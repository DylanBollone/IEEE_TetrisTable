/*********
  ESP32 NES Controller Web Server
  mDNS + Single Client Lock + GPIO + Relay
  Hostname: IEEE_PLAY_TETRIS.local
*********/

#include <WiFi.h>
#include <ESPmDNS.h>

/* ===================== WiFi ===================== */
const char* ssid     = "LSSU";
const char* password = "";

/* ===================== Server ===================== */
WiFiServer server(80);

/* ===================== GPIO MAP (ACTIVE LOW) ===================== */
#define PIN_A       13
#define PIN_B       12
#define PIN_SELECT  32
#define PIN_START   33
#define PIN_UP      25
#define PIN_DOWN    26
#define PIN_LEFT    27
#define PIN_RIGHT   14

#define RELAY1 19
#define RELAY2 5

/* ===================== TIMING ===================== */
const uint32_t PULSE_TIME_MS = 50;
const uint32_t CLIENT_TIMEOUT_MS = 15000;   // ✅ 15 seconds

/* ===================== CLIENT LOCK ===================== */
IPAddress lockedIP;
uint32_t lastClientActivity = 0;
bool clientLocked = false;

/* ===================== GPIO PULSE SCHEDULER ===================== */
struct Pulse {
  int pin;
  bool active;
  uint32_t start;
};

Pulse pulses[] = {
  {PIN_A,false,0},{PIN_B,false,0},{PIN_SELECT,false,0},{PIN_START,false,0},
  {PIN_UP,false,0},{PIN_DOWN,false,0},{PIN_LEFT,false,0},{PIN_RIGHT,false,0}
};

const int NUM_PULSES = sizeof(pulses)/sizeof(pulses[0]);

/* ===================== BUTTON MAP ===================== */
struct ButtonMap {
  const char* path;
  char code;
};

ButtonMap buttons[] = {
  {"/A",'A'}, {"/B",'B'},
  {"/select",'s'}, {"/start",'S'},
  {"/up",'U'}, {"/down",'D'},
  {"/left",'L'}, {"/right",'R'}
};

const int NUM_BUTTONS = sizeof(buttons)/sizeof(buttons[0]);

/* ===================== SETUP ===================== */
void setup() {
  Serial.begin(115200);
  delay(1000);

  int pins[] = {
    PIN_A,PIN_B,PIN_SELECT,PIN_START,
    PIN_UP,PIN_DOWN,PIN_LEFT,PIN_RIGHT,
    RELAY1,RELAY2
  };

  for(int i=0;i<10;i++){
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }

  // Relay startup sequence
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  delay(1000);
  digitalWrite(RELAY1, LOW);
  delay(1000);
  digitalWrite(RELAY2, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);

  Serial.print("Connecting to WiFi");
  while(WiFi.status()!=WL_CONNECTED){
    delay(200);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if(MDNS.begin("IEEE_PLAY_TETRIS")){
    MDNS.addService("http","tcp",80);
    Serial.println("mDNS active: http://IEEE_PLAY_TETRIS.local");
  }

  server.begin();
  Serial.println("Web server started");
}

/* ===================== LOOP ===================== */
void loop() {
  handlePulses();
  handleTimeout();
  handleClient();
}

/* ===================== CLIENT TIMEOUT ===================== */
void handleTimeout(){
  if(clientLocked && millis() - lastClientActivity > CLIENT_TIMEOUT_MS){
    Serial.println("Client timeout — control released");
    clientLocked = false;
  }
}

/* ===================== CLIENT HANDLER ===================== */
void handleClient(){
  WiFiClient client = server.available();
  if(!client) return;

  IPAddress remote = client.remoteIP();
  String req = client.readStringUntil('\r');
  client.flush();

  if(clientLocked && remote != lockedIP){
    client.println("HTTP/1.1 403 Forbidden\r\n\r\n");
    client.stop();
    return;
  }

  if(!clientLocked){
    lockedIP = remote;
    clientLocked = true;
    Serial.print("Controller locked to ");
    Serial.println(lockedIP);
  }

  lastClientActivity = millis();

  for(int i=0;i<NUM_BUTTONS;i++){
    if(req.indexOf(buttons[i].path)>=0){
      schedulePulse(buttons[i].code);
      client.println("HTTP/1.1 200 OK\r\n\r\nOK");
      client.stop();
      return;
    }
  }

  sendHTML(client);
}

/* ===================== GPIO ===================== */
void schedulePulse(char code){
  int pin=-1;
  const char* name="";

  switch(code){
    case 'A':pin=PIN_A;name="A";break;
    case 'B':pin=PIN_B;name="B";break;
    case 's':pin=PIN_SELECT;name="SELECT";break;
    case 'S':pin=PIN_START;name="START";break;
    case 'U':pin=PIN_UP;name="UP";break;
    case 'D':pin=PIN_DOWN;name="DOWN";break;
    case 'L':pin=PIN_LEFT;name="LEFT";break;
    case 'R':pin=PIN_RIGHT;name="RIGHT";break;
  }

  if(pin<0) return;

  Serial.print("Button pressed: ");
  Serial.println(name);

  for(int i=0;i<NUM_PULSES;i++){
    if(pulses[i].pin==pin && !pulses[i].active){
      pulses[i].active=true;
      pulses[i].start=millis();
      digitalWrite(pin,LOW);
      break;
    }
  }
}

void handlePulses(){
  uint32_t now=millis();
  for(int i=0;i<NUM_PULSES;i++){
    if(pulses[i].active && now-pulses[i].start>=PULSE_TIME_MS){
      digitalWrite(pulses[i].pin,HIGH);
      pulses[i].active=false;
    }
  }
}

/* ===================== HTML ===================== */
void sendHTML(WiFiClient &client){
  client.print(R"rawliteral(
HTTP/1.1 200 OK
Content-Type: text/html
Connection: close

<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<style>
body{margin:0;background:#222;color:white;font-family:Arial;
display:flex;justify-content:center;align-items:center;height:100vh;}
.container{display:grid;grid-template-columns:3fr 2fr;gap:40px;}
button{touch-action:manipulation;-webkit-user-select:none;
user-select:none;border:none;color:white;cursor:pointer;}
.dpad{display:grid;grid-template-columns:60px 60px 60px;
grid-template-rows:60px 60px 60px;}
.dpad button{background:#444;border-radius:10px;}
.actions{display:grid;grid-template-columns:80px 80px;gap:20px;}
.actions button{background:red;border-radius:50%;height:80px;}
.center{display:flex;justify-content:center;gap:20px;margin-bottom:20px;}
.center button{width:60px;height:30px;background:#666;border-radius:10px;}
</style>
<script>
function press(p){ fetch(p); }
</script>
</head>
<body>
<div class="container">
  <div class="dpad">
    <div></div><button onclick="press('/up')">Up</button><div></div>
    <button onclick="press('/left')">Left</button><div></div><button onclick="press('/right')">Right</button>
    <div></div><button onclick="press('/down')">Down</button><div></div>
  </div>
  <div>
    <div class="center">
      <button onclick="press('/select')">Select</button>
      <button onclick="press('/start')">Start</button>
    </div>
    <div class="actions">
      <button onclick="press('/B')">B</button>
      <button onclick="press('/A')">A</button>
    </div>
  </div>
</div>
</body>
</html>
)rawliteral");
}
