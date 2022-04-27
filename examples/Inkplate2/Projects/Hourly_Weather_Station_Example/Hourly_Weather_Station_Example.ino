/*
   Real time Weather station example for e-radionica.com Inkplate 2
   For this example you will need only USB cable and Inkplate 2.
   Select "Inkplate 2(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 2(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will show you how you can use Inkplate 2 to display API data,
   e.g. OpenWeatherMap API, and weatherstack for real time data

   IMPORTANT:
   Make sure to change your desired city, timezone and wifi credentials below
   Also have ArduinoJSON installed in your Arduino libraries

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   5 April 2022 by e-radionica.com
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE2
#error "Wrong board selection for this example, please select Inkplate 2 in the boards menu."
#endif

//---------- CHANGE HERE  -------------:

// Time zone for adding hours
int timeZone = 2;

// City name to de displayed on the bottom
char city[128] = "OSIJEK";

// Coordinates sent to the api
char lon[] = "18.5947808";
char lat[] = "45.5510548";

// Change to your wifi ssid and password
char ssid[] = "";
char pass[] = "";

// Uncomment this for MPH and Fahrenheit output, also uncomment it in the begining of Network.cpp
// #define AMERICAN

// Change to your api key, if you don't have one, head over to:
// https://openweathermap.org/guide , register and copy the key provided
char apiKey[] = "bf11bcaae795116ccc21beec25850aa0";

//----------------------------------

// Include Inkplate library to the sketch
#include "Inkplate.h"

// Header file for easier code readability
#include "Network.h"

// Including fonts used
#include "Fonts/Inter8pt7b.h"

// Including icons generated by the py file
#include "icons.h"

// Delay between API calls, about 1000 per month, which is the free tier limit
#define DELAY_MS 267800L

// Inkplate object
Inkplate display;

// All our network functions are in this object, see Network.h
Network network;

// Contants used for drawing icons
char abbrs[32][32] = {"01d", "02d", "03d", "04d", "09d", "10d", "11d", "13d", "50d",
                      "01n", "02n", "03n", "04n", "09n", "10n", "11n", "13n", "50n"
                     };

const uint8_t *s_logos[18] = {
  icon_s_01d, icon_s_02d, icon_s_03d, icon_s_04d, icon_s_09d, icon_s_10d, icon_s_11d, icon_s_13d, icon_s_50d,
  icon_s_01n, icon_s_02n, icon_s_03n, icon_s_04n, icon_s_09n, icon_s_10n, icon_s_11n, icon_s_13n, icon_s_50n,
};

// Variables for storing temperature
char temps[4][8] = {
  "0F",
  "0F",
  "0F",
  "0F",
};

// Variables for storing hour strings
char hours[4][8] = {
  "",
  "",
  "",
  "",
};

// Variable for counting partial refreshes
RTC_DATA_ATTR long refreshes = 0;

// Constant to determine when to full update
const int fullRefresh = 10;

// Variables for storing current time and weather info
char currentTemp[16] = "0F";
char currentWind[16] = "0m/s";

char currentTime[16] = "9:41";

char currentWeather[32] = "-";
char currentWeatherAbbr[8] = "01d";

char abbr1[16];
char abbr2[16];
char abbr3[16];
char abbr4[16];

// function defined below
void drawTemps();

void setup()
{
  // Begin serial and display
  Serial.begin(115200);
  display.begin();

  // Initial cleaning of buffer and physical screen
  display.clearDisplay();

  // Calling our begin from network.h file
  network.begin(city);

  // Wait a bit before proceeding
  delay(5000);

  // Clear display
  display.clearDisplay();

  // Get all relevant data, see Network.cpp for info
  network.getTime(currentTime);

  while (!network.getData(city, temps[0], temps[1], temps[2], temps[3], currentTemp, currentWind, currentTime,
                          currentWeather, currentWeatherAbbr, abbr1, abbr2, abbr3, abbr4))
  {
    Serial.println("Retrying fetching data!");
    delay(5000);
  }
  network.getHours(hours[0], hours[1], hours[2], hours[3]);


  // Draw data, see functions below for info
  drawTemps();

  // Refresh full screen every fullRefresh times, defined above

  display.display();

  // Go to sleep before checking again

  ++refreshes;
  esp_sleep_enable_timer_wakeup(1000L * DELAY_MS);
  (void)esp_deep_sleep_start();
}

void loop()
{
}

// Function for drawing temperatures
void drawTemps()
{
  // Drawing 4 black rectangles in which temperatures will be written

  display.drawRect(5, 5, 60, 94, BLACK); //Draw some rectangles
  display.drawRect(75, 5, 60, 94, BLACK);
  display.drawRect(145, 5, 60, 94, BLACK);

  display.setFont(&Inter8pt7b); //Set custom font
  display.setTextSize(1); //Set font size
  display.setTextColor(RED, BLACK); //Set font color and background color

  display.setCursor(18, 20);// Set cursor, custom font uses different method for setting cursor
                              // You can find more about that here https://learn.adafruit.com/adafruit-gfx-graphics-library/using-fonts
  display.println("Now");

  display.setCursor(92, 20);
  display.println(hours[1]);

  display.setCursor(162, 20);
  display.println(hours[2]);

  // Drawing temperature values into black rectangles
  display.setTextColor(BLACK, WHITE);

  display.setCursor(20, 40);
  display.print(temps[0]);
  display.println(F("C"));

  display.setCursor(90, 40);
  display.print(temps[1]);
  display.println(F("C"));

  display.setCursor(160, 40);
  display.print(temps[2]);
  display.println(F("C"));

  for (int i = 0; i < 18; ++i)
  {
    // If found draw specified icon
    if (strcmp(abbr1, abbrs[i]) == 0)
      display.drawBitmap(11, 48, s_logos[i], 48, 48,
                         BLACK, WHITE);
  }

  for (int i = 0; i < 18; ++i)
  {
    // If found draw specified icon
    if (strcmp(abbr2, abbrs[i]) == 0)
      display.drawBitmap(81, 48, s_logos[i], 48, 48,
                         BLACK, WHITE);
  }

  for (int i = 0; i < 18; ++i)
  {
    // If found draw specified icon
    if (strcmp(abbr3, abbrs[i]) == 0)
      display.drawBitmap(151, 48, s_logos[i], 48, 48,
                         BLACK, WHITE);
  }
}
