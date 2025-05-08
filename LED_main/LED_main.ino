#include "esp32_digital_led_lib.h"

/*
 * Simple example showing a single strand of NeoPixels. See Demo1 for multiple strands and other devices.
 */

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define GPIO_SK6812 13
#define N_LED_SQUARE 3 // Nombre de leds par "pixel"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"  // It's noisy here with `-Wall`


strand_t strand = {.rmtChannel = 0, .gpioNum = GPIO_SK6812, .ledType = LED_SK6812_V1, .brightLimit = 255, .numPixels = 144};
strand_t * STRANDS [] = {&strand};
int STRANDCNT = COUNT_OF(STRANDS);
#pragma GCC diagnostic pop

int stepper = 0;
int colord = 0;
unsigned int delay_ms;
unsigned long int lastime_color_change = millis();

enum{
  COLOR_RED = 0,
  COLOR_GREEN,
  COLOR_BLUE,
  NUM_COLORS
};

pixelColor_t strand_colors[NUM_COLORS];

//**************************************************************************//
void randomStrands(strand_t * strands[], int numStrands, unsigned long delay_ms, unsigned long timeout_ms)
{
  Serial.print("DEMO: random colors, delay = ");
  Serial.println(delay_ms);
  uint32_t dimmer = 0x0F3F3F3F;
  unsigned long start_ms = millis();
  while (timeout_ms == 0 || (millis() - start_ms < timeout_ms)) {
    for (int n = 0; n < numStrands; n++) {
      strand_t * pStrand = strands[n];
      for (uint16_t i = 0; i < pStrand->numPixels; i++) {
        pStrand->pixels[i].num = (esp_random() & dimmer);
      }
    }
    digitalLeds_drawPixels(strands, numStrands);
    delay(delay_ms);
  }
}

//**************************************************************************//
void init_leds()
{
  strand_colors[COLOR_RED]   = pixelFromRGB(255,0,0);
  strand_colors[COLOR_GREEN] = pixelFromRGB(0,255,0);
  strand_colors[COLOR_BLUE]  = pixelFromRGB(0,0,255);

  digitalLeds_initDriver();
  gpio_num_t gpioNumNative = static_cast<gpio_num_t>(strand.gpioNum);
  gpio_mode_t gpioModeNative = static_cast<gpio_mode_t>(OUTPUT);
  gpio_pad_select_gpio(gpioNumNative);
  gpio_set_direction(gpioNumNative, gpioModeNative);
  gpio_set_level(gpioNumNative, LOW);
  
  int rc = digitalLeds_addStrands(STRANDS, STRANDCNT);
  if (rc)
  {
    Serial.print("Init rc = ");
    Serial.println(rc);
  }

  if (digitalLeds_initDriver())
  {
    Serial.println("Init FAILURE: halting");
    while (true) {};

  }
  digitalLeds_resetPixels(STRANDS, STRANDCNT);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  init_leds();
  change_color_squares();
  delay_ms = 8000;
}

//**************************************************************************//
void allumer_carre(strand_t * strands [], int numStrands, int n_carre, pixelColor_t couleur)
{
  strand_t *ptr_strand = strands[0];
  for (int i=n_carre*N_LED_SQUARE; i < (n_carre+1)*N_LED_SQUARE; i++)
  {
    ptr_strand->pixels[i] = couleur;
  }
}

void change_color_squares()
{
  int i = 0;
  for(int x = 0; x < 4; x++){
    for(int y = 0; y < 4; y++){
      int n_color = random(0,NUM_COLORS);
      allumer_carre(STRANDS, STRANDCNT, y*4+x, strand_colors[n_color]);
    }
  }
  digitalLeds_drawPixels(STRANDS, STRANDCNT);
}

//**************************************************************************//
void loop()
{
  if(millis() - lastime_color_change > delay_ms)
  {
    // on change la couleur des "pixels" tout les "delay_ms" ms
    lastime_color_change = millis();
    change_color_squares();
  }
}
