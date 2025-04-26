
#include "ledManager.h"
#include "buttonManager.h"
#include "mqttManager.h"
#include "wifiManager.h"
#include "otaManager.h"
#include "StreamIO.h"
#include "VideoStream.h"
#include "RTSP.h"
#include "NNFaceDetectionRecognition.h"
#include "VideoStreamOverlay.h"
#include "fileManager.h"
#include "consoleNet.h"

void consoleCommandHandler(String cmd);
LEDManager ledManager;
ButtonManager buttonManager(BUTTON_PIN);
WiFiManager wifiManager;
MQTTManager mqttManager;
OTA_Manager otaManager(3000,"192.168.1.198");
consoleNet console;;

/************************************************************Camera Section started********************************************************/
#define CHANNELVID  0    // Channel for RTSP streaming
#define CHANNELJPEG 1    // Channel for taking snapshots
#define CHANNELNN   3    // RGB format video for NN only available on channel 3
// Customised resolution for NN
#define NNWIDTH  576
#define NNHEIGHT 320
// Select the maximum number of snapshots to capture
#define MAX_UNKNOWN_COUNT 5
#define FILE_RD_BUF_SIZE 1024*1024

VideoSetting configVID(VIDEO_FHD, CAM_FPS, VIDEO_H264, 0);
VideoSetting configJPEG(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);
VideoSetting configNN(NNWIDTH, NNHEIGHT, 10, VIDEO_RGB, 0);
NNFaceDetectionRecognition facerecog;
RTSP rtsp;
StreamIO videoStreamer(1, 1);
StreamIO videoStreamerFDFR(1, 1);
StreamIO videoStreamerRGBFD(1, 1);

uint32_t img_addr = 0;
uint32_t img_len = 0;
bool unknownDetected = false;
int unknownCount = 0;

unsigned char fileRedBuf[FILE_RD_BUF_SIZE];
void cameraModuleBegin()
{
   // Configure camera video channels with video format information
    Camera.configVideoChannel(CHANNELVID, configVID);
    Camera.configVideoChannel(CHANNELJPEG, configJPEG);
    Camera.configVideoChannel(CHANNELNN, configNN);
    Camera.videoInit();

    // Configure RTSP with corresponding video format information
    rtsp.configVideo(configVID);
    rtsp.begin();

    // Configure Face Recognition model
    facerecog.configVideo(configNN);
    facerecog.modelSelect(FACE_RECOGNITION, NA_MODEL, DEFAULT_SCRFD, DEFAULT_MOBILEFACENET);
    facerecog.begin();
    facerecog.setResultCallback(FRPostProcess);

    // Configure StreamIO object to stream data from video channel to RTSP
    videoStreamer.registerInput(Camera.getStream(CHANNELVID));
    videoStreamer.registerOutput(rtsp);
    if (videoStreamer.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Configure StreamIO object to stream data from RGB video channel to face detection
    videoStreamerRGBFD.registerInput(Camera.getStream(CHANNELNN));
    videoStreamerRGBFD.setStackSize();
    videoStreamerRGBFD.setTaskPriority();
    videoStreamerRGBFD.registerOutput(facerecog);
    if (videoStreamerRGBFD.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    // Start data stream from video channel
    Camera.channelBegin(CHANNELVID);
    Camera.channelBegin(CHANNELJPEG);
    Camera.channelBegin(CHANNELNN);

    // Start OSD drawing on RTSP video channel
    OSD.configVideo(CHANNELVID, configVID);
    OSD.begin();
    //restore the registered face
    facerecog.restoreRegisteredFace();
}

void cameraLoop(){
  OSD.createBitmap(CHANNELVID);
  OSD.update(CHANNELVID);
}

uint32_t gVal=0;
uint32_t bVal=0;
void faceRecogManager_toggleLed(int pin){
  if(pin == LED_G){
    gVal = !gVal;
    digitalWrite(LED_G, gVal);
  }else if(pin==LED_B){
    bVal = !bVal;
    digitalWrite(LED_B, bVal);
  }
}

void faceRecogManager_notifyDetection(PUB_TOPIC_TYPE topic, char* msg){
  Serial.println(msg);
  // Publish MQTT message
  mqttManager.publishMessage( topic, msg);
  //Serial.println("Button pressed. Setting LED pattern for 10 seconds.");
}

const int INTERVEL_MSEC= 5000; 
unsigned long previousMilli = 0;

void FRPostProcess(std::vector<FaceRecognitionResult> results){
  uint16_t im_h = configVID.height();
  uint16_t im_w = configVID.width();
  char msg[128];

  unsigned long currentMilli = millis();

  printf("Total number of faces detected = %d\r\n", facerecog.getResultCount());
  OSD.createBitmap(CHANNELVID);

  if (facerecog.getResultCount() > 0) {
    for (int i = 0; i < facerecog.getResultCount(); i++) {
      FaceRecognitionResult item = results[i];
      // Result coordinates are floats ranging from 0.00 to 1.00
      // Multiply with RTSP resolution to get coordinates in pixels
      int xmin = (int)(item.xMin() * im_w);
      int xmax = (int)(item.xMax() * im_w);
      int ymin = (int)(item.yMin() * im_h);
      int ymax = (int)(item.yMax() * im_h);

      uint32_t osd_color;
      if (String(item.name()) == String("unknown")) {
        osd_color = OSD_COLOR_RED;    
      } else {
          osd_color = OSD_COLOR_GREEN;     
      }
      // allow every INTERVEL_MSEC process the images
      if( currentMilli - previousMilli>= INTERVEL_MSEC){
        if (osd_color == OSD_COLOR_RED){
          unknownDetected = true;
          unknownCount++;
          if (unknownCount < (MAX_UNKNOWN_COUNT + 1)) {                     // Ensure number of snapshots under MAX_UNKNOWN_COUNT          
            String fName = "Stranger" + String(unknownCount) + ".jpg";    // Capture snapshot of stranger under name Stranger <No.>
            Camera.getImage(CHANNELJPEG, &img_addr, &img_len);
            fileWrite(fName,(uint8_t *)img_addr, img_len);   
            Serial.println(" A Stranger detected ") ;
            faceRecogManager_toggleLed(LED_B);
            ledManager.setLEDState(RED, true);
            sprintf(msg, "%s", "a stanger is on door");  
            // send notification
            faceRecogManager_notifyDetection(PUB_TOPIC_PERSON_UNKNOWN,msg);       
          }
        }else if (osd_color == OSD_COLOR_GREEN){
          sprintf(msg, "%s%s", item.name()," is on door");
          faceRecogManager_toggleLed(LED_G);
          ledManager.setLEDState(GREEN, true);   
          // send notification
          faceRecogManager_notifyDetection(PUB_TOPIC_PERSON_KNOWN,msg);   
        }
        previousMilli = millis();
        
      }
    
      // Draw boundary box
      printf("Face %d name %s:\t%d %d %d %d\n\r", i, item.name(), xmin, xmax, ymin, ymax);
      OSD.drawRect(CHANNELVID, xmin, ymin, xmax, ymax, 3, osd_color);

      // Print identification text above boundary box
      char text_str[40];
      snprintf(text_str, sizeof(text_str), "Face:%s", item.name());
      OSD.drawText(CHANNELVID, xmin, ymin - OSD.getTextHeight(CHANNELVID), text_str, osd_color);
    }
  }
  OSD.update(CHANNELVID);
}

/************************************************************Camera Section End***********************************************************/
void setup() {
  Serial.begin(115200);
  ledManager.begin();
  
  /*LEDPattern pattern = {HIGH_PRIORITY, {LOW, LOW, HIGH}, {0, 0, 0}};
  ledManager.setNextPattern(pattern);
  ledManager.updateLEDPattern(); 
  */
  ledManager.setLEDState(BLUE,HIGH);
  ledManager.setLEDState(GREEN,LOW);
  ledManager.setLEDState(RED,LOW);
  wifiManager.begin();
  
  ledManager.setLEDState(BLUE,HIGH);
  ledManager.setLEDState(GREEN,LOW);
  ledManager.setLEDState(RED,LOW);
  Serial.println("Connecting to WiFi...");
 
  mqttManager.begin();
  ledManager.setLEDState(BLUE,LOW);
  ledManager.setLEDState(GREEN,HIGH);
  ledManager.setLEDState(RED,LOW);
  
  buttonManager.begin();
  //file system  
  fsBegin();
  //camera module
  //cameraModuleBegin();
  console.setCallback(consoleCommandHandler);
  Serial.println(" device configured and ready to use");
}

void consoleCommandHandler(String input){
 
  input.trim();
  if (input.startsWith(String("REG="))) {
      String name = input.substring(4);
      Serial.println("register");
      console.sendResponse("OK", 3);
      facerecog.registerFace(name);
  } else if (input.startsWith(String("DEL="))) {
      String name = input.substring(4);
      Serial.println("Delete");
      console.sendResponse("OK", 3);
      facerecog.removeFace(name);
  } else if (input.startsWith(String("RESET"))) {
      facerecog.resetRegisteredFace();
  } else if (input.startsWith(String("BACKUP"))) {
      facerecog.backupRegisteredFace();
  } else if (input.startsWith(String("RESTORE"))) {
      facerecog.restoreRegisteredFace();
      Serial.println("restore");
      console.sendResponse("OK", 3);
  }
  else if (input.startsWith(String("OTA"))) {           
    if(!otaManager.startOTA()){
      Serial.println("OTA failed retry");
    }else{
      LEDPattern pattern = {2, {HIGH, HIGH, HIGH}, {500, 900, 1300}};
      ledManager.setNextPattern(pattern);
      while( true){
        ledManager.loop();
      }
    }
  } 
  else if(input.startsWith(String("DIR"))){
    char buff[512];
    
    int len= getDirList(buff, 512);
    console.sendResponse(buff, len);    
  }
  else if(input.startsWith(String("FREAD"))){
    String fName = input.substring(6);
    Serial.print("File Name = ");
    Serial.println(fName);
    int fileLen = fileRead(fName, fileRedBuf, FILE_RD_BUF_SIZE);
    console.sendResponse((char*)fileRedBuf, fileLen);    
  }  
  else if(input.startsWith(String("FREMOVE"))){
    String fName = input.substring(8);
    Serial.print("File Name = ");
    Serial.println(fName);
    if(fileRemove(fName)){
      console.sendResponse("OK", 2);    
    }else{
    console.sendResponse("NOK", 2);    
    }
  } 
  else{
    if(!console.sendResponse("NOK", 3)){
      Serial.println("Faild to send Udp command response");
    }
  }
}


unsigned long buttonPressTime = 0; // Tracks the time when the button was pressed
bool isButtonPressed = false;      // Tracks if the button press is being handled
const int ledPatternOffTime = 10000; // 10 seconds

void loop() {
  if (buttonManager.isButtonPressed()) {
    if (!isButtonPressed) { // Only handle the button press once
      isButtonPressed = true;
      buttonPressTime = millis(); // Record the time of the button press

      // Set the LED pattern
      LEDPattern pattern = {2, {HIGH, HIGH, HIGH}, {500, 900, 1300}};
      ledManager.setNextPattern(pattern);

      // Publish MQTT message
      mqttManager.publishMessage( PUB_TOPIC_BELL,"Button pressed. Setting LED pattern for 10 seconds.");
      Serial.println("Button pressed. Setting LED pattern for 10 seconds.");
    }
  }
  // Check if the LEDs need to be turned off after the specified time
  if (isButtonPressed && (millis() - buttonPressTime >= ledPatternOffTime)) {
    ledManager.setLEDStateAll(LOW); // Turn off all LEDs
    isButtonPressed = false;        // Reset the button press state
    Serial.println("LEDs turned off after 10 seconds.");
  }
  
  console.consoleNetLoop();

  ledManager.loop();
  //cameraLoop();
  //mqttManager.loop();
  delay(10);
}