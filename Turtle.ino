//

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define INTERRUPT_PIN 2
#define LED_PIN 4
int ANALOG_PIN = A0;
#define MODE_COUNT 4

// How many NeoPixels are attached to the Arduino?
#define NUM_PIXELS 129

#define BRIGHTNESS_MIN 10
#define BRIGHTNESS_MAX 255

// Modes:
//  0: Rainbow cycle
//  1: theaterChaseRainbow
//  2: pulseWhite
int lightingMode;

// Modes:
bool modeChangeLighting;
int oldLightingMode;
int brightness;
int oldBrightness;
bool interrupted;
bool modeLightingChanged;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel pixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// setup() function -- runs once at startup --------------------------------
void setup()
{
    bool mode_read_bug;

// These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
#endif

    modeChangeLighting = true;
    modeLightingChanged = true;
    interrupted = false;
    mode_read_bug = false;

    lightingMode = eepromReadInt(0); // read lightingMode from EEPROM
    if ((lightingMode < 0) || (lightingMode > MODE_COUNT))
    {
        lightingMode = 0;
        mode_read_bug = true;
    }

    Serial.begin(19200);
    Serial.println("--------------------------------------------");
    Serial.println("--------------------------------------------");
    Serial.println("Welcome to the Turtle Debugging");
    Serial.println("--------------------------------------------");
    if (mode_read_bug)
    {
        Serial.println("Could not read lighting mode. Setting 0.");
    }
    else
    {
        Serial.println("Read lighting mode: " + String(lightingMode));
    }

    // enable pin as input with internal pull up
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), ISR0, FALLING); // activate ISR

    brightness = readAnalog();

    // init NeoPixel object (REQUIRED)
    pixel.begin();
    constantColor(0x00FF00);
    pixel.setBrightness(brightness); // Set BRIGHTNESS to about 1/5 (max = 255)
    pixel.show();                    // Turn OFF all pixels ASAP
}

void loop()
{
    if (lightingMode != oldLightingMode)
    {
        Serial.println("Light mode: " + String(lightingMode));
    }

    brightness = readAnalog();
    Serial.println("Got brightness of " + String(brightness));

    // Constant

    if (lightingMode == 0)
    {
        if ((lightingMode != oldLightingMode) || (oldBrightness != brightness))
        {
            modeLightingChanged = false;
            Serial.println("Executing white");
            Serial.println("--------------------------------------------");
            constantColor(0xFFFFFF);
            oldLightingMode = lightingMode;
            oldBrightness = brightness;
        }
    }

    if (lightingMode == 1)
    {
        if ((lightingMode != oldLightingMode) || (oldBrightness != brightness))
        {
            modeLightingChanged = false;
            Serial.println("Executing warm white");
            Serial.println("--------------------------------------------");
            constantMixedColor(0xFFFFFF, 0xDE7214, 3);
            oldLightingMode = lightingMode;
            oldBrightness = brightness;
        }
    }

    if (lightingMode == 2)
    {
        if ((lightingMode != oldLightingMode) || (oldBrightness != brightness))
        {
            modeLightingChanged = false;
            Serial.println("Executing turquoise");
            Serial.println("--------------------------------------------");
            constantColor(0x00FFFF);
            oldLightingMode = lightingMode;
            oldBrightness = brightness;
        }
    }

    if (lightingMode == 3)
    {
        if ((lightingMode != oldLightingMode) || (oldBrightness != brightness))
        {
            modeLightingChanged = false;
            Serial.println("Executing green");
            Serial.println("--------------------------------------------");
            constantColor(0x00FF00);
            oldLightingMode = lightingMode;
            oldBrightness = brightness;
        }
    }

    // Animated

    if (lightingMode == 4)
    {
        Serial.println("Executing rainbow");
        Serial.println("--------------------------------------------");
        oldLightingMode = lightingMode;
        oldBrightness = brightness;
        rainbow(50);
    }

    if (lightingMode == 5)
    {
        Serial.println("Executing full rainbow");
        Serial.println("--------------------------------------------");
        oldLightingMode = lightingMode;
        oldBrightness = brightness;
        fullRainbow(100);
    }
}

void ISR0()
{
    int counter = 0;
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    Serial.println("Triggered interrupt. Waiting for debounce.");
    // while (digitalRead(INTERRUPT_PIN) == 0)
    // {
    //     delay(100);
    //     counter = counter + 1;
    // }
    if (interrupt_time - last_interrupt_time > 200)
    {

        Serial.println("Starting ISR0");
        Serial.println("Pressed button for " + String(counter) + " ms.");

        Serial.println("Change lighting.");
        if (lightingMode >= MODE_COUNT - 1)
        {
            lightingMode = 0;
        }
        else
        {
            lightingMode = lightingMode + 1;
        }
        Serial.println("Changed mode to " + String(lightingMode));
        eepromWriteInt(0, lightingMode); // save lightingMode in EEPROM
        Serial.println("ISR finished");
    }
    last_interrupt_time = interrupt_time;
}

void eepromWriteInt(int adr, int wert)
{
    byte low, high;
    low = wert & 0xFF;
    high = (wert >> 8) & 0xFF;
    EEPROM.write(adr, low); // takes 3,3ms
    EEPROM.write(adr + 1, high);
    return;
} // eepromWriteInt

int eepromReadInt(int adr)
{
    byte low, high;
    low = EEPROM.read(adr);
    high = EEPROM.read(adr + 1);
    return low + ((high << 8) & 0xFF00);
} // eepromReadInt

int readAnalog()
{
    int val = 1023 - analogRead(ANALOG_PIN);
    Serial.println("Analgo value read: " + String(val));
    return map(val, 0, 1023, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
}

// Some functions of our own for creating animated effects -----------------

void constantColor(uint32_t color)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        pixel.setPixelColor(i, color);
    }
    pixel.setBrightness(brightness);
    pixel.show();
    Serial.println("Colour written");

    return;
}

void constantMixedColor(uint32_t color1, uint32_t color2, uint32_t num2)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if ((i % num2) == 0)
        {
            pixel.setPixelColor(i, color2);
        }
        else
        {
            pixel.setPixelColor(i, color1);
        }
    }
    pixel.setBrightness(brightness);
    pixel.show();
    Serial.println("Colours written");

    return;
}

// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// pixel.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {                                  // For each pixel in strip...
        pixel.setPixelColor(i, color); //  Set pixel's color (in RAM)
        pixel.show();                  //  Update strip to match
        delay(wait);                   //  Pause for a moment
    }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la pixel.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait)
{
    for (int a = 0; a < 10; a++)
    { // Repeat 10 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            pixel.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in steps of 3...
            for (int c = b; c < NUM_PIXELS; c += 3)
            {
                pixel.setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            pixel.show(); // Update strip with new contents
            delay(wait);  // Pause for a moment
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait)
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < NUM_PIXELS; i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (NUM_PIXELS steps):
            int pixelHue = firstPixelHue + (i * 65536L / NUM_PIXELS);
            // pixel.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through pixel.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            pixel.setPixelColor(i, pixel.gamma32(pixel.ColorHSV(pixelHue)));
        }
        pixel.show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
        if (interrupted)
        {
            Serial.println("BREAK rainbow");
            interrupted = false;
            break;
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void fullRainbow(int wait)
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < NUM_PIXELS; i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (NUM_PIXELS steps):
            int pixelHue = firstPixelHue; // + (i * 65536L / NUM_PIXELS);
            // pixel.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through pixel.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            pixel.setPixelColor(i, pixel.gamma32(pixel.ColorHSV(pixelHue)));
        }
        pixel.show(); // Update strip with new contents
        delay(wait);  // Pause for a moment
        if (interrupted)
        {
            Serial.println("BREAK rainbow");
            interrupted = false;
            break;
        }
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait)
{
    int firstPixelHue = 0; // First pixel starts at red (hue 0)
    for (int a = 0; a < 30; a++)
    { // Repeat 30 times...
        for (int b = 0; b < 3; b++)
        {                  //  'b' counts from 0 to 2...
            pixel.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in increments of 3...
            for (int c = b; c < NUM_PIXELS; c += 3)
            {
                // hue of pixel 'c' is offset by an amount to make one full
                // revolution of the color wheel (range 65536) along the length
                // of the strip (NUM_PIXELS steps):
                int hue = firstPixelHue + c * 65536L / NUM_PIXELS;
                uint32_t color = pixel.gamma32(pixel.ColorHSV(hue)); // hue -> RGB
                pixel.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
            }
            pixel.show();                // Update strip with new contents
            delay(wait);                 // Pause for a moment
            firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
        }
    }
}

void whiteOverRainbow(int whiteSpeed, int whiteLength)
{

    if (whiteLength >= NUM_PIXELS)
        whiteLength = NUM_PIXELS - 1;

    int head = whiteLength - 1;
    int tail = 0;
    int loops = 3;
    int loopNum = 0;
    uint32_t lastTime = millis();
    uint32_t firstPixelHue = 0;

    for (;;)
    { // Repeat forever (or until a 'break' or 'return')
        for (int i = 0; i < NUM_PIXELS; i++)
        {                                       // For each pixel in strip...
            if (((i >= tail) && (i <= head)) || //  If between head & tail...
                ((tail > head) && ((i >= tail) || (i <= head))))
            {
                pixel.setPixelColor(i, pixel.Color(0, 0, 0, 255)); // Set white
            }
            else
            { // else set rainbow
                int pixelHue = firstPixelHue + (i * 65536L / NUM_PIXELS);
                pixel.setPixelColor(i, pixel.gamma32(pixel.ColorHSV(pixelHue)));
            }
        }

        pixel.show(); // Update strip with new contents
        // There's no delay here, it just runs full-tilt until the timer and
        // counter combination below runs out.

        firstPixelHue += 40; // Advance just a little along the color wheel

        if ((millis() - lastTime) > whiteSpeed)
        { // Time to update head/tail?
            if (++head >= NUM_PIXELS)
            { // Advance head, wrap around
                head = 0;
                if (++loopNum >= loops)
                    return;
            }
            if (++tail >= NUM_PIXELS)
            { // Advance tail, wrap around
                tail = 0;
            }
            lastTime = millis(); // Save time of last movement
        }
    }
}

void pulseWhite(uint8_t wait)
{
    for (int j = 0; j < 256; j++)
    { // Ramp up from 0 to 255
        // Fill entire strip with white at gamma-corrected brightness level 'j':
        pixel.fill(pixel.Color(0, 0, 0, pixel.gamma8(j)));
        pixel.show();
        delay(wait);
    }

    for (int j = 255; j >= 0; j--)
    { // Ramp down from 255 to 0
        pixel.fill(pixel.Color(0, 0, 0, pixel.gamma8(j)));
        pixel.show();
        delay(wait);
    }
}

void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops)
{
    int fadeVal = 0, fadeMax = 100;

    // Hue of first pixel runs 'rainbowLoops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbowLoops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
         firstPixelHue += 256)
    {

        for (int i = 0; i < NUM_PIXELS; i++)
        { // For each pixel in strip...

            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (NUM_PIXELS steps):
            uint32_t pixelHue = firstPixelHue + (i * 65536L / NUM_PIXELS);

            // pixel.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the three-argument variant, though the
            // second value (saturation) is a constant 255.
            pixel.setPixelColor(i, pixel.gamma32(pixel.ColorHSV(pixelHue, 255,
                                                                255 * fadeVal / fadeMax)));
        }

        pixel.show();
        delay(wait);

        if (firstPixelHue < 65536)
        { // First loop,
            if (fadeVal < fadeMax)
                fadeVal++; // fade in
        }
        else if (firstPixelHue >= ((rainbowLoops - 1) * 65536))
        { // Last loop,
            if (fadeVal > 0)
                fadeVal--; // fade out
        }
        else
        {
            fadeVal = fadeMax; // Interim loop, make sure fade is at max
        }
    }

    for (int k = 0; k < whiteLoops; k++)
    {
        for (int j = 0; j < 256; j++)
        { // Ramp up 0 to 255
            // Fill entire strip with white at gamma-corrected brightness level 'j':
            pixel.fill(pixel.Color(0, 0, 0, pixel.gamma8(j)));
            pixel.show();
        }
        delay(1000); // Pause 1 second
        for (int j = 255; j >= 0; j--)
        { // Ramp down 255 to 0
            pixel.fill(pixel.Color(0, 0, 0, pixel.gamma8(j)));
            pixel.show();
        }
    }

    delay(500); // Pause 1/2 second
}
