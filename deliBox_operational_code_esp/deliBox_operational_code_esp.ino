
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Keypad.h>
#include <Wire.h>



//DISPLAY
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


const char *ssid = "DeliveryBox";
const char *password = "12345678";
const char *htmlContent = R"(
<!DOCTYPE html>
<html>
<body>

<h2>ESP32 Configuration</h2>

<form action="/save" method="post">
  Wi-Fi SSID: <input type="text" name="ssid"><br>
  Wi-Fi Password: <input type="password" name="password"><br>
  Telegram Bot ID: <input type="text" name="bot_id"><br><br>
  <input type="submit" value="Save">
</form>

</body>
</html>
)";

#define echoPin 32               // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN
#define trigPin 33  
long thresh = 15;              // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN
long duration, distance;


#define weightPin 25
#define tarePin 26

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  3 // three columns

//put your telegram bot token here
#define BOTtoken "*********" 
// 1. Open Telegram and search for "BotFather"
// 2. Start a conversation with BotFather and type `/newbot`
// 3. Follow the prompts to name your bot and create a unique username (must end with "bot")
// 4. BotFather will provide your bot token (e.g., 123456789:ABCDEFghIJKLmnoPQRstuvWXYZ12345)
// 5. Copy the token and store it securely, you'll use it to interact with the Telegram API


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;

const int ledPin = 27;
const int buzzer = 15;

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte pin_rows[ROW_NUM]      = {19, 18, 5, 17}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 4, 0};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

bool ledState = LOW;
bool buzzerState = LOW;
String otp = "";
String userEnteredOTP = "";
const int maxAttempts = 5; // Maximum allowed attempts
int attempts = 0; // Counter for attempts
String chat_id = "";
String CHAT_ID;

AsyncWebServer server(80);

void handleNewMessages(int numNewMessages) {

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;

    if (text == "/generate_otp") {
    String OTP = generateOTP();
    String message = "Your OTP is: " + OTP;
    Serial.print("Generated OTP: ");
    Serial.println(OTP); 
    bot.sendMessage(chat_id, message, "");
    }
    if (text == "/off_b") {
    digitalWrite(buzzer, LOW);
    bot.sendMessage(chat_id, "Buzzer is turned off.", "");
    }
    if (text == "/state") {
      if (digitalRead(ledPin)){
        bot.sendMessage(chat_id, "BOX is OPEN", "");
      }
      else{
        bot.sendMessage(chat_id, "BOX is CLOSED", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(weightPin, INPUT);
  pinMode(tarePin, OUTPUT);
  digitalWrite(ledPin, ledState);
  digitalWrite(buzzer, buzzerState);

//%%%%%%%%%%%%%%%%%
  //%display
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  delay(2000);
  

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("DeliBox");
  display.display(); 
  delay(1000);
 // %%%%%%%%%%%%%%%%%%%%%%
 
  WiFi.softAP(ssid, password);
  Serial.println("Access Point IP address: " + WiFi.softAPIP().toString());
  displayText("Enter Cred");
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", htmlContent);
  });
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) {
    String wifiSSID = request->arg("ssid");
    String wifiPassword = request->arg("password");
    CHAT_ID = request->arg("bot_id");
    displayText("Connecting");
    saveCredentials(wifiSSID.c_str(), wifiPassword.c_str(), CHAT_ID.c_str());
  });
  server.begin();
}

void distanceCount(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 58.2;
}

void loop() {
  /*distanceCount();
  if(distance > thresh){
    //buzzer
    digitalWrite(buzzer, HIGH);
    bot.sendMessage(chat_id, "Box has been breached!!", "");
  }
  //digitalWrite(buzzer, LOW);*/
  verifyOTP();  
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void saveCredentials(const char *ssid, const char *password, const char *botID) {
  // Store the credentials in EEPROM, SPIFFS, or any other suitable storage mechanism
  // For simplicity, this example uses hardcoded storage (not recommended for production)
  // You need to implement your own secure storage logic here
  Serial.println("Saving credentials:");
  Serial.println("Wi-Fi SSID: " + String(ssid));
  Serial.println("Wi-Fi Password: " + String(password));
  Serial.println("Telegram Bot ID: " + String(botID));

  // In a real-world scenario, use secure storage like EEPROM, SPIFFS, etc.

  // Turn off the Access Point
  WiFi.softAPdisconnect(true);

  // Connect to the provided Wi-Fi network
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi. IP address: " + WiFi.localIP().toString());
  displayText("Connected");
  delay(2000);
  displayText("Welcome");
}

String generateOTP() {
  String newOTP = "";
  for (int i = 0; i < 6; ++i) {
    int digit = random(10); // Generates a random number between 0 and 9 (inclusive)
    newOTP += String(digit);
  }
  otp = newOTP; // Update the global otp variable with the new OTP
  return otp;
}

void verifyOTP() {

  if (attempts >= maxAttempts) {
    Serial.println("Maximum attempts reached. Access denied.");
    bot.sendMessage(chat_id, "Too many failed attempts. Access denied!", "");
    otp = "";
    attempts = 0;
    userEnteredOTP = "";
    return;
  }
  
  if (otp.length() != 0) {
    Serial.print("Enter OTP: ");
    displayText("Enter OTP:");
    while (userEnteredOTP.length() < 6) {
      char key = keypad.getKey();
      
      if(key){
        userEnteredOTP += key;
        OTPdisplay(userEnteredOTP);
        Serial.print(key);
      }
    } 
    
    Serial.print("\n");     
             
      if (userEnteredOTP == otp) {
        Serial.println("OTP verified. Access granted!");
        digitalWrite(tarePin, LOW);
        delay(200);
        digitalWrite(tarePin, HIGH);

        ledState = !ledState; // Toggle LED state
        digitalWrite(ledPin, ledState);
        displayText("Box Opened");
        bot.sendMessage(chat_id, "OTP matched. Box opened.", "");


        delay(15000);

        distanceCount();
        while(distance > thresh){
          //buzzer
          if(buzzerState == LOW){
            buzzerState = !buzzerState;
            digitalWrite(buzzer, buzzerState);
            displayText("CLOSE BOX!");
          }
          distanceCount();
          delay(1000);
        }
        if(buzzerState == HIGH){
        buzzerState = !buzzerState;
        }
        digitalWrite(buzzer, buzzerState);
        delay(1000);
        ledState = !ledState; // Toggle LED state
        digitalWrite(ledPin, ledState);
        displayText("Box Closed");
        
        Serial.println("After 15s box is autometically locked.");
        bot.sendMessage(chat_id, "Box is closed", "");
        int weightSignal = digitalRead(weightPin);
        if(weightSignal)
        {
          // buzzerState = !buzzerState;
          digitalWrite(buzzer, HIGH);
          bot.sendMessage(chat_id, "No parcel detected.");
        }
        else
        {
          bot.sendMessage(chat_id, "Parcel detected.");

        }
        otp = "";
        attempts = 0;
        delay(2000);
        displayText("Thanks");
        delay(2000);
        //displayText("DeliBox");
        displayText("Welcome");
      } else {
        attempts++;
        Serial.print("Incorrect OTP. Attempts left: ");
        displayText("Wrong OTP");
        delay(1000);
        
        Serial.println(maxAttempts - attempts);
        wrongOTPmessage(maxAttempts - attempts);
        delay(1000);
        if (attempts >= maxAttempts) {
          Serial.println("Maximum attempts reached. Access denied.");
          displayText("DENIED!");
          otp = "";
          attempts = 0;
          userEnteredOTP = "";          
          return;
        }
      }
      userEnteredOTP = ""; // Reset the user-entered OTP
    }
  }

  //#include <Fonts/FreeSans9pt7b.h>
void displayText(String message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
 // display.setTextWrap(false);

  display.setCursor(0, 0);
  // Display static text
  display.println(message);
  display.display();
  // display.startscrollleft(0x00, 0x0F);

  // delay(5000);
  // display.stopscroll();
}

void wrongOTPmessage(int attempt) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
 // display.setTextWrap(false);

  display.setCursor(0, 0);
  // Display static text
  String text = String(attempt) + " attempts";
  display.println(text);
    display.setTextSize(2);

  display.setCursor(0, 18);
  // Display static text
  display.println("are left");
  display.display();
  // display.startscrollleft(0x00, 0x0F);

  // delay(5000);
  // display.stopscroll();
}

void OTPdisplay(String OTP) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
 // display.setTextWrap(false);

  display.setCursor(0, 0);
  // Display static text
  String text = "Enter OTP:";
  display.println(text);
    display.setTextSize(2);

  display.setCursor(0, 18);
  // Display static text
  display.println(OTP);
  display.display();
  // display.startscrollleft(0x00, 0x0F);

  // delay(5000);
  // display.stopscroll();
}
