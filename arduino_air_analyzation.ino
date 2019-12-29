#include <Arduino.h>
#include "User_Setup.h"
#include <LiquidCrystal.h>

/* Task manager */
unsigned long lastSecond = 0;
unsigned long lastMin = 0;
unsigned long last10Min = 0;
unsigned long lastHour = 0;
unsigned long last4Hours = 0;

/* Consts */
#define TIME_SECOND 1000
#define TIME_MIN 60000
#define TIME_10MINS 600000
#define TIME_HOUR 3600000
#define TIME_4HOURS 14400000

/* Settings */
boolean settingsSound = true;
int settingsMaxGas = 32;
int settingsMinGas = 15;
int settingsMaxTmp = 27;
int settingsMinTmp = 18;

/* Buttons */
#define PIN_ANALOG_KEY A7
unsigned long timestampButtonDown = 0;
boolean pressedButton = false;

/* LCD Screen */
#define LCD_RS 4
#define LCD_EN 5
#define LCD_D4 9
#define LCD_D5 10
#define LCD_D6 11
#define LCD_D7 12
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
byte specialCharPercent1[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
};
byte specialCharPercent2[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
};

byte specialCharPercent3[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
};
byte specialCharPercent4[8] = {
        B00000,
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111,
};
byte specialCharPercent5[8] = {
        B00000,
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
};
byte specialCharPercent6[8] = {
        B00000,
        B00000,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
};
byte specialCharPercent7[8] = {
        B00000,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
};
byte specialCharPercent8[8] = {
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
        B11111,
};
int currentScreen = 1;
boolean isSettingsScreen = false;

/* GAS */
#define PIN_ANALOG_GAS A0 //подключение аналогового сигналоьного пина
#define PIN_DIGITAL_GAS 8 //подключение цифрового сигнального пина
boolean panicMode = false;
float gasValue = 0;
float historyPerMin[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float historyPer10Mins[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float historyPerHour[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float historyPer4Hours[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


/***************************************
 ************** Helpers ****************
 ***************************************/

int getSpecialCharForGas(float &value) {
    return constrain(map((int) value, settingsMinGas, settingsMaxGas, 1, 9), 1, 9);
}

float getGasPercentValue() {
    return analogRead(PIN_ANALOG_GAS) * 100.00 / 1024.00;
}

void saveDataToHistory(float value, float arrayLink[6]) {
    for (int i = 5; i >= 1; --i) {
        arrayLink[i] = arrayLink[i - 1];
    }
    arrayLink[0] = value;
}

float averageOfArray(float arrayLink[6]) {
    float result = 0.0;
    for (int i = 0; i < 6; ++i) {
        result += arrayLink[i];
    }
    return result / 6;
}

int getScreenNumber(int screen) {
    if (isSettingsScreen) {
        if (screen > 5) {
            return 1;
        }
        if (screen < 1) {
            return 5;
        }
        return screen;
    } else {
        if (screen > 4) {
            return 1;
        }
        if (screen < 1) {
            return 4;
        }
        return screen;
    }
}

void updateDataFromSensors(float &value, float arrayLink[6], const String &text) {
    lcd.setCursor(0, 0);
    lcd.write(getSpecialCharForGas(gasValue));
    lcd.print(gasValue);
    lcd.print("% ");

    for (int i = 0; i < 6; ++i) {
        int code = getSpecialCharForGas(arrayLink[i]);
        if (code == 1) {
            lcd.print(" ");
        } else {
            lcd.write(code);
        }
    }

    lcd.print(text);
}

//void printArray(float arrayLink[6]){
//  Serial.print("[");
//  for (int i = 0; i < 6; ++i) {
//    Serial.print(arrayLink[i]); Serial.print(",  ");
//  }
//  Serial.println("]");
//}

/////// Screens
void Screen1() {
    updateDataFromSensors(gasValue, historyPerMin, "6m");
}

void Screen2() {
    updateDataFromSensors(gasValue, historyPer10Mins, "1h");
}

void Screen3() {
    updateDataFromSensors(gasValue, historyPerHour, "6h");
}

void Screen4() {
    updateDataFromSensors(gasValue, historyPer4Hours, "1d");
}

void ScreenSetting1() {
    lcd.setCursor(0, 0);
    lcd.write("Sound alarm:");
    lcd.setCursor(0, 1);
    lcd.write(settingsSound ? "on " : "off");
}

void ScreenSetting2() {
    lcd.setCursor(0, 0);
    lcd.write("Max gas percent:");
    lcd.setCursor(0, 1);
    lcd.print(settingsMaxGas);
    lcd.print("%    ");
}

void ScreenSetting3() {
    lcd.setCursor(0, 0);
    lcd.write("Min gas percent:");
    lcd.setCursor(0, 1);
    lcd.print(settingsMinGas);
    lcd.print("%    ");
}

void ScreenSetting4() {
    lcd.setCursor(0, 0);
    lcd.write("Max tmp percent:");
    lcd.setCursor(0, 1);
    lcd.print(settingsMaxTmp);
    lcd.print("C degrees   ");
}

void ScreenSetting5() {
    lcd.setCursor(0, 0);
    lcd.write("Min tmp percent:");
    lcd.setCursor(0, 1);
    lcd.print(settingsMinTmp);
    lcd.print("C degrees   ");
}

void updateScreen(boolean isClear) {
    if (isClear) {
        lcd.clear();
        Serial.println(isSettingsScreen);
    }

    if (isSettingsScreen) {
        switch (currentScreen) {
            case 1:
                ScreenSetting1();
                break;
            case 2:
                ScreenSetting2();
                break;
            case 3:
                ScreenSetting3();
                break;
            case 4:
                ScreenSetting4();
                break;
            case 5:
                ScreenSetting5();
                break;
        }
    } else {
        switch (currentScreen) {
            case 1:
                Screen1();
                break;
            case 2:
                Screen2();
                break;
            case 3:
                Screen3();
                break;
            case 4:
                Screen4();
                break;
        }
    }
}


/////////// Actions
void clickButtonHome() {
    currentScreen = getScreenNumber(currentScreen + 1);
    updateScreen(true);
}

void clickButtonUp() {
    if (!isSettingsScreen) {
        currentScreen = getScreenNumber(currentScreen - 1);
        updateScreen(true);
        return;
    } else {
        switch (currentScreen) {
            case 1: // Sound alarm
                settingsSound = !settingsSound;
                updateScreen(false);
                break;
            case 2: // settingsMaxGas
                settingsMaxGas = constrain(settingsMaxGas + 1, 10, 100);
                updateScreen(false);
                break;
            case 3: // settingsMinGas
                settingsMinGas = constrain(settingsMinGas + 1, 10, 100);
                updateScreen(false);
                break;
            case 4: // settingsMaxGas
                settingsMaxTmp = constrain(settingsMaxTmp + 1, 5, 40);
                updateScreen(false);
                break;
            case 5: // settingsMinGas
                settingsMinTmp = constrain(settingsMinTmp + 1, 5, 40);
                updateScreen(false);
                break;
        }
    }
}

void clickButtonDown() {
    if (!isSettingsScreen) {
        currentScreen = getScreenNumber(currentScreen + 1);
        updateScreen(true);
        return;
    } else {
        switch (currentScreen) {
            case 1: // Sound alarm
                settingsSound = !settingsSound;
                updateScreen(false);
                break;
            case 2: // settingsMaxGas
                settingsMaxGas = constrain(settingsMaxGas - 1, 10, 100);
                updateScreen(false);
                break;
            case 3: // settingsMinGas
                settingsMinGas = constrain(settingsMinGas - 1, 10, 100);
                updateScreen(false);
                break;
            case 4: // settingsMaxTmp
                settingsMaxTmp = constrain(settingsMaxTmp - 1, 5, 40);
                updateScreen(false);
                break;
            case 5: // settingsMinTmp
                settingsMinTmp = constrain(settingsMinTmp - 1, 5, 40);
                updateScreen(false);
                break;
        }
    }
}

void holdButton() {
    isSettingsScreen = !isSettingsScreen;
    currentScreen = 1;
    updateScreen(true);
}





/***************************************
 ************* Lifecycle ***************
 ***************************************/

/**
 * Activity buttons
 */
void loopButtons() {
    if (timestampButtonDown > millis()) {
        return;
    }

    int keyValue = analogRead(PIN_ANALOG_KEY);
    boolean keyPressed = keyValue > 400;

    if (pressedButton == keyPressed) {
        return;
    }
    pressedButton = keyPressed;

    if (keyPressed) {
        if (keyValue > 900) {
            // button 1
            clickButtonHome();
        } else if (keyValue > 600 && keyValue < 750) {
            // button 2
            clickButtonDown();
        } else if (keyValue > 450 && keyValue < 600) {
            // button 3
            clickButtonUp();
        }
    } else {
        if (millis() - timestampButtonDown > 1000) {
            holdButton();
        }
    }
    timestampButtonDown = millis() + 222;
}

/**
 * Loop per second
 */
void loopPerSecond(unsigned int currentValue, unsigned long &lastValue) {
    if (currentValue == lastValue) {
        return;
    }
    lastValue = currentValue;

    gasValue = getGasPercentValue();
    updateScreen(false);
}

/**
 * Loop per 1 min
 */
void loopPerMin(unsigned int currentValue, unsigned long &lastValue) {
    if (currentValue == lastValue) {
        return;
    }
    lastValue = currentValue;

    saveDataToHistory(getGasPercentValue(), historyPerMin);
}

/**
 * Loop per 10 mins
 */
void loopPer10Mins(unsigned int currentValue, unsigned long &lastValue) {
    if (currentValue == lastValue) {
        return;
    }
    lastValue = currentValue;

    saveDataToHistory(averageOfArray(historyPerMin), historyPer10Mins);
}

/**
 * Loop per 1 hour
 */
void loopPerHour(unsigned int currentValue, unsigned long &lastValue) {
    if (currentValue == lastValue) {
        return;
    }
    lastValue = currentValue;

    saveDataToHistory(averageOfArray(historyPer10Mins), historyPerHour);
}


/**
 * Loop per 4 hour
 */
void loopPer4Hours(unsigned int currentValue, unsigned long &lastValue) {
    if (currentValue == lastValue) {
        return;
    }
    lastValue = currentValue;

    saveDataToHistory(averageOfArray(historyPer10Mins), historyPer4Hours);
}

//////////////////////////////
/**********
* Loop
*/
void loop() {
    panicMode = digitalRead(PIN_DIGITAL_GAS);
    loopButtons();

    // per 1 second
    loopPerSecond(millis() / TIME_SECOND, lastSecond);

    // per 1 min
    loopPerMin(millis() / TIME_MIN, lastMin);

    // per 10 mins
    loopPer10Mins(millis() / TIME_10MINS, last10Min);

    // per 1 hour
    loopPerHour(millis() / TIME_HOUR, lastHour);

    // per 4 hours
    loopPer4Hours(millis() / TIME_4HOURS, last4Hours);
}

/**********
* Setup
*/
void setup() {
    pinMode(PIN_DIGITAL_GAS, INPUT);
    Serial.begin(9600); //инициализация Serial порта

//    lcd.createChar(1, specialCharPercent0);
    lcd.createChar(2, specialCharPercent1);
    lcd.createChar(3, specialCharPercent2);
    lcd.createChar(4, specialCharPercent3);
    lcd.createChar(5, specialCharPercent4);
    lcd.createChar(6, specialCharPercent5);
    lcd.createChar(7, specialCharPercent6);
    lcd.createChar(8, specialCharPercent7);
    lcd.createChar(9, specialCharPercent8);

    lcd.begin(16, 2);
    lcd.print("Soroka...");
    delay(3333);
    lcd.clear();
    delay(777);
}