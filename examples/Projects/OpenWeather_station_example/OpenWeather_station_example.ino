/*
   Weather station example for e-radionica.com Inkplate 6
   For this example you will need only USB cable and Inkplate 6.
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will show you how you can use Inkplate 6 to display API data,
   e.g. Metaweather public weather API

   IMPORTANT:
   Make sure to change your desired city, timezone and wifi credentials below
   Also have ArduinoJSON installed in your Arduino libraries

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   28 July 2020 by e-radionica.com
*/

//WiFi Connection required
#include <WiFi.h>

//Required libraries
#include <ArduinoJson.h>
#include "HTTPClient.h"
#include <TimeLib.h>
#include <Time.h>

// ---------- CHANGE HERE  -------------:

// Change to your wifi ssid and password

#include <OpenWeatherOneCall.h>
#define HOMESSID ""
#define HOMEPW ""

// Openweather set up information
#define ONECALLKEY ""

// Your local coordinates

float myLatitude = 0.00000; //I got this from Wikipedia
float myLongitude = 0.00000;

char *city = "My City";

bool metric = true; //<------------------------------TRUE is METRIC, FALSE is IMPERIAL, BLANK is KELVIN

// ----------------------------------

// Include Inkplate library to the sketch
#include "Inkplate.h"

// Including fonts used
#include "Fonts/Roboto_Light_48.h"
#include "Fonts/Roboto_Light_36.h"
#include "Fonts/Roboto_Light_120.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// Including icons generated by the py file
#include "icons.h"

// Delay between API calls
#define DELAY_MS 60000

// Inkplate object
Inkplate display(INKPLATE_3BIT);

// Contants used for drawing icons
char abbrs[32][16] = { "sn", "sl", "h", "t", "hr", "lr", "s", "hc", "lc", "c" };
const uint8_t *logos[16] = { icon_sn, icon_sl, icon_h, icon_t, icon_hr, icon_lr, icon_s, icon_hc, icon_lc, icon_c };
const uint8_t *s_logos[16] = { icon_s_sn, icon_s_sl, icon_s_h, icon_s_t, icon_s_hr, icon_s_lr, icon_s_s, icon_s_hc, icon_s_lc, icon_s_c };


char abbr1[16];
char abbr2[16];
char abbr3[16];
char abbr4[16];

// Variables for storing temperature
char temps[8][4] = {
  "0F",
  "0F",
  "0F",
  "0F",
};

// Variables for storing days of the week
char days[8][4] = {
  "",
  "",
  "",
  "",
};

// Variable for counting partial refreshes
char refreshes = 0;

// Constant to determine when to full update
const int fullRefresh = 20;

// Constant to determine when to full update
const int dayOffset = 300;

// Variables for storing current time and weather info
char currentTemp[16] = "0F";
char currentWind[16] = "0m/s";

char currentTime[16] = "9:41";

char currentWeather[32] = "-";
char currentWeatherAbbr[8] = "th";

// functions defined below
void drawWeather();
void drawCurrent();
void drawTemps();
void drawCity();
void drawTime();

char Output[200] = {0};

OpenWeatherOneCall OWOC; // Invoke OpenWeather Library
time_t t = now();

void connectWifi() {

  Serial.println("Connect WiFi");
  WiFi.begin(HOMESSID, HOMEPW);

  Serial.print("Connecting.");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nConnected to: ");
  Serial.println(HOMESSID);
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected WiFi");
} //======================== END WIFI CONNECT =======================

void GetCurrentWeather() {
  //=================================
  // Get the Weather Forecast
  //=================================

  connectWifi();
  
  Serial.println("Getting weather");
  OWOC.parseWeather(ONECALLKEY, NULL, myLatitude, myLongitude, metric, NULL);
  setTime(OWOC.current.dt + 7200);
  t = now();

  //=================================================
  // Today's results
  //==================l===============================
  sprintf(Output,"%s: %02d:%02d,%02d:%02d-%02d:%02d,%02.01fC,%02.01fC,%04.0fhPa,%02.01f%% Rh,%02.01fC,%03d%%,UV:%02.01f,%02dkm,%02.01f%m/s,%02.01f%m/s,%03ddeg,%02.02fmm,%02.02fmm,id:%03d,%s,%s,%s",\
          dayShortStr(weekday(t)),\
          hour(t),\
          minute(t),\
          hour(OWOC.current.sunrise),\
          minute(OWOC.current.sunrise),\
          hour(OWOC.current.sunset),\
          minute(OWOC.current.sunset),\
          OWOC.current.temp,\
          OWOC.current.feels_like,\
          OWOC.current.pressure,\
          OWOC.current.humidity,\
          OWOC.current.dew_point,\
          OWOC.current.clouds,\
          OWOC.current.uvi,\
          OWOC.current.visibility/1000,\
          OWOC.current.wind_speed,\
          OWOC.current.wind_gust,\
          OWOC.current.wind_deg,\
          OWOC.current.rain_1h,\
          OWOC.current.snow_1h,
          OWOC.current.id,\
          OWOC.current.main,\
          OWOC.current.description,\
          OWOC.current.icon);\
  Serial.println(Output);
  Serial.println("");
  
  Serial.println("Minutely Forecast:");
  for (int minutey = 0; minutey < (sizeof(OWOC.minutely) / sizeof(OWOC.minutely[0])); minutey++) {
    sprintf(Output,"%02d:%02d:%02.02fmm,",\
            hour(OWOC.minutely[minutey].dt),\
            minute(OWOC.minutely[minutey].dt),\
            OWOC.minutely[minutey].precipitation);
    Serial.print(Output);
  }
  Serial.println("");
  Serial.println("");
  
  Serial.println("Hourly Forecast:");
  for (int Houry = 0; Houry < (sizeof(OWOC.hourly) / sizeof(OWOC.hourly[0])); Houry++) {
    sprintf(Output,"%02d:%02d:%02.02fC,%02.02fC,%04.0fhPa,%02.01f%% Rh,%02.02fC,%03d%%,%02dkm,%02.01f%m/s,%02.01f%m/s,%03ddeg,%03.00f%%,%02.02fmm,%02.02fmm,%03d,%s,%s,%s",\
            hour(OWOC.hourly[Houry].dt),\
            minute(OWOC.hourly[Houry].dt),\
            OWOC.hourly[Houry].temp,\
            OWOC.hourly[Houry].feels_like,\
            OWOC.hourly[Houry].pressure,\
            OWOC.hourly[Houry].humidity,\
            OWOC.hourly[Houry].dew_point,\
            OWOC.hourly[Houry].clouds,\
            OWOC.hourly[Houry].visibility/1000,\
            OWOC.hourly[Houry].wind_speed,\
            OWOC.hourly[Houry].wind_gust,\
            OWOC.hourly[Houry].wind_deg,\
            OWOC.hourly[Houry].pop*100,\
            OWOC.hourly[Houry].rain_1h,\
            OWOC.hourly[Houry].snow_1h,\
            OWOC.hourly[Houry].id,\
            OWOC.hourly[Houry].main,\
            OWOC.hourly[Houry].description,\
            OWOC.hourly[Houry].icon);
    Serial.println(Output);
  }
  Serial.println("");
  
  Serial.println("7 Day Forecast:");
  for (int y = 0; y < (sizeof(OWOC.forecast) / sizeof(OWOC.forecast[0])); y++) {
    sprintf(Output,"%s:%02d:%02d-%02d:%02d,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%02.01fC,%04.0fhPa,%02.01f%% Rh,%02.01fC,%02.01f%m/s,%02.01f%m/s,%03ddeg,%03d%%,UV:%02.01f,%02dkm,%03.0f%%,%02.02fmm,%02.02fmm,%03d,%s,%s,%s",\
            dayShortStr(weekday(OWOC.forecast[y].dt)),\
            hour(OWOC.forecast[y].sunrise),\
            minute(OWOC.forecast[y].sunrise),\
            hour(OWOC.forecast[y].sunset),\
            minute(OWOC.forecast[y].sunset),\
            OWOC.forecast[y].temp_morn,\
            OWOC.forecast[y].temp_day,\
            OWOC.forecast[y].temp_eve,\
            OWOC.forecast[y].temp_night,\
            OWOC.forecast[y].temp_min,\
            OWOC.forecast[y].temp_max,\
            OWOC.forecast[y].feels_like_morn,\
            OWOC.forecast[y].feels_like_day,\
            OWOC.forecast[y].feels_like_eve,\
            OWOC.forecast[y].feels_like_night,\
            OWOC.forecast[y].pressure,\
            OWOC.forecast[y].humidity,\
            OWOC.forecast[y].dew_point,\
            OWOC.forecast[y].wind_speed,\
            OWOC.forecast[y].wind_gust,\
            OWOC.forecast[y].wind_deg,\
            OWOC.forecast[y].clouds,\
            OWOC.forecast[y].uvi,\
            OWOC.forecast[y].visibility/1000,\
            OWOC.forecast[y].pop*100,\
            OWOC.forecast[y].rain,\
            OWOC.forecast[y].snow,\
            OWOC.forecast[y].id,\
            OWOC.forecast[y].main,\
            OWOC.forecast[y].description,\
            OWOC.forecast[y].icon);\
    Serial.println(Output);
  }
}

void setup()
{
  // Begin serial and display
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Serial Monitor Initialized");

  display.begin();

  // Initial cleaning of buffer and physical screen
  display.clearDisplay();
  display.clean();


  // Welcome screen
  display.setCursor(50, 290);
  display.setTextSize(3);
  display.print(F("Welcome to Wol Inkplate 6 weather example!"));
  display.display();
  Serial.println("Welcome to Wol Inkplate 6 weather example!");
  display.display();
}

void loop()
{
  int dayPos;
  if (refreshes == 0)
  {
    GetCurrentWeather();
    refreshes = fullRefresh;
    
    // Clear display
    display.clearDisplay();
  
    sprintf(Output,"http://openweathermap.org/img/wn/%s@4x.png",OWOC.current.icon);
    Serial.print(Output);
    display.drawImage(Output, 0, 0, true, true);
    int startDay = 1;
    int numOfDays = (sizeof(OWOC.forecast) / sizeof(OWOC.forecast[0]));
    int dayPitch = E_INK_WIDTH / (numOfDays - startDay);
    for (int day = startDay; day < numOfDays; day++) {
      dayPos = (day - startDay) * dayPitch;
      sprintf(Output,"http://openweathermap.org/img/wn/%s@2x.png",OWOC.forecast[day].icon);
      Serial.print(Output);
      display.drawImage(Output, dayPos, dayOffset, true, true);
      display.setTextColor(BLACK, WHITE);
      display.setTextSize(1);
      display.setFont(&FreeSans18pt7b);
      display.setCursor(15 + dayPos, dayOffset + 10);
      sprintf(Output,"%s",dayShortStr(weekday(OWOC.forecast[day].dt)));
      display.println(Output);
      display.setFont(&FreeSans9pt7b);
      sprintf(Output,"%02.0fC / %02.0fC",\
              OWOC.forecast[day].temp_day,\
              OWOC.forecast[day].temp_night);
      display.setCursor(5 + dayPos, dayOffset + 100);
      display.println(Output);
      sprintf(Output,"%02.0f%%",\
              OWOC.forecast[day].pop*100);
      display.setCursor(30 + dayPos, dayOffset + 120);
      display.println(Output);
      sprintf(Output,"%02.01fmm",\
              OWOC.forecast[day].rain+OWOC.forecast[day].snow);
      display.setCursor(20 + dayPos, dayOffset + 140);
      display.println(Output);
      sprintf(Output,"%04.0fhPa",\
              OWOC.forecast[day].pressure);
      display.setCursor(10 + dayPos, dayOffset + 160);
      display.println(Output);
      sprintf(Output,"%03.0f%% Rh",\
              OWOC.forecast[day].humidity);
      display.setCursor(10 + dayPos, dayOffset + 180);
      display.println(Output);
    }

    drawCity();
    drawTime();
    display.display();
  } else {
    drawTime();
    display.partialUpdate();
  }
  
  // put your main code here, to run repeatedly:
  // Draw data, see functions below for info
  //  drawWeather();
  //  drawCurrent();
  //  drawTemps();

  // Refresh full screen every fullRefresh times, defined above
  //  if (refreshes % fullRefresh == 0)
  //  else

  // Go to sleep before checking again
  esp_sleep_enable_timer_wakeup(1000L * DELAY_MS);
  (void)esp_light_sleep_start();
  --refreshes;
}

// Function for drawing weather info
void drawWeather()
{
  // Searching for weather state abbreviation
  for (int i = 0; i < 10; ++i)
  {
    // If found draw specified icon
    if (strcmp(abbrs[i], currentWeatherAbbr) == 0)
      display.drawBitmap(50, 50, logos[i], 152, 152, BLACK);
  }

  // Draw weather state
  display.setTextColor(BLACK, WHITE);
  display.setFont(&Roboto_Light_36);
  display.setTextSize(1);
  display.setCursor(40, 270);
  display.println(currentWeather);
}

// Function for drawing current time
void drawTime()
{
  t = now();
  // Drawing current time
  sprintf(currentTime,"%02d:%02d",hour(t),minute(t));
  Serial.println(currentTime);
  
  display.setTextColor(BLACK, WHITE);
  display.setFont(&Roboto_Light_36);
  display.setTextSize(1);

  display.setCursor(800 - 20 * strlen(currentTime), 35);
  display.println(currentTime);
}

// Function for drawing city name
void drawCity()
{
  // Drawing city name
  display.setTextColor(BLACK, WHITE);
  display.setFont(&Roboto_Light_36);
  display.setTextSize(1);

  display.setCursor(400 - 9 * strlen(city), 570);
  display.println(city);
}

// Function for drawing temperatures
void drawTemps()
{
  // Drawing 4 black rectangles in which temperatures will be written
  int rectWidth = 150;
  int rectSpacing = (800 - rectWidth * 4) / 5;

  display.fillRect(1 * rectSpacing + 0 * rectWidth, 300, rectWidth, 220, BLACK);
  display.fillRect(2 * rectSpacing + 1 * rectWidth, 300, rectWidth, 220, BLACK);
  display.fillRect(3 * rectSpacing + 2 * rectWidth, 300, rectWidth, 220, BLACK);
  display.fillRect(4 * rectSpacing + 3 * rectWidth, 300, rectWidth, 220, BLACK);

  int textMargin = 6;

  display.setFont(&Roboto_Light_48);
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);

  display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 300 + textMargin + 40);
  display.println(days[0]);

  display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 300 + textMargin + 40);
  display.println(days[1]);

  display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 300 + textMargin + 40);
  display.println(days[2]);

  display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 300 + textMargin + 40);
  display.println(days[3]);

  //Drawing temperature values into black rectangles
  display.setFont(&Roboto_Light_48);
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);

  display.setCursor(1 * rectSpacing + 0 * rectWidth + textMargin, 300 + textMargin + 120);
  display.print(temps[0]);
  display.println(F("C"));

  display.setCursor(2 * rectSpacing + 1 * rectWidth + textMargin, 300 + textMargin + 120);
  display.print(temps[1]);
  display.println(F("C"));

  display.setCursor(3 * rectSpacing + 2 * rectWidth + textMargin, 300 + textMargin + 120);
  display.print(temps[2]);
  display.println(F("C"));

  display.setCursor(4 * rectSpacing + 3 * rectWidth + textMargin, 300 + textMargin + 120);
  display.print(temps[3]);
  display.println(F("C"));

  for (int i = 0; i < 18; ++i)
  {
    //If found draw specified icon
    if (strcmp(abbr1, abbrs[i]) == 0)
      display.drawBitmap(1 * rectSpacing + 0 * rectWidth + textMargin, 300 + textMargin + 150, s_logos[i], 48, 48, WHITE, BLACK);
  }

  for (int i = 0; i < 18; ++i)
  {
    //If found draw specified icon
    if (strcmp(abbr2, abbrs[i]) == 0)
      display.drawBitmap(2 * rectSpacing + 1 * rectWidth + textMargin, 300 + textMargin + 150, s_logos[i], 48, 48, WHITE, BLACK);
  }

  for (int i = 0; i < 18; ++i)
  {
    //If found draw specified icon
    if (strcmp(abbr3, abbrs[i]) == 0)
      display.drawBitmap(3 * rectSpacing + 2 * rectWidth + textMargin, 300 + textMargin + 150, s_logos[i], 48, 48, WHITE, BLACK);
  }

  for (int i = 0; i < 18; ++i)
  {
    //If found draw specified icon
    if (strcmp(abbr4, abbrs[i]) == 0)
      display.drawBitmap(4 * rectSpacing + 3 * rectWidth + textMargin, 300 + textMargin + 150, s_logos[i], 48, 48, WHITE, BLACK);
  }
}

// Current weather drawing function
void drawCurrent()
{
  // Drawing current information

  // Temperature:
  display.setFont(&Roboto_Light_120);
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);

  display.setCursor(245, 150);
  display.print(currentTemp);

  int x = display.getCursorX();
  int y = display.getCursorY();

  display.setFont(&Roboto_Light_48);
  display.setTextSize(1);

  display.setCursor(x, y);
  display.println(F("C"));

  // Wind:
  display.setFont(&Roboto_Light_120);
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);

  display.setCursor(480, 150);
  display.print(currentWind);

  x = display.getCursorX();
  y = display.getCursorY();

  display.setFont(&Roboto_Light_48);
  display.setTextSize(1);

  display.setCursor(x, y);
  display.println(F("m/s"));

  // Labels underneath
  display.setFont(&Roboto_Light_36);
  display.setTextSize(1);

  display.setCursor(215, 210);
  display.println(F("TEMPERATURE"));

  display.setCursor(500, 210);
  display.println(F("WIND SPEED"));
}