/*  This is the program for the Pixel Chase arcade game
 *  Made by Raphael Texier and Johan Ondet
 *  
 *  Gamerule for pixels chase :
 *  - at least 1 pixel has to be "blue"
 *  - delay between color change can decrease but we need a max and a min
 *  - maybe we should be using fixed patterns ? + 3/5 of the time it is random => more difficulty, more random maybe ?
 *  - possible to hit 2 or more "pixels" at the same time
 *  - 6 difficulty for patterns : (Insane ?), Super Hard, Hard, Normal, Easy + Special
 *  - Special pattern can be triggered by doing something special in the game - hit X point in each second ? / Hit many pixels in a row ?   
 *  - difficulty goes in order : Easy, Normal ... Insane
 *  - one of the pattern is gonna be random (but following the rules)
 *  - Un power up qui active un pixel qui vaut chère mais très compliqué à hit ?
 *  
 * 2 modes:
 * - Normal mode -> "0" pixels does nothing
 * - Survival mode -> "0" pixels kill you ?
 * 
 *  Patterns rules (0: RED, 1: GREEN + 30, 2: BLUE + 100):
 *  
 *  - difficulty : insane ?
 *    only one pixel is lit in blue, other are red
 *  
 *  - difficulty : Super Hard
 *    random : (1 BLUE, 2 GREEN, other RED)
 *  
 *  - difficulty : Hard
 *    random : (2 BLUE, 2 GREEN, other RED)
 *  
 *  - difficulty : Normal
 *    random : (4 BLUE, 4 GREEN, 8 RED)
 *
 *  - difficulty : Easy
 *    random : (4 BLUE, 10 GREEN, 2 RED)
 *  
 *  Dynamisme:
 *  un seul carre bleu qui bouge en mode snake ?
 *  2 patterns alternatifs qui changent toutes les 2 secondes par exemple  <- plutôt ça je pense ou un truc ou on alterne les deux sinon trop plat 
 *  
 */


#define USE_ADAFRUIT_GFX_LAYERS
// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
#define GPIOPINOUT ESP32_JC_RIBBON_PINOUT
#include <MatrixHardware_ESP32_V0.h>     // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
#include <SmartMatrix.h>
#include <Adafruit_MCP23X17.h>
#include "esp32_digital_led_lib.h"
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// leds
#define GPIO_SK6812 13
#define N_LED_SQUARE 3 // Nombre de leds par "pixel"

// SPI
#define CS_PIN   26
#define SCK_PIN  32
#define MISO_PIN 25
#define MOSI_PIN 33

#define PIN_BUTTON 35
#define IR_PIN 21 // Pin where the IR LED is connected
#define INTERRUPT_PIN 34 // Interrupt pin MCP23S17

Adafruit_MCP23X17 mcp;

int random_pixel_count[][3] =
{// R, G, B
  {2, 10, 4}, // easy
  {8,  4, 4},
  {12, 2, 2},
  {13, 2, 1}  // super hard
};

#define N_PATTERNS_EASY 7
int patterns_easy[N_PATTERNS_EASY*4*4] =
{
  1,1,2,1,
  2,1,1,1,
  1,1,1,2,
  1,2,1,1,

  0,1,1,0,
  1,2,2,1,
  1,2,2,1,
  0,1,1,0,

  2,1,1,2,
  1,1,0,1,
  1,0,1,1,
  2,1,1,2,

  2,1,1,2,
  1,0,1,1,
  1,1,0,1,
  2,1,1,2,
  
  1,2,1,1,
  1,1,1,2,
  2,1,1,1,
  1,1,2,1,

  1,0,0,1,
  2,1,1,2,
  2,1,1,2,
  1,0,0,1,

  1,2,2,1,
  0,1,1,0,
  0,1,1,0,
  1,2,2,1
};

#define N_PATTERNS_MEDIUM 5
int patterns_medium[N_PATTERNS_MEDIUM*4*4] =
{
  0,1,2,1,
  2,1,0,1,
  1,0,1,2,
  1,2,1,0,

  1,0,0,1,
  0,2,2,0,
  0,2,2,0,
  1,0,0,1,
  
  2,0,0,2,
  0,1,2,0,
  0,2,1,0,
  2,0,0,2,

  1,0,2,1,
  2,1,1,0,
  0,1,1,2,
  1,2,0,1,

  0,2,2,0,
  1,0,0,1,
  1,0,0,1,
  0,2,2,0
};

#define N_PATTERNS_HARD 4
int patterns_hard[N_PATTERNS_HARD*4*4] =
{
  0,1,2,0,
  2,0,0,1,
  1,0,0,2,
  0,2,1,0,

  0,0,0,0,
  0,1,2,0,
  0,2,1,0,
  0,0,0,0,

  1,1,0,2,
  0,2,0,1,
  1,0,2,0,
  2,0,1,1,

  1,0,2,0,
  2,0,1,0,
  0,1,0,2,
  0,2,0,1
};

#define N_PATTERNS_SUPER_HARD 2
int patterns_super_hard[N_PATTERNS_SUPER_HARD*4*4] =
{
  1,0,0,2,
  0,1,0,0,
  0,0,1,0,
  2,0,0,1,
  
  2,0,0,2,
  0,0,0,0,
  0,0,0,0,
  2,0,0,2
};

int sensor_changed = 0;

int sensor_n_to_xy[][2] = {
//{x,y},
  {0,0}, // pin 0
  {0,1},
  {0,2},
  {0,3},
  
  {1,0},
  {1,1},
  {1,2},
  {1,3},
  
  {2,0},
  {2,1},
  {2,2},
  {2,3},
  
  {3,0},
  {3,1},
  {3,2},
  {3,3} // pin 16
};

int sensor_cases[4][4] = 
{
  {0,0,0,0},                     
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};

#define MIN_TIME_BETWEEN_BALLS (0.08*2.5/10)*1000 // t = (d/Vmax)/1000 avec d = diametre * 2.5 et Vmax = 10m/s
unsigned long int last_time_sensor[4][4] = 
{
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};

int color_cases[4][4] = 
{
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {0,0,0,0}
};

enum{
  COLOR_RED = 0,
  COLOR_GREEN,
  COLOR_BLUE,
  NUM_COLORS
};

#define SCORE_RED     0
#define SCORE_GREEN  30
#define SCORE_BLUE  100

enum difficulty{
  EASY = 0,
  MEDIUM,
  HARD,
  SUPER_HARD,
  COUNT_DIFFICULTY
};

enum gamemodes{
  NORMAL,
  SURVIVAL
};

int delays[COUNT_DIFFICULTY] =
{
  45*1000, // easy
  40*1000,
  35*1000,
  30*1000  // super hard
};

#define DELAY_SPECIAL_MODE 25*1000

pixelColor_t strand_colors[NUM_COLORS];

int game;
int button_pressed;
bool random_pixel_done[16];

strand_t strand = {.rmtChannel = 0, .gpioNum = GPIO_SK6812, .ledType = LED_SK6812_V1, .brightLimit = 255, .numPixels = 144};
strand_t * STRANDS [] = {&strand};
unsigned int STRANDCNT = COUNT_OF(STRANDS);

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 128;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // This isn't used on ESP32, leave as default
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

// Common colors for the 128*32 matrix
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define size_pixel 4
#define rows (32/size_pixel)
#define columns (128/size_pixel)

#define NUM_BALLS 30

double Convert(double degree){
    double pi = 3.14;
    return (degree * (pi / 180));
}

float coord[NUM_BALLS*2] = {2,2, 5,5};
float angles[NUM_BALLS] = {Convert(80), Convert(12)};

void bouncingBalls(){
  unsigned long int last_update = millis();
  unsigned long int begin_bouncing_balls = millis();
  const unsigned int vitesse = 2;
  const unsigned int size_balls = 3;
  float x = 2;
  float y = 2;
  float angle = Convert(80);
  backgroundLayer.fillScreen(BLACK);
  
  while(millis() - begin_bouncing_balls < 2500){
    waitForGame(1);
    if(millis() - last_update > 18){
      for(int i=0; i<NUM_BALLS*2; i+=2){ // on efface les anciens pixels
        x = coord[i];
        y = coord[i+1];
        angle = angles[i/2];
        backgroundLayer.fillCircle(int(x)-1, int(y)-1, size_balls, BLACK);
        last_update = millis();

        // test collisions
        if(y >= 29 or y <= 4){
          angle = -angle;
        }
        else if(x >= 125 or x <= 4){
          angle = Convert(180)-angle;
        }
         
        angles[i/2] = angle;
    
        x += vitesse*cos(angle);
        y += vitesse*sin(angle);
        
        coord[i] = x;
        coord[i+1] = y;
      }
    
      for(int i=0; i<NUM_BALLS*2 ; i+=2){ // on affiche les nouveaux pixels
        x = coord[i];
        y = coord[i+1];
        backgroundLayer.fillCircle(int(x)-1, int(y)-1, size_balls, RED);
      }
      backgroundLayer.setCursor(20, 6);
      backgroundLayer.setTextColor(WHITE);  backgroundLayer.setTextSize(3);
      backgroundLayer.print("Balls");
      
      backgroundLayer.swapBuffers();
    }
  }
}

void waitForGame(unsigned long int delay_ms)
{
  unsigned long int start_time = millis();
  do
  {
    if(game)
      startGame(NORMAL);
  } while (millis() - start_time < delay_ms);
}

void spawnPixels(int pixels[columns][rows]){
  int random_bool;
  for(int x=0; x<columns; x++){
    random_bool = random(0, 2);
    if(random_bool != 0){
      pixels[x][0] = 1;
      backgroundLayer.fillRect(x*size_pixel, 0, size_pixel, size_pixel, GREEN);
    }
  }
}

void PixelsAnimation()
{
  int pixels[columns][rows];

  // init all pixels to "0"
  for(int x=0; x<columns; x++){
    for(int y=0; y<rows; y++){
      pixels[x][y] = 0;
    }
  }

  spawnPixels(pixels);
  
  unsigned long int begin_pixels = millis();
  unsigned long int last_update = millis();

  backgroundLayer.fillScreen(BLACK);
  
  while(millis() - begin_pixels < 4000){
    waitForGame(1);
    
    if(millis() - last_update > 100){
      last_update = millis();

      bool filled = 1;
      
      // we check that the screen isn't already filled
      for(int x=0; x<columns; x++){
        for(int y=0; y<rows; y++){
          if (pixels[x][y] == 0){
            filled = 0;
            break;
          }
        }
      }
  
      // we put every pixel 1 pixel down
      // if(not(filled)){
      if(1){
        for(int x=0; x<columns; x++){
          for(int y=rows; y>0; y--){
            if (pixels[x][y-1] == 1){
              if (pixels[x][y] == 0){
                pixels[x][y] = 1;
                backgroundLayer.fillRect(x*size_pixel, y*size_pixel, size_pixel, size_pixel, GREEN);
                pixels[x][y-1] = 0;
                backgroundLayer.fillRect(x*size_pixel, (y-1)*size_pixel, size_pixel, size_pixel, BLACK);
              }
            }
          }
        }
        if(millis() - begin_pixels > 2600) // after 2.6 seconds even is the screen isn't perfectly filled we fill it to avoid weird behavior
          backgroundLayer.fillScreen(GREEN);
        
        spawnPixels(pixels);
        backgroundLayer.setCursor(6, 2);
        backgroundLayer.setTextColor(BLACK);  backgroundLayer.setTextSize(4);
        backgroundLayer.print("PIXEL");
        backgroundLayer.swapBuffers();
      }
    }
  }
}

void printScroll(char* text)
{
  uint16_t x = 80;
  for(uint16_t i=0; i < 270; i++)
  {
    backgroundLayer.fillScreen(BLACK);
    backgroundLayer.setCursor(x, 2);
    backgroundLayer.setTextWrap(false);  // we don't wrap text so it scrolls nicely
    backgroundLayer.setTextColor(WHITE);  backgroundLayer.setTextSize(4);
    backgroundLayer.print(text);
    backgroundLayer.swapBuffers();
    x-= 1;
    waitForGame(10);
  }
}

void animate()
{
  printScroll("PIXEL-CHASE");

  for(int i = 0; i < NUM_BALLS*2; i+=2)
  {
    coord[i] = random(2,126);
    coord[i+1] = random(2,30);
    angles[i/2] = Convert(random(0,359));
  }
  bouncingBalls();

  backgroundLayer.fillScreen(BLACK);
  backgroundLayer.setCursor(30, 2);
  backgroundLayer.setTextColor(WHITE);  backgroundLayer.setTextSize(4);
  backgroundLayer.print("And");
  backgroundLayer.swapBuffers();
  waitForGame(1200);

  PixelsAnimation();
}

void printScore(unsigned int score, const int color)
{
  char score_str[5]; // max score is 99999
  sprintf(score_str, "%05d", score); // return a c string (char array) with zeros to complete the left part
  backgroundLayer.fillScreen(BLACK);
  backgroundLayer.setCursor(6, 2);
  backgroundLayer.setTextColor(color); backgroundLayer.setTextSize(4);
  backgroundLayer.print(score_str);
  backgroundLayer.swapBuffers();
  
  /*
  backgroundLayer.setCursor(1, 1);
  backgroundLayer.setTextColor(MAGENTA);  backgroundLayer.setTextSize(2);
  backgroundLayer.print("Score :");
  backgroundLayer.setCursor(1, 17);
  backgroundLayer.setTextColor(CYAN); backgroundLayer.setTextSize(2);
  backgroundLayer.print(score_str);
  */
}

void printMatrixCenter(const char* text, const int text_size,  const int color)
{
  backgroundLayer.fillScreen(BLACK);
  backgroundLayer.setCursor(0, 0);
  backgroundLayer.setTextColor(color); 
  backgroundLayer.setTextSize(text_size);

  int16_t  x1, x2;
  uint16_t w, h;
  backgroundLayer.getTextBounds(text, 0, 0, &x1, &x2, &w, &h);
  backgroundLayer.setCursor(((128-w) >> 1) + 2, ((32-h) >> 1) + 2);
  backgroundLayer.print(text);
  backgroundLayer.swapBuffers();
}

void text3D(const char *text, int color, bool inverted = 0)
{
  if(inverted)
  {
    for(int i = 4; i > 0; i--){
      printMatrixCenter(text, i, color);
      delay(100);
    }
  }
  else
  {
    for(int i = 0; i < 4; i++){
      printMatrixCenter(text, i, color);
      delay(100);
    }
  }
}

void writeLedPixel(strand_t * strands [], int numStrands, int n_pixel, pixelColor_t color)
{
  strand_t *ptr_strand = strands[0];
  for(int i=n_pixel*N_LED_SQUARE; i < (n_pixel+1)*N_LED_SQUARE; i++){
    ptr_strand->pixels[i] = color;
  }
}

void randomPixel(int color, int count_pixel)
{
  int random_pixel;
  for(int i = 0; i < count_pixel; i++)
  {
    int count = 0;
    do{ // get a pixel that hasn't been colored yet
    random_pixel = random(0,16); 
    count++;
    } while (random_pixel_done[random_pixel] and count < 18);

    int x = sensor_n_to_xy[random_pixel][0];
    int y = sensor_n_to_xy[random_pixel][1];
    
    writeLedPixel(STRANDS, STRANDCNT, y*4+x, strand_colors[color]);
    color_cases[y][x] = color;
    random_pixel_done[random_pixel] = 1;
  }
}

void changeColorSquares(int difficulty, bool random_color = false)
{
  int n_pattern;
  int *patterns_ptr;
  int n_color;
  switch(difficulty)
  {
    case EASY:
      n_pattern = random(0,N_PATTERNS_EASY);
      patterns_ptr = patterns_easy;
    break;
    case MEDIUM:
      n_pattern = random(0,N_PATTERNS_MEDIUM);
      patterns_ptr = patterns_medium;
    break;
    case HARD:
      n_pattern = random(0,N_PATTERNS_HARD);
      patterns_ptr = patterns_hard;
    break;
    case SUPER_HARD:
      n_pattern = random(0,N_PATTERNS_SUPER_HARD);
      patterns_ptr = patterns_super_hard;
    break;
  }

  Serial.print("difficulty: ");
  Serial.println(difficulty);
  if(random_color)
  {
    int count_red   = random_pixel_count[difficulty][COLOR_RED];
    int count_green = random_pixel_count[difficulty][COLOR_GREEN];
    int count_blue  = random_pixel_count[difficulty][COLOR_BLUE];
    
    Serial.println("random screen pixels:");
  
    randomPixel(COLOR_RED,   count_red);
    randomPixel(COLOR_GREEN, count_green);
    randomPixel(COLOR_BLUE,  count_blue);
    
    for(int x = 0; x < 4; x++){
      for(int y = 0; y < 4; y++){
        Serial.print(color_cases[y][x]);
      }
      Serial.print("\n");
    }
  }
  else
  {
    Serial.println("screen pixels:");
    for(int x = 0; x < 4; x++){
      for(int y = 0; y < 4; y++){
        //n_color = random(0,NUM_COLORS);
        n_color = patterns_ptr[n_pattern*4*4 + y*4 + x];
        writeLedPixel(STRANDS, STRANDCNT, y*4+x, strand_colors[n_color]);
        color_cases[y][x] = n_color;
        Serial.print(n_color);
      }
      Serial.print("\n");
    }
  }
  digitalLeds_drawPixels(STRANDS, STRANDCNT);
}

void IRAM_ATTR sensor_ISR()
{
  sensor_changed = mcp.getLastInterruptPin();
}

void initSensors()
{
  //tone(IR_PIN, 38000); // 38 kHz frequency
  
  if(!mcp.begin_SPI(CS_PIN, SCK_PIN, MISO_PIN, MOSI_PIN))
  {
    Serial.println("Error.");
    while(1);
  }

  pinMode(INTERRUPT_PIN, INPUT);
  attachInterrupt(INTERRUPT_PIN, sensor_ISR, FALLING);
  /*
   * LOW : Déclenche l’interruption dès que le signal est à 0V
   * HIGH : Déclenche l’interruption dès que le signal est à 3.3V
   * RISING : Déclenche l’interruption dès que le signal passe de LOW à HIGH (0 à 3.3V)
   * FALLING : Déclenche l’interruption dès que le signal passe de HIGH à LOW (3.3V à 0)
   */

  // OPTIONAL - call this to override defaults
  // mirror INTA/B so only one wire required
  // active drive so INTA/B will not be floating
  // INTA/B will be signaled with a LOW
  mcp.setupInterrupts(true, false, LOW);
  
  // configure all 16 pins for input with interrupts
  //for(int i = 0; i < 16; i++)
  for(int i = 0; i < 1; i++)
  {
    mcp.pinMode(i, INPUT_PULLUP);
    mcp.setupInterruptPin(i, LOW);
  }

  mcp.clearInterrupts();  // clear
}

void initLeds()
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
  if(rc)
  {
    Serial.print("Init rc = ");
    Serial.println(rc);
  }

  if(digitalLeds_initDriver())
  {
    Serial.println("Init FAILURE: halting");
    while(1);

  }
  digitalLeds_resetPixels(STRANDS, STRANDCNT);
}

void IRAM_ATTR button_isr()
{
  game = 1;
}

void startGame(int gamemode)
{
  int difficulty = 0;
  int random_difficulty = random(0, COUNT_DIFFICULTY); // level wich is gonna be "random"
  unsigned int score = 0;
  unsigned int delay_ms = delays[difficulty];
  for(int i = 0; i < 16; i++)
    random_pixel_done[i] = 0;

  text3D("START", CYAN);
  text3D("START", CYAN, 1);
  text3D("START", CYAN);
  delay(1000);
  
  text3D("3", CYAN);
  delay(1000);
  text3D("2", CYAN);
  delay(1000);
  text3D("1", CYAN);
  delay(1000);

  backgroundLayer.fillScreen(BLACK);
  printScore(score, CYAN);
  changeColorSquares(difficulty, (difficulty == random_difficulty));
  unsigned long int lastime_color_change = millis();
  
  while(game)
  {
    if(millis() - lastime_color_change > delay_ms)
    {
      // on change la couleur des "pixels" tout les "delay_ms" ms
      lastime_color_change = millis();
      if(difficulty < COUNT_DIFFICULTY-1)
      {
        difficulty += 1;
        changeColorSquares(difficulty, (difficulty == random_difficulty));
        delay_ms = delays[difficulty];
      }
      else
      {
        game = 0;
      }
    }
  
    if(sensor_changed)
    {
      Serial.print("GPIO triggered :  ");
      Serial.println(sensor_changed);
      int x = sensor_n_to_xy[sensor_changed][0];
      int y = sensor_n_to_xy[sensor_changed][1];

      if(millis() - last_time_sensor[y][x] > MIN_TIME_BETWEEN_BALLS)
      {
        last_time_sensor[y][x] = millis();
        switch(color_cases[y][x])
        {
          case COLOR_GREEN:
            score += SCORE_GREEN;
            break;
          case COLOR_RED:
            score += SCORE_RED;
            break;
          case COLOR_BLUE:
            score += SCORE_BLUE;
            break;
        }
        sensor_changed = 0;
        printScore(score, CYAN);
      }
    }
    mcp.clearInterrupts(); // we call often this so that new interrupts can be registered
  }
  
  digitalLeds_resetPixels(STRANDS, STRANDCNT);
}

void setup()
{
  Serial.begin(115200);
  // wait for Serial to be ready
  delay(100);

  pinMode(PIN_BUTTON, INPUT);
  attachInterrupt(PIN_BUTTON, button_isr, FALLING);
  
  initSensors();
  initLeds();
  
  matrix.addLayer(&backgroundLayer);
  matrix.begin();
  matrix.setBrightness(255);
  
  backgroundLayer.fillScreen(BLACK);
  backgroundLayer.swapBuffers();
}

void loop()
{
  animate();

  waitForGame(1);
}
