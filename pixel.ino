#include <FastLED.h>

FASTLED_USING_NAMESPACE

#define LED_PIN  3
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND  120

// Params for width and height
const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 8;

// Params for Button with led
const int buttonPin1 = 8;    // the pin that the pushbutton is attached to
const int buttonPin2 = 9;    // the pin that the pushbutton is attached to
const int buttonLedPin1 = 10;       // the pin that the LED is attached to
const int buttonLedPin2 = 11;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState1 = 0;         // current state of the button
int buttonState2 = 0;         // current state of the button
int lastButtonState1 = 0;     // previous state of the button
int lastButtonState2 = 0;     // previous state of the button
uint16_t ledIndex = 0;

// Param for different pixel layouts
const bool kMatrixSerpentineLayout = false;

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[NUM_LEDS];
CRGB ledsTablero[NUM_LEDS];
//CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];
//CRGB* const leds(leds_plus_safety_pixel + 1);
//CRGB* const ledsTablero(leds_plus_safety_pixel + 1);

// DEMO

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
// END DEMO

// Variables de estado
bool state_standby = false;
bool state_getReady = false;
bool state_play = false;
bool state_capture = false;
bool state_winner = false;
bool state_reset = false;

bool playing = false;
bool demoing = false;
bool winnerAnimation = false;
bool getReadyAnimation = false;
bool winnerLedOn = false;

uint8_t pixelActual = 0;
byte winner = 0;

void setup() {
  randomSeed(analogRead(0));
  
  // initialize the button pin as a input:
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  
  // initialize the LED as an output:
  pinMode(buttonLedPin1, OUTPUT);
  pinMode(buttonLedPin2, OUTPUT);
  
  // initialize serial communication:
  Serial.begin(9600);

  // Configuracion tira leds  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness( BRIGHTNESS );

  state_reset = true;
}

// DEMO variables
// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
// END DEMO variables

// Game loop
// @author Egar Almeida
void loop(){
  
  if(state_standby) {
    resetFlags();
    demoing = true;
  } 
  else if(state_getReady) {
    resetFlags();

    pixelActual = 500;
    byte testPixel = random(0,63); //11
    while(pixelActual != testPixel) {
      if(ledsTablero[testPixel] == CRGB(0, 0, 0)) {
          pixelActual = testPixel;
      }
      else {
        testPixel = random(0,63);
      }
    }
    
    getReadyAnimation = true;
  } 
  else if(state_play) {
    resetFlags();
    playing = true;
    
    ledIndex = 2;
    winner = 0;
  }
  else if(state_capture) {
    resetFlags();

    if(winner == 1) {
      ledsTablero[pixelActual] = CRGB::Blue;
    } 
    else if(winner == 2) {
      ledsTablero[pixelActual] = CRGB::Green;
    }
    else {
      ledsTablero[pixelActual] = CRGB::Black;
    }

    // Copiar estado del tablero en array que se muestra
    for (int j = 0; j <= 63; j++) {
      leds[j] = ledsTablero[j];
    }
    FastLED.show();
    
    state_winner = true;
  }
  else if(state_winner) {
    resetFlags();
    
    winnerAnimation = true;
  }
  else if(state_reset) {
    resetFlags();
    
    for (int i = 0; i <= 63; i++) {
      ledsTablero[i] = CRGB::Black;
    }

    state_standby = true;
  }

  if (demoing) {
    doDemo();
    if (digitalRead(buttonPin1) == HIGH || digitalRead(buttonPin2) == HIGH) {
      demoing = false;
      state_getReady = true;
      // Borra el display
      for (int i = 0; i <= 63; i++) {
        leds[i] = CRGB::Black;
      }
    }
  } 
  else if (getReadyAnimation) {
    for(int i = 0; i < pixelActual; i++) {
      //leds[i] = CRGB(random(0, 255), random(0, 255), random(0, 255));
      leds[i] = CRGB(255, 0, 0);
      FastLED.show();
      leds[i] = ledsTablero[i];
      delay(25);
    }
    state_play = true;
  }
  else if (winnerAnimation) {
    for(int i = 0; i < 10; i++) {
      if(winner == 1){
        if(winnerLedOn) {
          digitalWrite(buttonLedPin1, LOW);
          winnerLedOn = false;
        }
        else{
          digitalWrite(buttonLedPin1, HIGH);
          winnerLedOn = true;
        }
      }
      else if(winner == 2) {
        if(winnerLedOn) {
          digitalWrite(buttonLedPin2, LOW);
          winnerLedOn = false;
        }
        else{
          digitalWrite(buttonLedPin2, HIGH);
          winnerLedOn = true;
        }
      }
      else {
        break;
      }
      delay(200);
    }
    state_getReady = true;
  }
  else if (playing) {
    leds[pixelActual] = CRGB(255, 0, 0);
    
    // Establece el color de los 3 leds que son el tren
    leds[ledIndex] = CRGB(random(0, 255), random(0, 255), random(0, 255));
    leds[ledIndex - 1] = leds[ledIndex];
    leds[ledIndex - 2] = leds[ledIndex];
    FastLED.show();
    
    leds[ledIndex] = ledsTablero[ledIndex];
    leds[ledIndex - 1] = ledsTablero[ledIndex - 1];
    leds[ledIndex - 2] = ledsTablero[ledIndex - 2];
    
    buttonState1 = digitalRead(buttonPin1);
    buttonState2 = digitalRead(buttonPin2);

    if(buttonState1 == HIGH) {
      if(ledIndex == pixelActual || ledIndex - 1 == pixelActual || ledIndex - 2 == pixelActual) {
        winner = 1;
      } else {
        winner = 2;
      }
      state_capture = true;
    } 
    else if (buttonState2 == HIGH) {
      if(ledIndex == pixelActual || ledIndex - 1 == pixelActual || ledIndex - 2 == pixelActual) {
        winner = 2;
      } else {
        winner = 1;
      }
      state_capture = true;
    }

    // Mover el tren
    if (ledIndex < 63) {
      ledIndex++; 
    }
    else {
      winner = 0;
      state_capture = true;
    }

    delay(100);
  }
}

void resetFlags() {
  demoing = false;
  getReadyAnimation = false;
  playing = false;
  winnerAnimation = false;
  
  state_standby = false;
  state_getReady = false;
  state_play = false;
  state_capture = false;
  state_winner = false;
  state_reset = false;
}

void doDemo() {
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

// Convertir de coordenadas XY a indice de array
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}

// Funciones del DEMO

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    if (demoing == false) {
      break;
    }
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    if (demoing == false) {
      break;
    }
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}



