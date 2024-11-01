// #include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

// #include <EEPROM.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Replace with your network credentials
const char* setupSSID = "MD123879";
const char* setupPassword = "$med123789";
IPAddress local_IP(192, 168, 4, 22);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);
ESP8266WebServer server(80);

// const unsigned char wifiOn[] PROGMEM = {
//  // 'wifi', 24x24px
// 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xfc,
// 0x00, 0x3f, 0xf0, 0x00, 0x0f, 0xe0, 0x00, 0x07, 0xc0, 0x7e, 0x03, 0x81, 0xff, 0x81, 0xc7, 0x81,
// 0xe3, 0xee, 0x00, 0x77, 0xfc, 0x00, 0x3f, 0xfc, 0x00, 0x3f, 0xfc, 0x3c, 0x3f, 0xfe, 0x7e, 0x7f,
// 0xff, 0xe7, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xc3, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff,
// 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool connectedWifi = false;
int cstTime = -6;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", cstTime * 60 * 60, 1800);

//Week Days
String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//Month names
String months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

// struct data {
//   int morningTimingHr;
//   int morningTimingMin;
//   bool morningTimingEnable;
//   int afternoonTimingHr;
//   int afternoonTimingMin;
//   bool afternoonTimingEnable;
//   int nightTimingHr;
//   int nightTimingMin;
//   bool nightTimingEnable;
// } user_settings = {};

const int buzzer = 2;
const int button1Pin = 13;
const int button2Pin = 12;
// int sensorValue;
// float voltage;
bool button1Previous = false;
bool button2Previous = false;


int morning_hour;
int morning_min;
bool morning;
bool morningComplete;

int afternoon_hour;
int afternoon_min;
bool afternoon;
bool afternoonComplete;

int night_hour;
int night_min;
bool night;
bool nightComplete;

// bool startUpStatus = false;

String header;

String index_html = "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before {box-sizing: border-box;}body {margin: 0;font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans';font-size: 1rem;font-weight: 400;line-height: 1.5;color: #212529;background-color: #f5f5f5;}.form-control {display: block;width: 100%;height: calc(1.5em + .75rem + 2px);border: 1px solid #ced4da;}button {cursor: pointer;border: 1px solid transparent;color: #fff;background-color: #007bff;border-color: #007bff;padding: .5rem 1rem;font-size: 1.25rem;line-height: 1.5;border-radius: .3rem;width: 100;}.form-signin {width: 100%;max-width: 400px;padding: 15px;margin: auto;}h1, h2 {text-align: center}</style></head><body><main class='form-signin'><form action='/' method='post'><h1 class=''>Wifi Setup</h1><h2>Networks::</h2><ul>";

String indexTwo_html = "</ul><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br /><label>Password</label><input type='password' class='form-control'name='password'></div><br /><br /><h2>Settings:</h2><div> <label class='switch'><input type='checkbox' name='morning'><span class='slider round'></span><label>Morning</label><input type='time' id='morning_time' name='morning_time' value='00:00'></label></div><div> <label class='switch'><input type='checkbox' name='afternoon'><span class='slider round'></span><label>Afternoon</label><input type='time' id='afternoon_time' name='afternoon_time' value='00:00'></label></div><div> <label class='switch'><input type='checkbox' name='night'><span class='slider round'></span><label>Night</label><input type='time' id='night_time' name='night_time' value='00:00'></label></div><p><strong>Note:</strong> Please make sure that times are within the below ranges.</p><ul><li>Morning: 12:01 AM - 11:59AM</li><li>Afternoon: 12:00 PM - 4:59 PM</li><li>Night: 5:00 PM - 11:59 PM</li></ul><h2> </h2><button type='submit'>Save</button><!-- <p style='text-align: right'><a href='https://www.mrdiy.ca' style='color: #32C5FF'>mrdiy.ca</a></p> --></form></main></body></html>";

const char output_html[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang='en'>

<head>
    <meta charset='utf-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Wifi Setup</title>
    <style>
        *,
        ::after,
        ::before {
            box-sizing: border-box;
        }

        body {
            margin: 0;
            font-family: 'Segoe UI', Roboto, 'Helvetica Neue', Arial, 'Noto Sans', 'Liberation Sans';
            font-size: 1rem;
            font-weight: 400;
            line-height: 1.5;
            color: #212529;
            background-color: #f5f5f5;
        }

        .form-control {
            display: block;
            width: 100%;
            height: calc(1.5em + .75rem + 2px);
            border: 1px solid #ced4da;
        }

        button {
            border: 1px solid transparent;
            color: #fff;
            background-color: #007bff;
            border-color: #007bff;
            padding: .5rem 1rem;
            font-size: 1.25rem;
            line-height: 1.5;
            border-radius: .3rem;
            width: 100%
        }

        .form-signin {
            width: 100%;
            max-width: 400px;
            padding: 15px;
            margin: auto;
        }

        h1,
        p {
            text-align: center
        }
    </style>
</head>

<body>
    <main class='form-signin'>
        <h1>Wifi Setup</h1> <br />
        <p>Your settings have been saved successfully!<br />Please wait a few secound for the settings to save.</p>
    </main>
</body>

</html>)rawliteral";

// <!DOCTYPE HTML><html>
// <head>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <style>
//     html {
//      font-family: Arial;
//      display: inline-block;
//      margin: 0px auto;
//      text-align: center;
//     }
//     h2 { font-size: 1.0rem; }
//     p { font-size: 3.0rem; }
//     .units { font-size: 1.2rem; }
//   </style>
// </head>
// <body>
//   <h2>Connect to a Wifi Networks</h2>
//   <p>
//     <form action="#">
//   <label for="wifi">Choose a hotspot to connect:</label>
//   <select name="wifi" id="wifi">
//     <option value="hotspot">None</option>
//   </select>

//   <label for="wifi">Enter password:</label>
//   <input type="text"></input>
  


//   <input type="button" value="Rescan">
//   <input type="submit" value="Connect">
//   </form>
//   </p>
// </body>

// </script>
// </html>)rawliteral";

struct settings {
  char ssid[30];
  char password[30];
  int morningTimingHr;
  int morningTimingMin;
  bool morningTimingEnable;
  int afternoonTimingHr;
  int afternoonTimingMin;
  bool afternoonTimingEnable;
  int nightTimingHr;
  int nightTimingMin;
  bool nightTimingEnable;
} user_settings = {};

void network() {

  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, user_settings);

  morning_hour = user_settings.morningTimingHr;
  morning_min = user_settings.morningTimingMin;
  morning = user_settings.morningTimingEnable;

  afternoon_hour = user_settings.afternoonTimingHr;
  afternoon_min = user_settings.morningTimingMin;
  afternoon = user_settings.afternoonTimingEnable;

  night_hour = user_settings.nightTimingHr;
  night_min = user_settings.nightTimingMin;
  night = user_settings.nightTimingEnable;

  WiFi.mode(WIFI_STA);
  WiFi.begin(user_settings.ssid, user_settings.password);

  byte tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (tries++ > 30) {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(local_IP, gateway, subnet);
      WiFi.softAP(setupSSID, setupPassword, 1, 0, 4);
      break;
    }
  }
  server.on("/", handlePortal);
  server.begin();

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  // timeClient.setTimeOffset(0);
}

// void startUp() {
//   Serial.print("Setting AP (Access Point)…");
//   // Remove the password parameter, if you want the AP (Access Point) to be open
//   WiFi.softAPConfig(local_IP, gateway, subnet)
//   WiFi.softAP(ssid, password);

//   IPAddress IP = WiFi.softAPIP();
//   Serial.print("AP IP address: ");
//   Serial.println(IP);

//   // Print ESP8266 Local IP Address
//   Serial.println(WiFi.localIP());

//   // Route for root / web page
//    // Route for root / web page
//   // ap_server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
//   //   request->send_P(200, "text/html", index_html, processor);
//   // });

//   // // Start server
//   // ap_server.begin();
// }

// // String* getWifiHotspots() {
// // //  String* arr = new String[];
// //   String arr[100];
// //   Serial.print("Scan start ... ");
// //   int n = WiFi.scanNetworks();
// //   for (int i = 0; i < n; i++)
// //   {
// //     arr[i] = WiFi.SSID(i).c_str();
// //   }
// //   return arr;
// // }

void setup() {
  Serial.begin(9600);
  network();
  pinMode(buzzer, OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  // initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  // Display Text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Medicine");
  display.display();

  display.setCursor(3, 20);
  display.setTextSize(0.5);
  display.println("Version: 0.1");
  display.setCursor(115, 20);
  display.println("C");
  // display.write(169);
  display.display();
  delay(2000);
  // display.clearDisplay();
  // Clear the buffer.
  display.clearDisplay();
}

void reset() {
  WiFi.mode(WIFI_OFF);
  // WiFi.disconnect()
  EEPROM.begin(512);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
    Serial.println(i);
  }

  // turn the LED on when we're done
  // pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  EEPROM.end();
  setup();

}

void settings() {
  Serial.println("Info: In Settings!!");
  reset();
}


void loop() {
  display.clearDisplay();
  if (WiFi.isConnected() == true) {
    int rssi;
    int numberOfNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numberOfNetworks; i++) {
      if (WiFi.SSID(i) == user_settings.ssid) {
        rssi = WiFi.RSSI(i);
        break;
      }
    }
    if (rssi >= -55) { 
      display.drawRect(0,7,4,1, WHITE);
      display.drawRect(5,6,4,2, WHITE);
      display.drawRect(10,4,4,4, WHITE);
      display.drawRect(15,2,4,6, WHITE);
      display.drawRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    } else if (rssi < -55 & rssi > -65) {
      display.drawRect(0,7,4,1, WHITE);
      display.drawRect(5,6,4,2, WHITE);
      display.drawRect(10,4,4,4, WHITE);
      display.drawRect(15,2,4,6, WHITE);
      display.fillRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    } else if (rssi < -65 & rssi > -75) {
      display.drawRect(0,8,4,1, WHITE);
      display.drawRect(5,6,4,2, WHITE);
      display.drawRect(10,4,4,4, WHITE);
      display.fillRect(15,2,2,6, WHITE);
      display.fillRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    } else if (rssi < -75 & rssi > -85) {
      display.drawRect(0,8,4,1, WHITE);
      display.drawRect(5,6,4,2, WHITE);
      display.fillRect(10,4,4,4, WHITE);
      display.fillRect(15,2,4,6, WHITE);
      display.fillRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    } else if (rssi < -85 & rssi > -96) {
      display.drawRect(0,8,4,1, WHITE);
      display.fillRect(5,6,4,2, WHITE);
      display.fillRect(10,4,4,4, WHITE);
      display.fillRect(15,2,4,6, WHITE);
      display.fillRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    } else {
      display.fillRect(0,8,4,1, WHITE);
      display.fillRect(5,6,4,2, WHITE);
      display.fillRect(10,4,4,4, WHITE);
      display.fillRect(15,2,4,6, WHITE);
      display.fillRect(20,0,4,8, WHITE);
      // display.sendBuffer();
    }
  } else {
    server.handleClient();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("X");
  }
  if (morning == true && morningComplete == false) {
    display.setCursor(50, 0);
    display.print("M");
  }
  if (afternoon == true && afternoonComplete == false) {
    display.setCursor(60, 0);
    display.print("A");
  }
  if (night == true && nightComplete == false) {
    display.setCursor(70, 0);
    display.print("N");
  }
  // display.setCursor(0,0);
  // display.drawBitmap(0, 0, wifiOn, 8, 8, WHITE);
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  // Serial.println("Hour: " + currentHour + " Min: " currentMinute);

  display.setTextSize(2);
  display.setCursor(35, 8);
  if(currentHour>=0 && currentHour < 10) {
    display.print("0");
  }
  display.print(currentHour);
  // Serial.print(currentHour);
  // Serial.print(":");
  // Serial.print(currentMinute);
  // Serial.println();
  display.print(":");
  if(currentMinute>=0 && currentMinute < 10) {
    display.print("0");
  }
  display.print(currentMinute);
  // display.drawRect(70, 10, 30, 30, 0x001F);
  display.setCursor(2, 24);
  display.setTextSize(1);
  display.print("OK");
  display.drawRect(0, 23, 15, 9, WHITE);
  display.setCursor(102, 24);
  display.print("MENU");
  display.drawRect(100, 23, 26, 9, WHITE);
  display.display();
  // display.drawRect(0,0,30,15);
  // display.setCursor(8, 4);
  // display.println("M");
  // display.drawRect(35,0,30,15);
  // display.setCursor(43, 4);
  // display.println("A");
  // display.drawRect(70,0,30,15, "white");
  // display.setCursor(78, 4);
  // display.println("N");










  // Serial.println(digitalRead(button1Pin));
  // Serial.println(digitalRead(button2Pin));
  // Serial.println(digitalRead(button3Pin));
  // Hold button 3 down for 3 secound to activate the setting menu.
  if (digitalRead(button2Pin) == 1 && digitalRead(button1Pin) == 1) {
    bool button2Held = false;
    for (int i = 0; i <= 5; i++) {
      if (i == 5 && button2Held == true) {
        settings();
        button2Held = false;
      }
      delay(500);
      if (digitalRead(button2Pin) == 1) button2Held = true;
      else button2Held = false;
    }
  }
  if ((digitalRead(button1Pin) == 0 && button1Previous == true) ||(digitalRead(button2Pin) == 0 && button2Previous == true)) {
    button1Previous = false;
    button2Previous = false;
  } else if (digitalRead(button1Pin) == 1 && button1Previous == false && ((currentHour >= 0 && currentHour < 12 && morningComplete == false && morning == true) || (currentHour >= 12 && currentHour < 17 && afternoonComplete == false && afternoon == true) || (currentHour >= 17 && currentHour < 24 && nightComplete == false && night == true))) {
    button1Previous = true;
    if (currentHour >= 0 && currentHour < 12 && morningComplete == false && morning == true) {
      morningComplete = true;
      digitalWrite(buzzer, LOW);
    }
    if (currentHour >= 12 && currentHour < 17 && afternoonComplete == false && afternoon == true) {
      morningComplete = true;
      afternoonComplete = true;
      digitalWrite(buzzer, LOW);
    }
    if (currentHour >= 17 && currentHour < 24 && nightComplete == false && night == true) {
      morningComplete = true;
      afternoonComplete = true;
      nightComplete = true;
      digitalWrite(buzzer, LOW);
    }
    Serial.println("The button 1 is pressed");
    // digitalWrite(buzzer, HIGH);
    // delay(1000);        // ...for 1 sec
    // digitalWrite(buzzer, LOW);
    // delay(1000);        // ...for 1sec
  } else if (digitalRead(button2Pin) == 1 && button2Previous == false) {
    button2Previous = true;
    Serial.println("The button 2 is pressed");
    settings();
  }
}


void handlePortal() {
  String avalibleNetworks = "";
  int numberOfNetworks = WiFi.scanNetworks();
  Serial.println("-----------------------");
  for (int i = 0; i < numberOfNetworks; i++) {
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
    avalibleNetworks += "<li>" + WiFi.SSID(i) + "</li>";
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
    Serial.println("-----------------------");
  }
  // <script>
  // setInterval(function ( ) {
  //   var xhttp = new XMLHttpRequest();
  //   xhttp.onreadystatechange = function() {
  //     document.getElementById("temperature").innerHTML = this.responseText;
  //   };
  //   xhttp.open("GET", "/", true);
  //   xhttp.send();
  // }, 10000 ) ;
  // </script>
  if (server.method() == HTTP_POST) {
    // reset();
    Serial.println(server.arg("morning_time"));
    if (server.arg("morning") == "on") {
      if (server.arg("morning_time").substring(0, 1) == "0") {
        user_settings.morningTimingHr = ((server.arg("morning_time")).substring(1, 2)).toInt();
      } else {
        user_settings.morningTimingHr = ((server.arg("morning_time")).substring(0, 2)).toInt();
      }
      if (server.arg("morning_time").substring(3, 4) == "0") {
        user_settings.morningTimingMin = ((server.arg("morning_time")).substring(4)).toInt();
      } else {
        user_settings.morningTimingMin = ((server.arg("morning_time")).substring(3)).toInt();
      }
      user_settings.morningTimingEnable = true;
    } else {
      user_settings.morningTimingEnable = false;
    }
    if (server.arg("afternoon") == "on"){
      if (server.arg("afternoon_time").substring(0, 1) == "0") {
        user_settings.afternoonTimingHr = ((server.arg("afternoon_time")).substring(1, 2)).toInt();
      } else {
        user_settings.afternoonTimingHr = ((server.arg("afternoon_time")).substring(0, 2)).toInt();
      }
      if (server.arg("afternoon_time").substring(3, 4) == "0") {
        user_settings.afternoonTimingMin = ((server.arg("afternoon_time")).substring(4)).toInt();
      } else {
        user_settings.afternoonTimingMin = ((server.arg("afternoon_time")).substring(3)).toInt();
      }
      user_settings.afternoonTimingEnable = true;
    } else {
      user_settings.afternoonTimingEnable = false;
    }
    if (server.arg("night") == "on") {
      if (server.arg("night_time").substring(0, 1) == "0") {
        user_settings.nightTimingHr = ((server.arg("night_time")).substring(1, 2)).toInt();
      } else {
        user_settings.nightTimingHr = ((server.arg("night_time")).substring(0, 2)).toInt();
      }
      if (server.arg("afternoon_time").substring(3, 4) == "0") {
        user_settings.nightTimingMin = ((server.arg("night_time")).substring(4)).toInt();
      } else {
        user_settings.nightTimingMin = ((server.arg("night_time")).substring(3)).toInt();
      }
      user_settings.nightTimingEnable = true;
    } else {
      user_settings.nightTimingEnable = false;
    }
    strncpy(user_settings.ssid, server.arg("ssid").c_str(), sizeof(user_settings.ssid));
    strncpy(user_settings.password, server.arg("password").c_str(), sizeof(user_settings.password));
    user_settings.ssid[server.arg("ssid").length()] = user_settings.password[server.arg("password").length()] = '\0';
    EEPROM.put(0, user_settings);
    EEPROM.commit();

    server.send(200, "text/html", output_html);
    setup();
  } else {

    server.send(200, "text/html", index_html + avalibleNetworks + indexTwo_html);// + avalibleNetworks + index2_html
  }
}