/*
 * LED ROS controller for the Eye + Ear LEDs for the PoliV2 project. 
 * This file also handles the limit switch for the pan/tilt motor system.
 *
 * The LED libraries that can be used generally are APA102 and Adafruit_DotStar.
 * This file MUST be flashed to an Arduino Mega or else non-deterministic behavior will ensue.
 * 
 * The DOTSTAR, matrix LEDs (eyes) use hardware SPI to streamline. This requires that the 
 * clock pin (yellow) go to 52 (SCK) and the green pin go to 51 (MOSI).
 * 
 * Author Maxwell Svetlik, Prashant Rao 
 */

#include <APA102.h>

#include <ros.h>
#include <std_msgs/Int16.h>
#include <poli_msgs/LedEye.h>
#include <poli_msgs/LedEar.h>
#include <rosserial_arduino/Test.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#ifndef PSTR
#define PSTR // Make Arduhino Due happy
#endif

#define DATAPIN  11
#define CLOCKPIN 13

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
(uint8_t)8, (uint8_t)8, 2, 1, 
DS_TILE_TOP   + DS_TILE_RIGHT   + DS_TILE_ROWS   + DS_TILE_PROGRESSIVE +
DS_MATRIX_TOP + DS_MATRIX_RIGHT + DS_MATRIX_ROWS + DS_MATRIX_ZIGZAG,
DOTSTAR_BRG);

ros::NodeHandle  nh;
using rosserial_arduino::Test;

// Define which pins to use.
const uint8_t ears_dataPin = 8;
const uint8_t ears_clockPin = 9;
const uint8_t limitSwitchPin = 4;

// Create an object for writing to the LED strip.
APA102<ears_dataPin, ears_clockPin> ledStrip;

const uint16_t colors[] = {
  matrix.Color(255,255,255),
  matrix.Color(255, 0, 0), 
  matrix.Color(255, 0, 180), 
  matrix.Color(255, 0, 255), 
  matrix.Color(180, 0, 255), 
  matrix.Color(0,   0, 255), 
  matrix.Color(0, 180, 255), 
  matrix.Color(0, 255, 255), 
  matrix.Color(0, 255, 180),
  matrix.Color(0, 255, 0),
  matrix.Color(180, 255, 0),
  matrix.Color(255, 255, 0), 
  matrix.Color(255, 180, 0)};

//Delete this once refactor complete
const uint16_t mycolor = colors[4];

// Set the number of LEDs to control.
const uint16_t ledCount = 56;

// Create a buffer for holding the colors (3 bytes per color).
rgb_color ear_colors[ledCount];

// Set the brightness to use (the maximum is 31).
const uint8_t max_brightness = 10;
uint8_t brightness = 5;

//Control variables
const uint8_t EAR_SOLID = 2;
const uint8_t EAR_GRADIENT = 3;
const uint8_t EAR_BREATH= 4;

//Eye shape constants
const uint8_t NORMAL = 0;
const uint8_t CLOSE = 1;
const uint8_t CRY = 2;
const uint8_t SQUINT = 3;
const uint8_t WINK = 4;
const uint8_t MAD = 5;
const uint8_t SIDELEFT = 6;
const uint8_t HAPPY = 7;
const uint8_t SAD = 8;
const uint8_t SIDERIGHT = 9;
const uint8_t HEART = 10;
const uint8_t SLEEPY = 11;
const uint8_t HAPPY_WIDE = 12;
const uint8_t SLEEPY_WIDE = 13;
const uint8_t DEAD = 14;

//Mouth shape constants
const uint8_t FLAT = 0;
const uint8_t GRIN = 1;
const uint8_t GRIMACE = 2;
const uint8_t OPEN = 3;
const uint8_t LONGFACE = 4;
const uint8_t SMILE = 5;
const uint8_t FROWN = 6;
const uint8_t BIGOPEN = 7;
const uint8_t SQUIGGLE = 8;
const uint8_t WHISTLE = 9;

//Full face constants
const uint8_t HI = 0;

//Direction constants
const uint8_t CENTER = 0;
const uint8_t LEFT = 1;
const uint8_t RIGHT = 2;

int ear_mode = EAR_BREATH;
rgb_color ear_color = rgb_color(255, 255, 255);
boolean ear_enabled = true;

boolean eye_enabled = true;
boolean face_mode = false;
int eye_shape = 0; // 13: sleepy
int eye_direction = 0;
int eye_color_idx = 4;
int mouth_shape = 5; //smile
int mouth_direction = 0;
int mouth_color_idx = 0;
int face_shape = 0;

/*
 * EYE methods
 */
void normal_eyes()
{
  matrix.fillCircle(3,3,2,mycolor);
  matrix.fillCircle(12,3,2,mycolor);
}

void blinky_eyes_1()
{
  matrix.drawLine(3,1,5,3,mycolor);
  matrix.drawLine(3,5,5,3,mycolor);
  matrix.drawLine(10,3,12,1,mycolor);
  matrix.drawLine(10,3,12,5,mycolor); 
}

void blinky_eyes_2()
{
  matrix.drawLine(1,3,5,3,mycolor);
  matrix.drawLine(10,3,14,3,mycolor); 
}

void winky_eyes()
{
  matrix.drawLine(3,1,5,3,mycolor);
  matrix.drawLine(3,5,5,3,mycolor);
  matrix.fillCircle(12,3,2,mycolor);
}

void cry_eyes()
{
  matrix.drawLine(1,3,5,3,mycolor);
  matrix.drawLine(3,3,3,6,mycolor);
  matrix.drawLine(10,3,14,3,mycolor);
  matrix.drawLine(12,3,12,6,mycolor); 
}

void draw_eyes()
{
  uint16_t eye_color = colors[eye_color_idx];

  // eye centers
  int rx = 3;
  int lx = 12;
  //robot's r/l
  if(eye_direction==LEFT){
    rx = 4;
    lx = 13;
  }
  else if(eye_direction==RIGHT){
    rx = 2;
    lx = 11;
  }

  int ry = 3;
  int ly = 3;

  switch(eye_shape){
  case NORMAL:
    matrix.fillRect(rx-1,ry-1,3,3,eye_color);
    matrix.drawPixel(rx,ry-2,eye_color);
    matrix.drawPixel(rx,ry+2,eye_color);

    matrix.fillRect(lx-1,ly-1,3,3,eye_color);
    matrix.drawPixel(lx,ly-2,eye_color);
    matrix.drawPixel(lx,ly+2,eye_color);
    break;
  case CLOSE:
    matrix.drawLine(rx-1,ry,rx+1,ry,eye_color);
    matrix.drawLine(lx-1,ly,lx+1,ly,eye_color);
    break;
  case CRY:
    matrix.drawLine(rx-1,ry-2,rx+1,ry-2,eye_color);
    matrix.drawLine(rx,ry-2,rx,ry+2,eye_color);

    matrix.drawLine(lx-1,ly-2,lx+1,ly-2,eye_color);
    matrix.drawLine(lx,ly-2,lx,ly+2,eye_color);    
    break;
  case SQUINT:
    matrix.drawLine(rx-1,ry-2,rx+1,ry,eye_color);
    matrix.drawLine(rx+1,ry,rx-1,ry+2,eye_color);

    matrix.drawLine(lx+1,ly-2,lx-1,ly,eye_color);
    matrix.drawLine(lx-1,ly,lx+1,ly+2,eye_color);   
    break;
  case WINK:
    matrix.drawLine(rx-1,ry-2,rx+1,ry,eye_color);
    matrix.drawLine(rx+1,ry,rx-1,ry+2,eye_color);

    matrix.fillRect(lx-1,ly-1,3,3,eye_color);
    matrix.drawPixel(lx,ly-2,eye_color);
    matrix.drawPixel(lx,ly+2,eye_color);
    break;
  case MAD:
    matrix.drawLine(rx-1,ry-2,rx-1,ry+1,eye_color);
    matrix.drawLine(rx,ry-1,rx,ry+2,eye_color);
    matrix.drawLine(rx+1,ry,rx+1,ry+1,eye_color);

    matrix.drawLine(lx-1,ly,lx-1,ly+1,eye_color);
    matrix.drawLine(lx,ly-1,lx,ly+2,eye_color);
    matrix.drawLine(lx+1,ly-2,lx+1,ly+1,eye_color);
    break;
  case SAD:
    matrix.drawLine(rx-1,ry,rx-1,ry+1,eye_color);
    matrix.drawLine(rx,ry-1,rx,ry+2,eye_color);
    matrix.drawLine(rx+1,ry-2,rx+1,ry+1,eye_color);

    matrix.drawLine(lx-1,ly-2,lx-1,ly+1,eye_color);
    matrix.drawLine(lx,ly-1,lx,ly+2,eye_color);
    matrix.drawLine(lx+1,ly,lx+1,ly+1,eye_color);
    break;
  case SIDELEFT:
    matrix.drawLine(rx-1,ry,rx+1,ry,eye_color);
    matrix.drawPixel(rx+1,ry+1,eye_color);

    matrix.drawLine(lx-1,ly,lx+1,ly,eye_color);
    matrix.drawPixel(lx+1,ry+1,eye_color);
    break;
  case SIDERIGHT:
    matrix.drawLine(rx-1,ry,rx+1,ry,eye_color);
    matrix.drawPixel(rx-1,ry+1,eye_color);

    matrix.drawLine(lx-1,ly,lx+1,ly,eye_color);
    matrix.drawPixel(lx-1,ry+1,eye_color);
    break;
  case HEART:
    matrix.fillRect(rx-1,ry-1,3,3,eye_color);
    matrix.drawPixel(rx,ry+2,eye_color);
    matrix.drawLine(rx-2,ry-1,rx-2,ry,eye_color);
    matrix.drawLine(rx+2,ry-1,rx+2,ry,eye_color);
    matrix.drawPixel(rx-1,ry-2,eye_color);
    matrix.drawPixel(rx+1,ry-2,eye_color);

    matrix.fillRect(lx-1,ly-1,3,3,eye_color);
    matrix.drawPixel(lx,ly+2,eye_color);
    matrix.drawLine(lx-2,ly-1,lx-2,ly,eye_color);
    matrix.drawLine(lx+2,ly-1,lx+2,ly,eye_color);
    matrix.drawPixel(lx-1,ly-2,eye_color);
    matrix.drawPixel(lx+1,ly-2,eye_color);
    break;
  case SLEEPY:
    matrix.drawLine(rx-1,ry-1,rx-1,ry+1,eye_color);
    matrix.drawLine(rx+1,ry-1,rx+1,ry+1,eye_color);
    matrix.drawPixel(rx,ry+1,eye_color);

    matrix.drawLine(lx-1,ly-1,lx-1,ly+1,eye_color);
    matrix.drawLine(lx+1,ly-1,lx+1,ly+1,eye_color);
    matrix.drawPixel(lx,ly+1,eye_color);
    break;
  case HAPPY:
    matrix.drawLine(rx-1,ry-1,rx-1,ry+1,eye_color);
    matrix.drawLine(rx+1,ry-1,rx+1,ry+1,eye_color);
    matrix.drawPixel(rx,ry-1,eye_color);

    matrix.drawLine(lx-1,ly-1,lx-1,ly+1,eye_color);
    matrix.drawLine(lx+1,ly-1,lx+1,ly+1,eye_color);
    matrix.drawPixel(lx,ly-1,eye_color);
    break;
  case SLEEPY_WIDE:
    matrix.drawLine(rx,ry+1,rx+1,ry+1,eye_color);
    matrix.drawPixel(rx-1,ry,eye_color);
    matrix.drawPixel(rx+2,ry,eye_color);

    matrix.drawLine(lx-1,ly+1,lx,ly+1,eye_color);
    matrix.drawPixel(lx-2,ly,eye_color);
    matrix.drawPixel(lx+1,ly,eye_color);
    break;
  case HAPPY_WIDE:
    matrix.drawLine(rx,ry-1,rx+1,ry-1,eye_color);
    matrix.drawPixel(rx-1,ry,eye_color);
    matrix.drawPixel(rx+2,ry,eye_color);

    matrix.drawLine(lx-1,ly-1,lx,ly-1,eye_color);
    matrix.drawPixel(lx-2,ly,eye_color);
    matrix.drawPixel(lx+1,ly,eye_color);
    break;
  case DEAD:
    matrix.drawLine(rx-1,ry-1,rx+1,ry+1,eye_color);
    matrix.drawLine(rx+1,ry-1,rx-1,ry+1,eye_color);

    matrix.drawLine(lx-1,ly-1,lx+1,ly+1,eye_color);
    matrix.drawLine(lx+1,ly-1,lx-1,ly+1,eye_color);   
    break;
  default:
    matrix.fillRect(rx-1,ry-1,3,3,eye_color);
    matrix.drawPixel(rx,ry-2,eye_color);
    matrix.drawPixel(rx,ry+2,eye_color);
    matrix.fillRect(lx-1,ly-1,3,3,eye_color);
    matrix.drawPixel(lx,ly-2,eye_color);
    matrix.drawPixel(lx,ly+2,eye_color);
    break;
  } 
}

void draw_mouth()
{
  uint16_t mouth_color = colors[mouth_color_idx];

  //note: robot's r/l!
  int rx = 6;
  int lx = 9;
  int my = 6;

  if(mouth_direction==LEFT){
    rx = 7;
    lx = 10;
  }
  else if(mouth_direction==RIGHT){
    rx = 5;
    lx = 8;
  }

  switch(mouth_shape){
  case FLAT:
    matrix.drawLine(rx,my,lx,my,mouth_color);
    break;
  case GRIN:
    matrix.drawLine(rx,my-1,lx,my-1,mouth_color);
    matrix.drawLine(rx+1,my,lx-1,my,mouth_color);
    break;
  case GRIMACE:
    matrix.drawLine(rx,my,lx,my,mouth_color);
    matrix.drawLine(rx+1,my-1,lx-1,my-1,mouth_color);
    break;
  case OPEN:
    matrix.drawLine(rx,my,lx,my,mouth_color);
    matrix.drawLine(rx+1,my-1,lx-1,my-1,mouth_color);
    matrix.drawLine(rx+1,my+1,lx-1,my+1,mouth_color);
    break;
  case LONGFACE:
    matrix.drawLine(rx-4,my+1,lx+4,my+1,mouth_color);
    break;
  case SMILE:
    matrix.drawLine(rx+1,my,lx-1,my,mouth_color);
    matrix.drawPixel(rx,my-1,mouth_color);
    matrix.drawPixel(lx,my-1,mouth_color);
    break;
  case FROWN:
    matrix.drawLine(rx+1,my-1,lx-1,my-1,mouth_color);
    matrix.drawPixel(rx,my,mouth_color);
    matrix.drawPixel(lx,my,mouth_color);
    break;
  case BIGOPEN:
    matrix.drawLine(rx+1,my-2,lx-1,my-2,mouth_color);
    matrix.drawLine(rx,my-1,rx,my,mouth_color);
    matrix.drawLine(lx,my-1,lx,my,mouth_color);
    matrix.drawLine(rx+1,my+1,lx-1,my+1,mouth_color);
    break;
  case SQUIGGLE:
    matrix.drawLine(rx+1,my,lx-1,my,mouth_color);
    matrix.drawLine(rx-1,my,rx,my-1,mouth_color);
    matrix.drawLine(lx+1,my,lx,my-1,mouth_color);
    break;
  case WHISTLE:
    matrix.fillRect(rx+1,my,2,2,mouth_color);
    break;
  default:
    matrix.drawLine(6,6,9,6,mouth_color);
    break;
  } 
}

void draw_face()
{
  switch(face_shape){
  case HI:
    break;
  default:
    break;
  }
}


void writeEye(){
  matrix.fillScreen(0);
  matrix.setCursor(2, 1);

  if(!eye_enabled){
    matrix.fillScreen(0);
    matrix.show();
  }

  if(face_mode){
    draw_face();
  }
  else{
    draw_eyes();
    draw_mouth();
  }

  if(eye_enabled){
    matrix.show();
  }
}  


/*
 * blink_action blinks normally in a deterministic fashion. 
 * Blinking is done in 'rounds' or a percentage of calls to this function due to the 
 * unparallelized nature of the arduino. 
 * Using delays pauses other LED actions on the ears, such as pulsing or gradients. 
 * Thus using this scheme of cycles is necessary for animations.
 * 
 * You can change blinking rates by the following parameters:
 *    round_cutoff = the number of cycles that the eyes will be 'normal' or 'open'
 *    blink_difference = the number of cycles the eyes will be 'blinking' or 'closed'
 */
int round_cutoff = 60;
int blink_difference = 5;
int blink_round = 0;
void blink_action()
{
  matrix.fillScreen(0);
  matrix.setCursor(0, 1);
  if(blink_round < round_cutoff){
    normal_eyes();
  }
  else
  {
    blinky_eyes_2();
  }
  blink_round += 1;
  if(blink_round > round_cutoff + blink_difference)
    blink_round = 0;
}

ros::ServiceServer<poli_msgs::LedEar::Request, poli_msgs::LedEar::Response> ear_server("led_ear",&ear_srv_callback);
ros::ServiceServer<poli_msgs::LedEye::Request, poli_msgs::LedEye::Response> eye_server("led_eye",&eye_srv_callback);


void ear_srv_callback(const poli_msgs::LedEar::Request & req, poli_msgs::LedEar::Response & res){
  if(req.command == req.DISABLE){
    ear_enabled = false;
  }
  else if(req.command == req.ENABLE){
    ear_enabled = true;
  }
  else if(req.command == req.SOLID){
    switch(req.color){
    case 0:
      ear_color = rgb_color(255, 0, 0); 
      break;
    case 1:
      ear_color = rgb_color(0, 0, 255); 
      break;
    case 2:
      ear_color = rgb_color(0, 255, 0); 
      break;
    case 3:
      ear_color = rgb_color(255, 255, 0); 
      break;
    case 4:
      ear_color = rgb_color(255, 255, 255); 
      break;
    default:
      ear_color = rgb_color(255, 255, 255); 
      break;
    }
    ear_mode = EAR_SOLID;

  } 
  else if(req.command == req.GRADIENT){
    ear_mode = EAR_GRADIENT;
  }
  else if(req.command == req.BREATH){
    ear_mode = EAR_BREATH; 
  }
  res.response = req.SUCCESS;

}



void eye_srv_callback(const poli_msgs::LedEye::Request & req, poli_msgs::LedEye::Response & res){

  if(req.command == req.DISABLE){
    eye_enabled = false;
  }
  else if(req.command == req.ENABLE){
    eye_enabled = true;
  }
  else{
    if(req.which_part == req.FULL_FACE){
      face_mode = true;
      face_shape = req.face_shape;
    }
    if(req.which_part == req.EYES || req.which_part == req.BOTH){
      face_mode = false;
      if(req.which_feature == req.DIRECTION || req.which_feature == req.BOTH){
        eye_direction = req.eye_direction;
      } 
      if(req.which_feature == req.SHAPE || req.which_feature == req.BOTH){
        eye_shape = req.eye_shape;
      }
      if(req.which_feature == req.COLOR){
        eye_color_idx = req.eye_color;
      }
    }
    if(req.which_part == req.MOUTH || req.which_part == req.BOTH){
      face_mode = false;
      if(req.which_feature == req.DIRECTION || req.which_feature == req.BOTH){
        mouth_direction = req.mouth_direction;
      } 
      if(req.which_feature == req.SHAPE || req.which_feature == req.BOTH){
        mouth_shape = req.mouth_shape;
      }
      if(req.which_feature == req.COLOR){
        mouth_color_idx = req.mouth_color;
      }
    }
  }
  res.response = 1; //req.SUCCESS;
}


void initialize_ear_colors(){
  for(uint16_t i = 0; i < ledCount; i++)
  {
    ear_colors[i] = rgb_color(0, 128, 0);
  }
}

//TODO once the both ears are established, need to make the limits (i=50) more concrete
// and document it
uint16_t additor = 1;
uint16_t breath_color = 16;
bool darken = false;
void writeEar(){
  uint8_t time = millis() >> 2;
  if(ear_mode == EAR_SOLID){
    brightness = 8;
    for(uint16_t i = 0; i < ledCount; i++)
    {
      ear_colors[i] = ear_color;
    }
  }
  else if(ear_mode == EAR_GRADIENT){ 
    brightness = 8;
    for(uint16_t i = 0; i < ledCount; i++)
    {
      uint8_t x = time - i;
      ear_colors[i] = rgb_color(255 - x, 255 - x, 255 - x);
    }
  }
  else if(ear_mode == EAR_BREATH){
    brightness = 8;
    
    if(darken){breath_color--;}else{breath_color++;}
    if(breath_color==126){darken=true;}
    if(breath_color==16){darken=false;}
    for(uint16_t i = 0; i < ledCount; i++)
    {
      ear_colors[i] = rgb_color(breath_color*2, breath_color, 0);
    }
  }
  if(ear_enabled)
    ledStrip.write(ear_colors, ledCount, brightness);
  else
    ledStrip.write(ear_colors, ledCount, 0);

   
}




int x    = 2;
int pass = 0;
int seed = 0;
//This method randomly blinks in different ways
void prashants_eye_loop(){
  seed = random(1,4);

  for (int i = 0;i<40;i++)
  { 
    matrix.fillScreen(0);
    matrix.setCursor(x, 1);
    if(i<33)
    {
      normal_eyes();
    }
    else
    {
      switch(seed)
      {
      case 1:
        blinky_eyes_1();
        break;
      case 2:
        blinky_eyes_2();
        break;
      case 3:
        winky_eyes();
        break;
      case 4:
        cry_eyes();

      }

    }
    matrix.show();
    delay(100);

  } 
}

std_msgs::Int16 limit_switch_msg;
ros::Publisher limit_pub("pillar/limit_switch", &limit_switch_msg);
void checkLimitSwitch(){
  if (digitalRead(limitSwitchPin) == HIGH) {
    limit_switch_msg.data = 1;
  }
  else{
    limit_switch_msg.data = 0; 
  }
  limit_pub.publish(&limit_switch_msg);
}





void setup()
{
  pinMode(limitSwitchPin, INPUT_PULLUP);
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(10);
  initialize_ear_colors();

  nh.initNode();


  nh.advertiseService(ear_server);

  nh.advertiseService(eye_server);
  nh.advertise(limit_pub);
}

void loop()
{ 
  writeEye();
  writeEar();
  checkLimitSwitch();

  nh.spinOnce();
  delay(10);
}
