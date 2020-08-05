#include "FastLED.h"
#include <IRremote.h>


#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    6
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    300
#define BRIGHTNESS  255
#define SATURATION 255

uint8_t paletteBeatsPerMinute = 120;

int RECV_PIN = 11;          // IR PIN.
IRrecv irrecv(RECV_PIN);    // Pin auslesen
decode_results results;
CRGB leds[NUM_LEDS];



void setup() {
  delay(3000); // 3 second delay for recovery

  //  // Setup LED --------------------------------------------------------------------------

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(TypicalLEDStrip)
  .setDither(BRIGHTNESS < 255);

  FastLED.setBrightness(BRIGHTNESS);

  FastLED.setBrightness(BRIGHTNESS);

  // IR --------------------------------------------------------------------------------

  Serial.begin(9600);    // Serielle Verbindung startet + monitoring sehen
  pinMode (13, OUTPUT);
  irrecv.enableIRIn();   // initialisiere den Infrarotempfänger.
  decode_results results;

}

void loop()
{
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();

  //checkIRInput();
  //sobpmMix();

  delay(paletteBeatsPerMinute);
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(paletteBeatsPerMinute);
  fill_solid(leds, NUM_LEDS, CRGB::Magenta);
  FastLED.show();
  delay(paletteBeatsPerMinute);
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(paletteBeatsPerMinute);
  fill_rainbow(leds, NUM_LEDS, 0, 255 / NUM_LEDS);
  FastLED.show();
  delay(paletteBeatsPerMinute);

  CylonBounce(0xff, 0, 0, 30, 10, 50);



}

void bpmMix() {
  int i = 0;
  while (i < 100) {
    PeriodicallyChooseNewColorPalettes(); // currently does nothing
    MixBeatPalette();                     // mix up the new 'beat palette' to draw with
    DrawOneFrameUsingBeatPalette();       // draw a simple animation using the 'beat palette'
    FastLED.show();
    FastLED.delay(20);
    i++;
    Serial.println(i);
  }
}


// IR --------------------------------------------------------------------------------

void checkIRInput() {
  if (irrecv.decode(&results)) {          // wenn Daten empfangen wurden,
    Serial.print("DEC: ");   // DEC ausgabe
    Serial.print(results.value, DEC);
    Serial.print(" BIN: ");   // DEC ausgabe
    Serial.println(results.value, BIN);
    Serial.print(paletteBeatsPerMinute);
    Serial.println(": BPM");
    // +
    if (results.value == 16754775) {
      paletteBeatsPerMinute++;
      results.value = 0;
    }
    // -
    if (results.value == 16769055) {
      paletteBeatsPerMinute--;
      results.value = 0;
    }
    irrecv.resume();
  }
}

// Check BPM -------------------------------------------------------------------------

void checkBPM() {
  checkIRInput();
  // +
  if (results.value == 16754775) {
    paletteBeatsPerMinute++;
    results.value = 0;
  }
  // -
  if (results.value == 16769055) {
    paletteBeatsPerMinute--;
    results.value = 0;
  }
}


// Serial Output ---------------------------------------------------------------------

void serialOutPut(uint8_t a) {
  Serial.print(a);
  Serial.print(": wurde gedrueckt ");
  Serial.print(paletteBeatsPerMinute);
  Serial.println(": BPM");
}

// LichtBall

void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {

  for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
    setAll(0, 0, 0);
    setPixel(i, red / 10, green / 10, blue / 10);
    for (int j = 1; j <= EyeSize; j++) {
      setPixel(i + j, red, green, blue);
    }
    setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
    showStrip();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);
}


// BPM -------------------------------------------------------------------------------

CRGBPalette16 gCurrentPaletteA( CRGB::Black);
CRGBPalette16 gCurrentPaletteB( CRGB::Black);

CRGBPalette16 gTargetPaletteA( RainbowColors_p);
CRGBPalette16 gTargetPaletteB( CRGB::Red );

CRGBPalette16 gBeatPalette;

// Here's where the magic happens.  Since internally palettes are
// nothing more than an array of CRGB, we can use the FastLED "blend"
// function to mix up a NEW palette, which is a blend of our two current
// palettes, with the amount of blending controlled by a 'mixer' that
// pulses back and forth smoothly between the two, at a given BPM.
//
void MixBeatPalette()
{

  uint8_t beat = beat8(paletteBeatsPerMinute); // repeats from 0..255

  // 'cubicwave8' spends more time at each end than sin8, and less time
  // in the middle.  Try others: triwave8, quadwave8, sin8, cubicwave8
  uint8_t mixer = cubicwave8(beat);

  // Mix a new palette, gBeatPalette, with a varying amount of contribution
  // from gCurrentPaletteA and gCurrentPaletteB, depending on 'mixer'.
  // The 'beat palette' is then used to draw onto the LEDs.
  uint8_t palettesize = sizeof( gBeatPalette) / sizeof(gBeatPalette[0]); // = 16
  blend( gCurrentPaletteA, gCurrentPaletteB, gBeatPalette, palettesize, mixer);
}


// Sample draw function to draw some pixels using the colors in gBeatPalette
void DrawOneFrameUsingBeatPalette()
{
  uint8_t startindex = millis() / 2;
  uint8_t incindex = 7;
  fill_palette(leds, NUM_LEDS, startindex, incindex, gBeatPalette, 255, LINEARBLEND);
}


// If you wanted to change the two palettes from 'rainbow' and 'red'
// to something else, here's where you'd do it.
void PeriodicallyChooseNewColorPalettes()
{

  EVERY_N_MILLISECONDS(20) {
    nblendPaletteTowardPalette( gCurrentPaletteA, gTargetPaletteA);
    nblendPaletteTowardPalette( gCurrentPaletteB, gTargetPaletteB);
  }
}
