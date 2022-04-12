//Connections of the RFID-RC522 module to LoLin Node MCU v3 ESP8266:
//  SDA   D4
//  SCK   D5
//  MOSI  D7
//  MISO  D6
//  RST   D3
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN 2  //D4  / 2 / 10
#define RST_PIN 0 //D3  / 0 / 9    // Configurable, see typical pin layout above
#define SERVER_IP "10.135.254.53"
#define RELAY_PIN 4
#define OPEN_FOR_MILISECONDS 1000

const char* ssid = "dd-wrt";
const char* password = "telephone";
long int opened_millis = 0;
String payload = "";


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void setup() {
  pinMode(5, OUTPUT); // SCK
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0);
  Serial.begin(115200);		// Initialize serial communications with the PC
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  WiFiClient client;
  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  opened_millis = millis();
}

void loop() {
  /*if (millis() - opened_millis > OPEN_FOR_MILISECONDS) {
    digitalWrite(RELAY_PIN, 0);
  }*/
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  digitalWrite(5, 1);
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  Serial.println();
  Serial.print(" UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "+0" : "+"));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  String Link = "http://" SERVER_IP ":8082/validate?UID=" + content.substring(1);
  Serial.println("Link: " + Link);

  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, Link); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header and body
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        Serial.println(" Access Granted ");
        digitalWrite(RELAY_PIN, 1);
        digitalWrite(5, 0);
        payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
        //delay(OPEN_FOR_MILISECONDS+500);
      }
    } else {
      Serial.println(" Access Denied ");
      digitalWrite(RELAY_PIN, 0);
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
  }
  //delay(OPEN_FOR_MILISECONDS);
  /*if (millis() - opened_millis > OPEN_FOR_MILISECONDS) {
    digitalWrite(5, 0);
   */
   
  digitalWrite(5, 0);
  ESP.reset();
}
//commit dellay in the end;
//+ dellay when the rellay in activated
// SCK?
