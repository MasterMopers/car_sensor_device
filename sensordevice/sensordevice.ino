//camera libraries
#include <mcu_tmf882x_config.h>
#include <qwiic_i2c.h>
#include <qwiic_tmf882x.h>
#include <sfe_arduino.h>
#include <SparkFun_TMF882X_Library.h>
#include <tmf882x_interface.h>
#include <tof_bin_image.h>
#include <tof_factory_cal.h>
#include "esp_camera.h"
#include <WiFi.h>
//camera model
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

//sensor libraries
#include "Arduino.h"
#include <Wire.h>
#include "SparkFun_TMF882X_Library.h" //http://librarymanager/All#SparkFun_Qwiic_TMPF882X

//sensor pin definitions
#define I2C_SDA 14
#define I2C_SCL 15
TwoWire I2CSensors = TwoWire(0);
SparkFun_TMF882X  myTMF882X;
// Structure to hold the measurement results - this is defined by the TMF882X SDK.
static struct tmf882x_msg_meas_results myResults;

//esp32 wifi credentials
const char* ssid = "gapi2022";
const char* password = "Matrim59";

//camera thing
void startCameraServer();


void setup(){
//camera setup
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");


//sensor setup
    I2CSensors.begin(I2C_SDA, I2C_SCL, 100000ul);
    delay(1000);
    Serial.begin(115200);
    Serial.println("");

    Serial.println("In setup");
    Serial.println("==============================");

    // Initialize the TMF882X device
    //0x76, &I2CSensors - in the parenthesis
    if(!myTMF882X.begin())
    {
        Serial.println("Error - The TMF882X failed to initialize - is the board connected?");
        while(1);
    }else
        Serial.println("TMF882X started.");

    // The device is now ready for operations
}

void loop()
{
    delay(2000);
    // get a Measurement
    if(myTMF882X.startMeasuring(myResults))
    {
        // print out results
        Serial.println("Measurement:");
        Serial.print("     Result Number: "); Serial.print(myResults.result_num);
        Serial.print("  Number of Results: "); Serial.println(myResults.num_results);       

        for (int i = 0; i < myResults.num_results; ++i) 
        {
            Serial.print("       conf: "); Serial.print(myResults.results[i].confidence);
            //confidence level (0,255) with 255 being the highest
            Serial.print(" distance mm: "); Serial.print(myResults.results[i].distance_mm);
            Serial.print(" channel: "); Serial.print(myResults.results[i].channel);
            Serial.print(" sub_capture: "); Serial.println(myResults.results[i].sub_capture);   

        }
        Serial.print("     photon: "); Serial.print(myResults.photon_count);    
        Serial.print(" ref photon: "); Serial.print(myResults.ref_photon_count);
        Serial.print(" ALS: "); Serial.println(myResults.ambient_light); Serial.println();

    }
//the delay for the camera needs to be 10000

}
