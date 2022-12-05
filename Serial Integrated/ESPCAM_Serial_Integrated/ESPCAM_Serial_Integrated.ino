#include <Arduino.h>
#include <ESP32QRCodeReader.h>
#include "esp_camera.h"
#include "driver/rtc_io.h"
#include <HardwareSerial.h>
#define FLASH_GPIO_NUM 4

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
camera_config_t config;

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      if (qrCodeData.valid)
      {
        Serial.println((const char *)qrCodeData.payload);
        pinMode(FLASH_GPIO_NUM, OUTPUT);
        
        digitalWrite(FLASH_GPIO_NUM, LOW);
      }
      else
      {
        Serial.print("X");
      }
      delay(500);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(FLASH_GPIO_NUM, OUTPUT);
  
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10; //10-63 lower number means higher quality
    config.fb_count = 2;
  }else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  reader.setup();

  Serial.println("Setup QRCode Reader");

  reader.beginOnCore(1);

  Serial.println("Begin on Core 1");

  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop()
{
  delay(1000);
}
