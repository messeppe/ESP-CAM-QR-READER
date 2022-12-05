#include <LiquidCrystal_I2C.h>

HardwareSerial SerialCAM(1);
#define RX2 13
#define TX2 12

int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

const unsigned int MAX_MESSAGE_LENGTH = 50;

void setup() {
  Serial.begin(115200);
  SerialCAM.begin(115200, SERIAL_8N1, RX2, TX2);
  Serial.println("Serial Txd is on pin: "+String(TX2));
  Serial.println("Serial Rxd is on pin: "+String(RX2));
  lcd.init();
  lcd.backlight();
}

void loop() {
  while (SerialCAM.available() > 0)
 {
   //Create a place to hold the incoming message
   static char message[MAX_MESSAGE_LENGTH];
   static unsigned int message_pos = 0;
  
   //Read the next available byte in the serial receive buffer
   char inByte = SerialCAM.read();

   //Message coming in (check not terminating character/Serial.Println) and guard for over message size
   if ( inByte == 'X'){
      lcd.setCursor(0, 0);
      Serial.println("Gagal Baca");
      lcd.print("QR Code Tak Terbaca!");
      lcd.clear();
    }
    else if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
   {
     //Add the incoming byte to our message
     message[message_pos] = inByte;
     message_pos++;
//     Serial.println("test");
   }
   //Full message received...
   else
   {
     //Add null character to string
     message[message_pos] = '\0';

     //Print the message (or do other things)
     lcd.setCursor(0, 0);
     lcd.print("QR Code Terbaca!");
     Serial.println(message);

     //Reset for the next message
     message_pos = 0;
   }
 }
}
