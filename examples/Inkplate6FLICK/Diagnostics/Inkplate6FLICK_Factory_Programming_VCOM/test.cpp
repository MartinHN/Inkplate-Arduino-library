#include "test.h"

const char sdCardTestStringLength = 100;
const char *testString = {"This is some test string..."};

const char *WSSID = {"Soldered-testingPurposes"};
const char *WPASS = {"Testing443"};

// Change this to your used slave device
const uint8_t easyCDeviceAddress = 0x30;

const int TOUCHSCREEN_TIMEOUT = 30;

void testPeripheral()
{
    // Set display for test report
    display.setTextSize(3);
    display.setTextColor(BLACK);
    display.setCursor(50, 50);
    display.println("INKPLATE CHECKLIST");

    //  Power up epaper PSU
    display.einkOn();

    // Check if epaper PSU (TPS65186 EPD PMIC) is ok.
    Wire.beginTransmission(0x48); // Send address 0x48 on I2C
    if (!(Wire.endTransmission() == 0) ||
        (display.readPowerGood() != PWR_GOOD_OK)) // Check if there was an error in communication
    {
        Serial.println("- TPS Fail!");
        failHandler();
    }
    display.println("- TPS65186: OK");
    display.partialUpdate(0, 1);

    // Check I/O expander internal
    display.printf("- I/O Expander Internal:");
    display.partialUpdate(0, 1);

    // Try to communicate with I/O expander
    Wire.beginTransmission(IO_INT_ADDR);
    if (Wire.endTransmission() ==
        0) // Check if there was an error in communication and print out the results on display.
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }

    display.printf("- I/O Expander External:");
    display.partialUpdate(0, 1);

    // Try to communicate with I/O expander
    Wire.beginTransmission(IO_EXT_ADDR);
    if (Wire.endTransmission() ==
        0) // Check if there was an error in communication and print out the results on display.
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }


    // Check touch screen and frontlight
    // Check frontlight (just a visual check). Set frontlight to max.
    display.frontlight(true);  // Enable frontlight circuit
    display.setFrontlight(63); // Set frontlight intensity to the max.
    display.println("- Frontlight test (visual check)");
    display.partialUpdate(0, 1);
    delay(1000);

    // Check the touchscreen (init and touch)
    display.print("- Touchscreen init: ");
    display.partialUpdate(0, 1);
    if (checkTouch(TOUCHSCREEN_TIMEOUT))
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }


    // Check the micro SD card slot
    display.print("- microSD card slot: ");
    display.partialUpdate(0, 1);
    if (checkMicroSDCard())
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }

    // Check the WiFi
    display.print("- WiFi: ");
    display.partialUpdate(0, 1);
    if (checkWiFi(WSSID, WPASS, WTIMEOUT))
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }

    // Check the RTC
    display.print("- PCF85063 RTC: ");
    if (rtcCheck())
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }


    // Check I2C (easyc)
    // A slave must be connected via easyC address (0x30)
    display.print("- I2C (easyC): ");
    display.partialUpdate(0, 1);
    if (checkI2C(easyCDeviceAddress))
    {
        display.println("OK");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }

    float batteryVoltage = 0;
    float temperature = 0;

    // Check battery and temp
    display.print("- Battery and temperature: ");
    display.partialUpdate(0, 1);
    if (checkBatteryAndTemp(&temperature, &batteryVoltage))
    {
        display.println("OK");
        display.print("- Battery voltage: ");
        display.print(batteryVoltage);
        display.println("V");
        display.print("- Temperature: ");
        display.print(temperature);
        display.println("c");
        display.partialUpdate(0, 1);
    }
    else
    {
        display.println("FAIL");
        failHandler();
    }

    // Text wake up button
    // Disable touchscreen to avoid accidental TS INT trigger that would trigger WAKE button.
    // (since Touch INT and WAKE BTN share same line).
    display.tsInit(true);
    // Wait a little bit.
    delay(100);
    
    long beginWakeUpTest = millis();
    int wakeButtonState = digitalRead(GPIO_NUM_36);

    display.println("Press WAKEUP button within 30 seconds to finish testing");
    display.partialUpdate(0, 1);

    while (true)
    {
        long now = millis();
        if (now - beginWakeUpTest > 30000)
        {
            display.println("WAKEUP not pressed for 30 seconds!");
            display.partialUpdate(0, 1);
            failHandler();
        }

        if (digitalRead(GPIO_NUM_36) != wakeButtonState)
        {
            break;
        }
        delay(1);
    }

    display.println("WAKEUP button pressed!");
    display.partialUpdate(0, 1);

}

int checkWiFi(const char *_ssid, const char *_pass, uint8_t _wifiTimeout)
{
    unsigned long _timeout = millis();

    // Try to connect to WiFi network
    WiFi.begin(WSSID, WPASS);

    // Wait until WiFi connection is established or timeout has occured.
    while ((WiFi.status() != WL_CONNECTED) && ((unsigned long)(millis() - _timeout) < (_wifiTimeout * 1000UL)))
        ;

    // Check if board is connected to WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
        return 1;
    }
    else
    {
        return 0;
    }

    // Something is wrong if you got there.
    return 0;
}

int checkMicroSDCard()
{
    int sdInitOk = 0;
    sdInitOk = display.sdCardInit();

    if (sdInitOk)
    {
        File file;

        if (file.open("/testFile.txt", O_CREAT | O_RDWR))
        {
            file.print(testString);
            file.close();
        }
        else
        {
            return 0;
        }

        delay(250);

        if (file.open("/testFile.txt", O_RDWR))
        {
            char sdCardString[sdCardTestStringLength];
            file.read(sdCardString, sizeof(sdCardString));
            sdCardString[file.fileSize()] = 0;
            int stringCompare = strcmp(testString, sdCardString);
            file.remove();
            file.close();
            if (stringCompare != 0)
                return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

int checkI2C(int address)
{
    Wire.setTimeOut(3000);
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int checkBatteryAndTemp(float *temp, float *batVoltage)
{
    int temperature;
    float voltage;
    int result = 1;

    temperature = display.readTemperature();
    voltage = display.readBattery();
    *temp = temperature;
    *batVoltage = voltage;

    // Check the temperature sensor of the TPS65186.
    // If the result is -10 or +85, something is wrong.
    if (temperature <= -10 || temperature >= 85)
    {
        result = 0;
    }

    // Check the battery voltage.
    // If the measured voltage is below 2.8V and above 4.6V, charger is dead.
    if (voltage <= 2.8 || voltage >= 4.6)
    {
        result = 0;
    }

    return result;
}

int checkTemp(float *temp)
{
    int temperature;
    int result = 1;

    temperature = display.readTemperature();
    *temp = temperature;

    // Check the temperature sensor of the TPS65186.
    // If the result is -10 or +85, something is wrong.
    if (temperature <= -10 || temperature >= 85)
    {
        result = 0;
    }

    return result;
}

int rtcCheck()
{
    // First "ping" RTC on the I2C protocol and reset the RTC
    Wire.beginTransmission(0x51);
    int _res = Wire.endTransmission();

    // If result is from I2C is anything else than success (_res = 0), return 0 (error).
    if (_res != 0)
        return 0;

    // Reset and re-init RTC.
    display.rtcReset();

    // Set some time in epoch in RTC.
    uint32_t _epoch = 1640995200ULL;
    display.rtcSetEpoch(_epoch);

    // Wait at least one and a half second
    delay(1500);

    // Read the epoch (if everything is ok, epoch from RTC should not be the same)
    if (display.rtcGetEpoch() != _epoch)
    {
        return 1;
    }
    else
    {
        return 0;
    }

    return 0;
}

int checkTouch(uint8_t _tsTimeout)
{
    unsigned long _timeout;

    // First try to init touchscreen controller.
    if (!display.tsInit(true))
    {
        return 0;
    }

    // Now wait for the touch
    display.print("OK\r\n- Touch the corner within 30 seconds: ");
    display.drawRect(900, 0, 124, 124, BLACK);
    display.partialUpdate(0, 1);
    _timeout = millis();

    // Wait 10 seconds to detect touch in specified area, otherwise return 0 (error).
    while (((unsigned long)(millis() - _timeout)) < (_tsTimeout * 1000UL))
    {
        if (display.tsAvailable())
        {
            uint8_t n;
            uint16_t x[2], y[2];
            // See how many fingers are detected (max 2) and copy x and y position of each finger on touchscreen
            n = display.tsGetData(x, y);

            if ((x[0] > 900) && (x[0] < 1024) && (y[0] > 0) && (y[0] < 124))
                return 1;
        }
    }
    return 0;
}


// Show a message and stop the code from executing.
void failHandler(bool printErrorOnSerial)
{
    if (printErrorOnSerial)
    {
        Serial.println(" -> Test stopped!");
    }
    else
    {
        display.print(" -> Test stopped!");
        display.partialUpdate(0, 1);
    }


    // Inf. loop... halt the program!
    while (true)
        delay(1000);
}
