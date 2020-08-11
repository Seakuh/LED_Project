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
boolean choice;
char eingabe;


int RECV_PIN = 11;          // IR PIN.
IRrecv irrecv(RECV_PIN);    // Pin auslesen
decode_results results;
CRGB leds[NUM_LEDS];



void setup() {
  delay(3000); // 3 second delay for recovery

  // Setup LED --------------------------------------------------------------------------

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(TypicalLEDStrip)
  .setDither(BRIGHTNESS < 255);

  FastLED.setBrightness(BRIGHTNESS);

  // IR --------------------------------------------------------------------------------

  Serial.begin(9600);    // Serielle Verbindung startet + monitoring sehen
  pinMode (13, OUTPUT);
  irrecv.enableIRIn();   // initialisiere den Infrarotempfänger.
  decode_results results;

}

// Des haett ma au besser machn kenna, ging mit den if-Abfragen aber ab besten

void loop()
{
  checkIRInput();
  if (results.value == 16724175) {
    eingabe = '1';
  }
  if (results.value == 16718055) {
    eingabe = '2';
  }
  if (results.value == 16743045) {
    eingabe = '3';
  }
  if (results.value == 16716015) {
    eingabe = '4';
  }
  if (results.value == 16726215) {
    eingabe = '5';
  }
  if (results.value == 16734885) {
    eingabe = '6';
  }
  if (results.value == 16728765) {
    eingabe = '7';
  }
  if (results.value == 16730805) {
    eingabe = '8';
  }
  if (results.value == 16732845) {
    eingabe = '9';
  }

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

  //Regenbogen
  if (eingabe == '1') {
    rainbow((paletteBeatsPerMinute / 60));
  }

  // HSV
  if (eingabe == '2') {
    setHSV();
    checkIRInput();

  }

  // DROP
  if (eingabe == '3') {
    PeriodicallyChooseNewColorPalettes(); // currently does nothing
    MixBeatPalette();                     // mix up the new 'beat palette' to draw with
    DrawOneFrameUsingBeatPalette();       // draw a simple animation using the 'beat palette'
    FastLED.show();
    FastLED.delay(30);
    checkIRInput();
    checkIRInput();

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



// HSV -----------------------------------------------------------------------------

void setHSV() {
  leds[0].setHue( 160);
}


// Rainbow -----------------------------------------------------------------------------

void rainbow(uint8_t wait) {
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(i - (j * 2), BRIGHTNESS, SATURATION); /* The higher the value 4 the less fade there is and vice versa */
    }
    checkBPM();
    FastLED.show();
    delay(wait); /* Change this to your hearts desire, the lower the value the faster your colors move (and vice versa) */
  }
}


// BPM -------------------------------------------------------------------------------

CRGBPalette16 gCurrentPaletteA( CRGB::Black);
CRGBPalette16 gCurrentPaletteB( CRGB::Black);

CRGBPalette16 gTargetPaletteA( RainbowColors_p);
CRGBPalette16 gTargetPaletteB( CRGB::Blue );

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
  uint8_t mixer = cubicwave8( beat);

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
