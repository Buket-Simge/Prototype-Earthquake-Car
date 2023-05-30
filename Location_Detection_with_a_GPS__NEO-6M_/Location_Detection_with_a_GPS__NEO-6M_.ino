#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>

#define CONNECTION_TIMEOUT 10 // The connecton will be restarted after 10 seconds

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>



#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1 //Reset pin 
#define SCREEN_ADDRESS 0x3C //Setting the LCD I2C Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Create LCD instance


#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1); // Create the GPS module instance

TinyGPSPlus gps; // Library instance for parsing GPS data

const char* ssid = "Galaxy S20 FEDC3C"; //Your Wi-Fi name
const char* password = "yvlt7136"; // Your Wi-Fi password

#define CHAT_ID "5872174548" // Your chat id

// Initialize Telegram BOT
#define BOTtoken "6247962591:AAGJ2HMxmK-ZnqllVz7vzYJtl4SczTbmJuU"  // Your Bot Token 

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client); // Creating a bot

//Checking for new messages every 1 second - Checking for new Telegram messages
int botRequestDelay = 1000; //1000 milisecons = 1 second
unsigned long lastTimeBotRan;

String getReadings(){ // Getting the GPS location data, and in order to send them to the Telegram bot returns the data as a String
  
   if (gps.location.isValid() == 1)
  {
    float Latitude,Longitude,Speed,Satellites,Altitude;
    Latitude = gps.location.lat();
    Longitude = gps.location.lng();
    Speed = gps.speed.kmph();
    Satellites = gps.satellites.value();
    Altitude = gps.altitude.meters();

    
   String message = "Latitude: " + String(Latitude) + "\n";
   message += "Longitude: " + String(Longitude) + "\n";
   message += "Speed: " + String(Speed) + " kmph \n";
   message += "Satellites: " + String(Satellites) + "\n";
   message += "Altitude: " + String(Altitude) + " meters \n";
  
   return message;
  }
  else{
    String message = "NO DATA  \n";
    Serial.println("NO DATA");

    return message;
  }
  
}

void handleNewMessages(int numNewMessages) { 
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) { 
    
    String chat_id = String(bot.messages[i].chat_id); // Chat id of the user who wants to take message
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    String text = bot.messages[i].text;  // Printing the received message
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command to get current readings.\n\n";
      welcome += "/readings \n";
      bot.sendMessage(chat_id, welcome, "");
      Serial.println(welcome);
    }

    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }

  }
}


void setup() {
  
  Serial.begin(115200); //Begin serial communication 
  
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2); //Begin serial communication for NEO-6M GPS Module
  
  //Starting I2c communication with LCD
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
  delay(2000);


   #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    client.setTrustAnchors(&cert);
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  #endif

  int timeout_counter = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    timeout_counter++;
    if(timeout_counter >= CONNECTION_TIMEOUT*5){
      ESP.restart();
    }
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

}

void loop() { 
 
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }

  if(newData == true)
  {
    newData = false;
    Serial.println(gps.satellites.value());
    print_data();
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
  }  

  //checking for new messages every second.
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  
}

void print_data() // Display GPS Data on OLED Display
{
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
       
  if (gps.location.isValid() == 1)
  {
    display.setTextSize(1);
    
    display.setCursor(25, 5);
    display.print("Lat: ");
    display.setCursor(50, 5);
    display.print(gps.location.lat(),6);

    display.setCursor(25, 20);
    display.print("Lng: ");
    display.setCursor(50, 20);
    display.print(gps.location.lng(),6);

    display.setCursor(25, 35);
    display.print("Speed: ");
    display.setCursor(65, 35);
    display.print(gps.speed.kmph());
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("SAT:");
    display.setCursor(25, 50);
    display.print(gps.satellites.value());

    display.setTextSize(1);
    display.setCursor(70, 50);
    display.print("ALT:");
    display.setCursor(95, 50);
    display.print(gps.altitude.meters(), 0);

    display.display();
    
  }
  else
  {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.print("No Data");
    display.display();
  }  

}
