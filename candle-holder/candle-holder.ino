#include <math.h>

#include <Adafruit_NeoPixel.h>

// Use this to get Serial output
// #define DEBUG
// #define DEBUG_TEMP

// data pin
#define PIN 6
// led count
#define CNT 24

// values for which light show to display
#define SHOW_FLAME 0
#define SHOW_STARS 1
#define SHOW_RAINBOW 2
#define LIGHTS_OFF 3

int show = LIGHTS_OFF;

#define NEW_STAR_TIME 2000
#define STAR_ON_MAX_TIME 500
long lastStarTime = millis();

Adafruit_NeoPixel strip = Adafruit_NeoPixel(CNT, PIN, NEO_GRB + NEO_KHZ800);

enum StarState { off, brightening, dimming };
int stars[CNT];
long starTime[CNT];


int firstHue = 0;

void _setup() {
  # if defined(DEBUG) || defined(DEBUG_TEMP)
  Serial.begin(9600);
  # endif

  pinMode(6,OUTPUT);
  //pinMode(A1, INPUT);

  randomSeed(analogRead(A0));
  show = int(random(SHOW_FLAME, SHOW_RAINBOW+1));
  for (int i = 0; i < CNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
  firstHue = random(0, 65535);
}

void setup()
{
  _setup();
}

#define TEMP_DELTA_T 1000

double temp1 = 0;
double temp2 = 0;
long temp1Time = 0;
long temp2Time = 0;

double rocTotal = 0;
int rocCount = 0;

#define ROC_COUNT_THRESHOLD 2
#define ROC_COUNT_THRESHOLD_MIN -2
int rocRaiseCount = 0;
int rocLowerCount = 0;

void loop()
{
  // read input from thermistor and assign 'show':
  // if reading is < threshold show = LIGHTS_OFF
  // if last reading is > threshold show = int(random(SHOW_FLAME, SHOW_RAINBOW+1)
  // MIN_THRESHOLD = 145?
  // int reading = analogRead(A7);
  // Serial.println(reading);

    // float temp = doTemp();
    // Serial.print("temp: ");
    // Serial.println(temp);
  // double rateOfChange = 0;
  // if (temp1 == 0) {
  //   temp1 = doTemp();
  //   temp1Time = millis();
  // }
  // if (temp2 == 0 && millis() - temp1Time >= TEMP_DELTA_T) {
  //   temp2 = doTemp();
  //   temp2Time = millis();
  //   double tempDiff = temp2 - temp1;
  //   long timeDiff = temp2Time - temp1Time;
  //   // Serial.println(tempDiff);
  //   // Serial.println(timeDiff);
  //   rateOfChange = tempDiff / double(timeDiff / 1000);
  //   temp1 = 0;
  //   temp2 = 0;
  //   temp1Time = 0;
  //   temp2Time = 0;
  //   // Serial.println(rateOfChange);
  //   rocTotal += rateOfChange;
  //   rocCount++;
  // }

  // double rocAverage = 0;
  // if (rocCount >= 10) {
  //   rocAverage = rocTotal / rocCount;
  //   rocCount = 0;
  //   rocTotal = 0;
  // }

  // #ifdef DEBUG_TEMP
  // // Serial.println(reading);
  // #endif

  // if (show != LIGHTS_OFF && rocAverage < 0) {
  //   rocLowerCount++;
  //     // Serial.print("lowercount: ");
  //     // Serial.println(rocLowerCount);
  //   if (rocLowerCount >= ROC_COUNT_THRESHOLD) {
  //     rocLowerCount = 0;
  //     rocRaiseCount = 0;
  //     // Serial.println("off");
  //     show = LIGHTS_OFF;
  //     for (int i = 0; i < CNT; i++) {
  //       strip.setPixelColor(i, strip.Color(0, 0, 0));
  //     }
  //     strip.show();
  //   }
  // } else if (show == LIGHTS_OFF && rocAverage > 0.01) {
  //   rocRaiseCount++;
  //     // Serial.print("rocRaiseCount: ");
  //     // Serial.println(rocRaiseCount);
  //   if (rocRaiseCount >= ROC_COUNT_THRESHOLD) {
  //     rocLowerCount = 0;
  //     rocRaiseCount = 0;
  //     // Serial.println("on");
  //     show = int(random(SHOW_FLAME, SHOW_RAINBOW+1));
  //   }
  // } else if (show == LIGHTS_OFF && rocAverage < 0) {
  //     // Reset the count to turn lights ON here
  //     rocRaiseCount = 0;
  //     // Serial.println("rocRaiseCount reset");
  // } else if (show != LIGHTS_OFF && rocAverage > 0.01) {
  //     // Reset the count to turn lights OFF here
  //     // Serial.println("rocLowerCount reset");
  //     rocLowerCount = 0;
  // }

  if (show == SHOW_FLAME) {
    for (int i = 0; i < CNT; i++) {
      strip.setPixelColor(i, strip.Color(160, 55, 0));
      float m = pnoise(sin(i) + i, cos(i) + i, i);
      
      long r = mapf(m, -0.5, 0.5, 0, 255) / 8;
      uint32_t diff = strip.Color(r, r / 3, r / 4);
      long rand = random(0, 10); 

      if (rand > 5) {
      SubtractColor(i, diff);
      } else {
        AddColor(i, diff);
      }
    }
    strip.show();
    delay(random(50, 100));

  } else if (show == SHOW_STARS) {
    // If NEW_STAR_TIME has elapsed: 
    //    Find a position
    //      If that is not lit & and is not next to a lit star,
    //          begin lighting that star
    //
    // Update all stars to their next brightness, given delta time.
    if (millis() - lastStarTime >= NEW_STAR_TIME) {
      int newStar = random(0, CNT);
      if (stars[newStar] == StarState::off && stars[newStar+1] == StarState::off && stars[newStar-1] == StarState::off) {
        stars[newStar] = StarState::brightening;
        lastStarTime = millis();
      }
    }
    #ifdef DEBUG
      Serial.println(lastStarTime);
      Serial.print("[ ");
      for (int i = 0; i < CNT; i++) {
        Serial.print(stars[i]);
        if (i < CNT-1) Serial.print(", ");
      }
      Serial.println("]");
    #endif
    
    for (int i = 0; i < CNT; i++) {
      switch (stars[i]) {
        case StarState::off:
          // erronous state, shouldn't happen, but kill the light if it does
          strip.setPixelColor(i, 0);
          break;
        case StarState::brightening:
          AddColor(i, strip.Color(2, 2, 2));
          if (strip.getPixelColor(i) == strip.Color(255, 255, 255)) {
            if (starTime[i] == 0) {
              starTime[i] = millis();
            } else {
              if (millis() - starTime[i] >= STAR_ON_MAX_TIME) {
                stars[i] = StarState::dimming;
                starTime[i] = 0;
              }
            }
            //stars[i] = StarState::dimming;
          }
          break;
        case StarState::dimming:
          SubtractColor(i, strip.Color(2, 2, 2));
          if (strip.getPixelColor(i) == strip.Color(0, 0, 0)) {
            stars[i] = StarState::off;
          }
          break;
      }
    }
    strip.show();
    delay(15);

  
  } else if (show == SHOW_RAINBOW) {
    strip.rainbow(firstHue);
    #ifdef DEBUG
      Serial.print("first hue: ");
      Serial.println(firstHue);
    #endif
    firstHue += 500;
    if (firstHue >= 65535) {
      firstHue = 0;
    }
    strip.show();
    delay(100);
  }
}


// pixel colour blending:
void AddColor(uint8_t position, uint32_t color)
{
uint32_t blended_color = Blend(strip.getPixelColor(position), color);
strip.setPixelColor(position, blended_color);
}

// ///
// /// Set color of LED
// ///
void SubtractColor(uint8_t position, uint32_t color)
{
uint32_t blended_color = Substract(strip.getPixelColor(position), color);
strip.setPixelColor(position, blended_color);
}

// ///
// /// Color blending
// ///
uint32_t Blend(uint32_t color1, uint32_t color2)
{
uint8_t r1,g1,b1;
uint8_t r2,g2,b2;
uint8_t r3,g3,b3;

r1 = (uint8_t)(color1 >> 16),
g1 = (uint8_t)(color1 >>  8),
b1 = (uint8_t)(color1 >>  0);

r2 = (uint8_t)(color2 >> 16),
g2 = (uint8_t)(color2 >>  8),
b2 = (uint8_t)(color2 >>  0);

return strip.Color(constrain(r1+r2, 0, 255), constrain(g1+g2, 0, 255), constrain(b1+b2, 0, 255));
}

// ///
// /// Color blending
// ///
uint32_t Substract(uint32_t color1, uint32_t color2)
{
uint8_t r1,g1,b1;
uint8_t r2,g2,b2;
uint8_t r3,g3,b3;
int16_t r,g,b;

r1 = (uint8_t)(color1 >> 16),
g1 = (uint8_t)(color1 >>  8),
b1 = (uint8_t)(color1 >>  0);

r2 = (uint8_t)(color2 >> 16),
g2 = (uint8_t)(color2 >>  8),
b2 = (uint8_t)(color2 >>  0);

r=(int16_t)r1-(int16_t)r2;
g=(int16_t)g1-(int16_t)g2;
b=(int16_t)b1-(int16_t)b2;
if(r<0) r=0;
if(g<0) g=0;
if(b<0) b=0;

return strip.Color(r, g, b);
}

 float mapf(double x, double in_min, double in_max, double out_min, double out_max)
 {
     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
 }


static const byte p[] = {   151,160,137,91,90, 15,131, 13,201,95,96,
53,194,233, 7,225,140,36,103,30,69,142, 8,99,37,240,21,10,23,190, 6,
148,247,120,234,75, 0,26,197,62,94,252,219,203,117, 35,11,32,57,177,
33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,
48,27,166, 77,146,158,231,83,111,229,122, 60,211,133,230,220,105,92,
41,55,46,245,40,244,102,143,54,65,25,63,161, 1,216,80,73,209,76,132,
187,208, 89, 18,169,200,196,135,130,116,188,159, 86,164,100,109,198,
173,186, 3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,
212,207,206, 59,227, 47,16,58,17,182,189, 28,42,223,183,170,213,119,
248,152,2,44,154,163,70,221,153,101,155,167,43,172, 9,129,22,39,253,
19,98,108,110,79,113,224,232,178,185,112,104,218,246, 97,228,251,34,
242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,
150,254,138,236,205, 93,222,114, 67,29,24, 72,243,141,128,195,78,66,
215,61,156,180
};

double fade(double t){ return t * t * t * (t * (t * 6 - 15) + 10); }
double lerp(double t, double a, double b){ return a + t * (b - a); }
double grad(int hash, double x, double y, double z)
{
int     h = hash & 15;          /* CONVERT LO 4 BITS OF HASH CODE */
double  u = h < 8 ? x : y,      /* INTO 12 GRADIENT DIRECTIONS.   */
          v = h < 4 ? y : h==12||h==14 ? x : z;
return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

#define P(x) p[(x) & 255]

double pnoise(double x, double y, double z)
{
int   X = (int)floor(x) & 255,             /* FIND UNIT CUBE THAT */
      Y = (int)floor(y) & 255,             /* CONTAINS POINT.     */
      Z = (int)floor(z) & 255;
x -= floor(x);                             /* FIND RELATIVE X,Y,Z */
y -= floor(y);                             /* OF POINT IN CUBE.   */
z -= floor(z);
double  u = fade(x),                       /* COMPUTE FADE CURVES */
        v = fade(y),                       /* FOR EACH OF X,Y,Z.  */
        w = fade(z);
int  A = P(X)+Y, 
     AA = P(A)+Z, 
     AB = P(A+1)+Z,                        /* HASH COORDINATES OF */
     B = P(X+1)+Y, 
     BA = P(B)+Z, 
     BB = P(B+1)+Z;                        /* THE 8 CUBE CORNERS, */

return lerp(w,lerp(v,lerp(u, grad(P(AA  ), x, y, z),   /* AND ADD */
                          grad(P(BA  ), x-1, y, z)),   /* BLENDED */
              lerp(u, grad(P(AB  ), x, y-1, z),        /* RESULTS */
                   grad(P(BB  ), x-1, y-1, z))),       /* FROM  8 */
            lerp(v, lerp(u, grad(P(AA+1), x, y, z-1),  /* CORNERS */
                 grad(P(BA+1), x-1, y, z-1)),          /* OF CUBE */
              lerp(u, grad(P(AB+1), x, y-1, z-1),
                   grad(P(BB+1), x-1, y-1, z-1))));
}


// temp stuff:
// which analog pin to connect
#define THERMISTORPIN A7         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 4050
// the value of the 'other' resistor
#define SERIESRESISTOR 10000 

int samples[NUMSAMPLES];

double doTemp() {
  uint8_t i;
  double average;

  // take N samples in a row, with a slight delay
  // for (i=0; i< NUMSAMPLES; i++) {
  //  samples[i] = analogRead(THERMISTORPIN);
  // }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
    //  average += samples[i];
     average += analogRead(THERMISTORPIN);
  }
  average /= NUMSAMPLES;

  // Serial.print("Average analog reading "); 
  // Serial.println(average);
  
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  // Serial.print("Thermistor resistance "); 
  // Serial.println(average);
  
  double steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C
  
  #ifdef DEBUG_TEMP
  // Serial.println(steinhart);
  #endif

  return steinhart;
}