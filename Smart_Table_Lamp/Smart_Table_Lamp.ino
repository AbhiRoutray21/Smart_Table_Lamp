
#include <esp_now.h>
#include <WiFi.h>
#include <LBDL_Slider.h>
#include <FastLED.h>

#define NUM_LEDS 8
#define LED_PIN 19
uint8_t hue = 0;

CRGB leds[NUM_LEDS];
uint8_t paletteIndex = 0;
uint8_t paletteIndex1 = 0;
uint8_t colorIndex[NUM_LEDS];

Slider slider;
int slide_counter = 0;
int count = 0;
int long_counter = 0;

bool action = true;
bool slider_action = true;
bool single_action = true;
bool Long_action = true;
bool double_action = true;
bool Led_indication = LOW;
bool control_action = false;


int buttonPin;
unsigned long inStateAtMs = millis();
int buttonState;  // 0 = waiting for press, 
//1= waiting for release or if touch is not release within 500 milli sec then long press,
//2= differentiate single or double, 
//3= wait touch release (double),
//4= wait touch release (long)

uint8_t broadcastAddress[] = {0xC8,0xC9,0xA3,0x09,0xD0,0xE9}; // REPLACE WITH YOUR RECEIVER MAC Address
struct struct_message  // Must match the receiver structure
{
  int a;
};

struct_message myData; // Create a struct_message called myData

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);
  slider.setup(T4,T5,T6);
  pinMode(buttonPin, INPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  for(int i = 0; i < NUM_LEDS; i++){
    colorIndex[i] = random8();
  }
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(5, OUTPUT);

}
 
void loop() {

  if (control_action == false){
  WiFi.mode(WIFI_OFF);
  }
  else if(control_action == true){
    WiFi.mode(WIFI_STA);
    esp_now_init();
     memcpy(peerInfo.peer_addr, broadcastAddress, 6);
     peerInfo.channel = 0;  
     peerInfo.encrypt = false;
     esp_now_add_peer(&peerInfo);
     esp_now_register_send_cb(OnDataSent);
  }

  buttonPin = touchRead(T9);
  if (buttonState == 0) {
    // wait for button press
    if (millis() - inStateAtMs > 100) 
    {
      if (buttonPin < 27) 
      {
        buttonState = 1;
        inStateAtMs = millis();
      }
    }
  } 
  else if (buttonState == 1) 
  {
    // wait for button release 1
    if (millis() - inStateAtMs > 50 && buttonPin > 27) 
    {
      buttonState = 2;
      inStateAtMs = millis();
    }
    if (Long_action == true)
    {
     if (millis() - inStateAtMs > 600 && buttonPin < 27) 
     {
      // got long press within timeout - long press
      Serial.println("LongPress"); 
      long_counter > 2 ? long_counter = 0 : long_counter++;
      Serial.println(long_counter); 
      count = 0;
      slide_counter = 0;
      buttonState = 4;
      inStateAtMs = millis();
     }
    }    
  } 
  else if (buttonState == 2) 
  {
    // differentiate between single and double touch
    if (millis() - inStateAtMs > 300) 
    {
      // timeout - is a single tap
      Serial.println("single");
      if(single_action == true)
      {
       if(action == true)
       {
       count >= 4 ? count = 4 : count++;  // switch counter
       Serial.println(count);
       slide_counter = 0;
       }
       else if(action == false)
       {
        digitalWrite(5, LOW);
        delay(100);
        digitalWrite(5, HIGH);
        myData.a = 1;
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
       }
      }

      buttonState = 0;
      inStateAtMs = millis();
    } 
    else if ((buttonPin < 27) && (millis() - inStateAtMs > 100)) 
    {
      // got second touch within timeout - double tap
      Serial.println("\ndouble");
      if(double_action == true)
      {
      action = !action;
      slider_action = !slider_action;
      Led_indication = !Led_indication;
      Long_action = !Long_action;
      control_action = !control_action;
      single_action = true;
   
      digitalWrite(5, Led_indication);

      slide_counter = 0;
      count = 0;
      }
      buttonState = 3;
      inStateAtMs = millis();
    }
  } 
  else if (buttonState == 3) 
  {
    // wait for stable button release 2
    if (millis() - inStateAtMs > 100 && buttonPin > 27) 
    {
      buttonState = 0;
      inStateAtMs = millis();
    }
  }
  else if (buttonState == 4) 
  {
    // wait for stable button release 1
    if (millis() - inStateAtMs > 100 && buttonPin > 27) 
    {
      buttonState = 0;
      inStateAtMs = millis();
    }
  }

 // this switch case turn ON/OFF two lights white and warm white
    switch (count) 
     {
      case 1:
         digitalWrite(17, HIGH);
        break;
      case 2:
        digitalWrite(17, LOW);
         analogWrite(18,235);
        break;  
      case 3:
        digitalWrite(17, HIGH);
         analogWrite(18,235);
        break;  
      default:
        analogWrite(18,0);
        digitalWrite(17, LOW);
        count = 0;
        break;
     }
 // this switch case turn ON/OFF two lights white and warm white 

 // this will detect sliding action
 if (slider_action == true)
 {
  slider.detectDirection();
  
  if(slider.getDirection() == "LEFT"){
   	slide_counter >= 10 ? slide_counter = 0 : slide_counter++;	
    Serial.println(slide_counter);
    count = 0;
    slider.clearDirection();
  }

  if(slider.getDirection() == "RIGHT"){
    slide_counter <= 0 ? slide_counter = 0 : slide_counter--;
    Serial.println(slide_counter);
    slider.clearDirection();	
  }
 }
 switch (slide_counter) 
     {
      case 1:    
        fill_solid(leds, NUM_LEDS, CHSV(0,199,255)); // cherry
        FastLED.show();
        break;
      case 2:
        fill_solid(leds, NUM_LEDS, CHSV(58,240,255)); // yellow
        FastLED.show();
        break;
      case 3:
        fill_solid(leds, NUM_LEDS, CHSV(96,190,255)); // green
        FastLED.show();
        break;  
      case 4:
        fill_solid(leds, NUM_LEDS, CHSV(15,255,255)); // orange
        FastLED.show();
        break;
      case 5:
        fill_solid(leds, NUM_LEDS, CHSV(196,255,255)); //purple
        FastLED.show();
        break;   
      case 6:
        fill_solid(leds, NUM_LEDS, CHSV(128,255,255)); // cyan
        FastLED.show();
        break;
      case 7:
        fill_solid(leds, NUM_LEDS, CHSV(224,220,255)); // pink
        FastLED.show();
        break;  
      case 8:
        fill_solid(leds, NUM_LEDS, CHSV(140,255,255)); // light blue
        FastLED.show();
        break;
      case 9:
        fill_rainbow(leds, NUM_LEDS, 0, 255/ NUM_LEDS); // Rainbow
        FastLED.show();
        break;            
      default:
        fill_solid(leds, NUM_LEDS, CHSV(0,0,0));
        FastLED.show();
        slide_counter = 0;
        break;
     }
  
  if(long_counter > 0)  // long action light control 
  { 
    slider_action = false;
    single_action = false;
    double_action = false;
    if(long_counter == 1)
    {
      for(int i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = CHSV(hue + (i * 10), 255, 255);
        }
        EVERY_N_MILLISECONDS(15){
           hue++;
        }
        FastLED.show();
    }
    else if(long_counter == 2)
    {
     slider_action = true;
     single_action = true;
     double_action = true;
     long_counter = 0;
    }
  }

}



