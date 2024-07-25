#include <Arduino.h>
#include <esp_camera.h>
#include <SPI.h>

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

#define LED_GPIO_NUM   4

// SPI Pins
#define SPI_SCK 14
#define SPI_MISO 12
#define SPI_MOSI 13
#define SPI_CS 15

#define BUFFER_SIZE 14000

const char END_MARKER[] = "\n";
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Configure the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA; // Reduce frame size for stability
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Adjust settings for PSRAM
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_QVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  } else {
    Serial.println("Camera initialization successful");
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  if (config.pixel_format == PIXFORMAT_GRAYSCALE) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }
camera_fb_t * fb = NULL;

//Serial.println("Capturing image...");
    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    else {
      Serial.println(sizeof(fb->len));
    }

Serial.print("Image size: ");
Serial.println(fb->len); // Print the length of the image buffer

// Print the first few bytes of the image buffer (for debugging purposes)
const int bytesToPrint = fb->len; // Adjust the number of bytes to print as needed
/*Serial.println("Image data (All bytes):");
for (int i = 0; i < bytesToPrint; i += 2) {
  for (int j = 0; j < 2 && (i + j) < bytesToPrint; j++) {
    Serial.print(fb->buf[i + j], HEX);
  }
  Serial.print(" ");
}*/

  // Print image data in hex format
 // Serial.println("Image data (All bytes):");
  for (int i = 0; i < fb->len; i++) {
    //if (i % 2 == 0 && i > 0) {
     // Serial.print(" ");
    //}
    if (fb->buf[i] < 0x10) {
      Serial.print("0"); // Print leading zero for single digit hex values
    }
    Serial.print(fb->buf[i], HEX);
  }
  Serial.println();

/*Serial.println("Image data (All bytes):");
for (int i = 0; i < bytesToPrint; i++) {
  Serial.print(fb->buf[i], HEX);
  //Serial.print(" ");
}*/
//Serial.println();
  // Initialize SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  SPI.setFrequency(1000000);
  Serial.println("SPI initialization successful");

  // Transmit image size over SPI
 /* Serial.println("Transmitting image size over SPI...");
  uint32_t imageSize = fb->len;
  digitalWrite(SPI_CS, LOW); // Select SPI device
  SPI.transfer((uint8_t*)&imageSize, sizeof(imageSize)); // Send size as 4 bytes
  digitalWrite(SPI_CS, HIGH); // Deselect SPI device


  uint32_t received;
 // SPI.readBytes((uint8_t*)&received, sizeof(received));
  // Delay before sending image data (adjust as needed)
  delay(5000);
*/

 // Transmit image over SPI
  Serial.println("Transmitting image over SPI...");
  digitalWrite(SPI_CS, LOW); // Select SPI device
  SPI.writeBytes((uint8_t*)&fb->len, sizeof(fb->len));
  SPI.writeBytes(fb->buf, fb->len); // Send image data
    // Transmit zeroes to pad the buffer to 10,000 bytes
  int padding_size = BUFFER_SIZE - fb->len;
  if (padding_size > 0) {
    uint8_t padding[padding_size] = {0}; // Create padding buffer with zeroes
    SPI.writeBytes(padding, padding_size); // Send padding data
  }
  //delay(100);
  digitalWrite(SPI_CS, HIGH); // Deselect SPI device
  Serial.println("Transmitting finish");
  // Release the frame buffer back to the camera
  esp_camera_fb_return(fb);

}



void loop() {

 

  // Delay before capturing the next image
  delay(5000); // Adjust delay as needed
}
