//LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Setting Komunikasi ESP
HardwareSerial SerialCAM(1);
#define RX2 13
#define TX2 12

#define FLASH_GPIO_NUM 2

const unsigned int MAX_MESSAGE_LENGTH = 100;

//relay
#define relay 33

//Cek Alat
#define IR_1 32
#define IR_2 35
#define IR_3 34

//Buzzer
#define BUZZER 14 
int freq = 1000;
int channel = 1;
int resolution = 8;

//Limit Switch
typedef struct switches{
  const uint8_t pin;
  int pressed;
} Switch;

Switch pintu = {27, 0};
int statusPintu = 0;

//Switch debounce
unsigned long switch_time = 0;  
unsigned long last_switch_time = 0; 

long oldTime = 0;

//User 
typedef struct user_session{
  String qr_code;
  int id_user;
  String username;
  int id_alat;      // 1 atau 2 atau 3
  int request_alat; // 0 untuk pinjam, 1 untuk kembali
} User_session;

User_session current_user;
String id_user;

//Fungsi Interrupt Pintu kalau pintu ketutup
void IRAM_ATTR runaway(){
  switch_time = millis();
  if(switch_time - last_switch_time > 250){
    ledcWriteTone(channel, 0);
    pintu.pressed = 1;
    last_switch_time = switch_time;
    }
  }

void setup() {
  Serial.begin(115200);
  SerialCAM.begin(115200, SERIAL_8N1, RX2, TX2);
  Serial.println("Serial Txd is on pin: "+String(TX2));
  Serial.println("Serial Rxd is on pin: "+String(RX2));

  //Setup Infrared Alat
  pinMode(IR_1, INPUT); //0 alat ada, 1 alat tidak ada
  pinMode(IR_2, INPUT); 
  pinMode(IR_3, INPUT); 

  //Setup relay
  pinMode(relay, OUTPUT);
  digitalWrite(relay,HIGH);
  
  //Setup Buzzer
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(BUZZER, channel);

  //Setup LCD
  lcd.begin();
  
  //Setup Switch Pintu & interruptnya
  pinMode(pintu.pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pintu.pin), runaway, RISING);
  
  pinMode(FLASH_GPIO_NUM, OUTPUT);
}

//Pilih Alat yang pengen dipinjam
int pilih_alat(User_session user){
  if(user.id_alat == 1){
    return IR_1; 
    }
  else if(user.id_alat == 2){
    return IR_2;
    }
  else if(user.id_alat == 3){
    return IR_3;
    }
  }

//Pilih Alat yang ga pengen dipinjam
int pilih_nonalat1(User_session user){
  if(user.id_alat == 1){
    return IR_2; 
    }
  else if(user.id_alat == 2){
    return IR_3;
    }
  else if(user.id_alat == 3){
    return IR_1;
    }
  }

//Pilih Alat yang ga pengen dipinjam
int pilih_nonalat2(User_session user){
  if(user.id_alat == 1){
    return IR_3; 
    }
  else if(user.id_alat == 2){
    return IR_1;
    }
  else if(user.id_alat == 3){
    return IR_2;
    }
  }

//Fungsi Pengambilan Barang
void ambil_barang(User_session user){
  long currentMillis  = millis();
  oldTime = currentMillis;
  int pilihan = pilih_alat(user);
  int non_pilihan1 = pilih_nonalat1(user);
  int statusANP1Awal = digitalRead(non_pilihan1);
  int non_pilihan2 = pilih_nonalat2(user);
  int statusANP2Awal = digitalRead(non_pilihan2);
  digitalWrite(relay, LOW);
  //Jika pintu masih tertutup
  do{
    Serial.println("Silahkan Buka Pintu");
    lcd.backlight();
    lcd.clear
      while(Serial.available()>0){
        lcd.write(Serial.read());
      }
    }
  while(digitalRead(pintu.pin) == LOW);  

  //Jika pintu sudah terbuka 
  while(true){
    if(digitalRead(pintu.pin) == HIGH){        //Jika Pintu Terbuka, Perintahkan untuk ambil alat
      currentMillis  = millis();
      int statusAP = digitalRead(pilihan);
      int statusANP1 = digitalRead(non_pilihan1);
      int statusANP2 = digitalRead(non_pilihan2);
      Serial.println(currentMillis-oldTime);
      
      if(currentMillis - oldTime > 30000){      //Waktu 30 detik untuk ambil dan reminder apabila terbuka terus
        while(digitalRead(pintu.pin) == HIGH){   //Perintahkan orang untuk tutup pintu
          ledcWriteTone(channel, 1000);
          Serial.println("Segera Tutup Pintu");
          lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
          if(pintu.pressed == 1 && digitalRead(pintu.pin) == LOW){
            oldTime = millis();
            pintu.pressed = 0;
            digitalWrite(relay, HIGH);
            return;
          }
        }
      }
      else if(statusAP == 1 && statusANP1 == statusANP1Awal && statusANP2 == statusANP2Awal){    //Apabila alat sudah diambil, perintahkan untuk tertutup
        Serial.println("Silahkan tutup pintu");
        lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }  
        }
      else if(statusANP1 != statusANP1Awal || statusANP2 != statusANP2Awal){                   //Apabila salah ambil, perintahkan untuk taruh kembali
        while(statusANP1 != statusANP1Awal || statusANP2 != statusANP2Awal){
          ledcWriteTone(channel, 1000);
          Serial.println("Kembalikan Alat Yang dipegang!!");
          lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
          statusANP1 = digitalRead(non_pilihan1);
          statusANP2 = digitalRead(non_pilihan2);
          }
        ledcWriteTone(channel, 0);
      }
      else{
        Serial.println("Silahkan ambil alat");
        lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
        } 
      if(pintu.pressed == 1 && digitalRead(pintu.pin) == LOW){   //Apabila pintu tertutup, keluar
        oldTime = millis();
        pintu.pressed = 0;
        digitalWrite(relay, HIGH);
        return;
        }   
    }
  }
}

//Fungsi Pengembalian Barang
void taruh_barang(User_session user){
  long currentMillis  = millis();
  oldTime = currentMillis;
  int pilihan = pilih_alat(user);
  int non_pilihan1 = pilih_nonalat1(user);
  int statusANP1Awal = digitalRead(non_pilihan1);
  int non_pilihan2 = pilih_nonalat2(user);
  int statusANP2Awal = digitalRead(non_pilihan2);
  digitalWrite(relay, LOW);
  //Jika pintu masih tertutup
  do{
    Serial.println("Silahkan Buka Pintu");
    lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
    }
  while(digitalRead(pintu.pin) == LOW);  

  //Jika pintu sudah terbuka 
  while(true){
    if(digitalRead(pintu.pin) == HIGH){        //Jika Pintu Terbuka, Perintahkan untuk ambil alat
      currentMillis  = millis();
      int statusAP = digitalRead(pilihan);
      int statusANP1 = digitalRead(non_pilihan1);
      int statusANP2 = digitalRead(non_pilihan2);
      Serial.println(currentMillis-oldTime);
      
      if(currentMillis - oldTime > 30000){      //Waktu 30 detik untuk ambil dan reminder apabila terbuka terus
        while(digitalRead(pintu.pin) == HIGH){   //Perintahkan orang untuk tutup pintu
          ledcWriteTone(channel, 1000);
          Serial.println("Segera Tutup Pintu");
          lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
          if(pintu.pressed == 1 && digitalRead(pintu.pin) == LOW){
            oldTime = millis();
            pintu.pressed = 0;
            digitalWrite(relay, HIGH);
            return;
          }
        }
      }
      else if(statusAP == 0 && statusANP1 == statusANP1Awal && statusANP2 == statusANP2Awal){    //Apabila alat sudah ditaroh, perintahkan untuk tertutup
        Serial.println("Silahkan tutup pintu");
        lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }  
        }
      else if(statusANP1 != statusANP1Awal || statusANP2 != statusANP2Awal){                   //Apabila salah ambil, perintahkan untuk taruh kembali
        while(statusANP1 != statusANP1Awal || statusANP2 != statusANP2Awal){
          ledcWriteTone(channel, 1000);
          Serial.println("Kembalikan Alat Yang dipegang!!");
          lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
          statusANP1 = digitalRead(non_pilihan1);
          statusANP2 = digitalRead(non_pilihan2);
          }
        ledcWriteTone(channel, 0);
      }
      else{
        Serial.println("Silahkan taruh alat");
        lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
        } 
      if(pintu.pressed == 1 && digitalRead(pintu.pin) == LOW){   //Apabila pintu tertutup, keluar
        oldTime = millis();
        pintu.pressed = 0;
        digitalWrite(relay, HIGH);
        return;
        }   
    }
  }
}

//Fungsi ambil alat atau kembalikan alat
void cek_request(User_session user){
  Serial.println("Checking Request");
  delay(500);
  if(user.request_alat == 0){        //Jika user ingin ambil alat
    Serial.println("Request Ambil Alat");
    delay(500);
    ambil_barang(user);
    }
   else if(user.request_alat == 1){  //Jika user ingin kembalikan alat
    Serial.println("Request Kembalikan Alat");
    delay(500);
    taruh_barang(user);
    }
  }

void loop() {
  Serial.println("start");
  delay(1000);
  while(true){
    if(SerialCAM.available() > 0){
    //Create a place to hold the incoming message
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;
    static unsigned int last_pos= 0;
    
    //Read the next available byte in the serial receive buffer
    char inByte = SerialCAM.read();

    //Message coming in (check not terminating character/Serial.Println) and guard for over message size
    if( inByte == 'X'){
      Serial.println("Gagal Baca");
      lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
      //Command untuk esp32cam bisa scan lagi
      Serial.println("sending Command to ESP32CAM");
      SerialCAM.print('0');
      message_pos = 0;
      break;
      }
    else if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) ){
     //Add the incoming byte to our message
     message[message_pos] = inByte;
     message_pos++;
    }
    //Full message received...
    else{
       //Add null character to string
      message[message_pos] = '\0';
       //Print the message (or do other things)
      Serial.print("Success: ");
      Serial.println(message);
      //Jika Merupakan Kode unik dari QRCode, Kirim ke Backend!!
      if(isDigit(message[0]) && message[message_pos] == '\0'){
        //Kode Kirim ke Backend, verifikasi dengan database, 

        //untuk sementara dummy data ya
        current_user.qr_code = "2222";
        current_user.id_user = 18;
        current_user.username = "GeryGirsang";
        current_user.id_alat = 1;
        current_user.request_alat = 0;
            
        //Udah dapet data user, cek requestnya apa, pindah fungsi biar gampang
        Serial.println("User Detected");
        lcd.backlight();
          lcd.clear
          while(Serial.available()>0){
            lcd.write(Serial.read());
          }
        delay(1000);
        cek_request(current_user);
        digitalWrite(FLASH_GPIO_NUM, HIGH);
        delay(1000);
        digitalWrite(FLASH_GPIO_NUM, LOW);

        //Command untuk esp32cam bisa scan lagi
        Serial.println("sending Command to ESP32CAM");
        SerialCAM.print('0');
        message_pos = 0;
        break;
        } 
      message_pos = 0;
      delay(50);
      }  
    }
  }
}
