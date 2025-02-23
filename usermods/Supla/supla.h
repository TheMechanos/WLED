#pragma once

#include "wled.h"
/*
#include "../lib/SuplaDevice/src/supla/network/esp_wifi.h"
#include "../lib/SuplaDevice/src/SuplaDevice.h"
*/

#include <SuplaDevice.h>
#include <supla/control/rgb_base.h>
#include <supla/network/esp_wifi.h>

Supla::ESPWifi wifi("", "");

bool disableWLEDSync=false;
bool disableSuplSync=false;

byte lastRed;
byte lastGreen;
byte lastBlue;
byte lastColorBrightness;

class SuplaUsermodRGB : public Supla::Control::RGBBase {
  public:
    virtual void setRGBWValueOnDevice(uint32_t red, uint32_t green, uint32_t blue, uint32_t colorBrightness, uint32_t brightness){

      if(disableSuplSync){
        disableSuplSync = false;
        return;
      }

      bri = colorBrightness / 4;
      col[0] = red / 4;
      col[1] = green / 4;
      col[2] = blue / 4;

      effectCurrent = 0;

      colorUpdated(CALL_MODE_BUTTON);
      updateInterfaces(CALL_MODE_BUTTON);

      disableWLEDSync = true;
    }
/*
    void setRGBWnoRef(uint32_t red, uint32_t green, uint32_t blue, uint32_t colorBrightness){
      setRGBW(red, green, blue, colorBrightness, 0);

    }
*/
};

class SuplaUsermod : public Usermod {
  private:
    //Private class members. You can declare variables and functions only accessible to your usermod here
    unsigned long lastTime = 0;

    // set your config variables to their boot default value (this can also be done in readFromConfig() or a constructor if you prefer)
    String suplaSvr;
    String suplaMail;



  public:
    //Functions called by WLED
    SuplaUsermodRGB* rgbw;



    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() {

        wifi.setSsid(clientSSID);
        wifi.setPassword(clientPass);

        rgbw = new SuplaUsermodRGB();

        rgbw->setFadeEffectTime(0);
        

        char GUID[SUPLA_GUID_SIZE] = {0x18,0x20,0xEE,0x72,0x03,0x9B,0xDE,0xAB,0x86,0xF9,0x23,0xB8,0x53,0x1A,0xAD,0xFA};
        char AUTHKEY[SUPLA_AUTHKEY_SIZE] = {0x3E,0xCC,0x88,0x74,0xCB,0xAC,0x5E,0x5E,0x32,0x37,0x31,0x00,0xAA,0x99,0x21,0x5F};
        SuplaDevice.setName("Test WLED");
        SuplaDevice.setGUID(GUID);
        SuplaDevice.setAuthKey(AUTHKEY);
        SuplaDevice.begin();

    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() {
        wifi.setSsid(clientSSID);
        wifi.setPassword(clientPass);
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() {
      if (millis() - lastTime > 10) {

        lastTime = millis();

        if(lastRed != col[0] || lastGreen != col[1] || lastBlue != col[2] || lastColorBrightness != bri){
          lastRed = col[0];
          lastGreen = col[1];
          lastBlue = col[2];
          lastColorBrightness = bri;

          if(disableWLEDSync){
            disableWLEDSync=false;
            return;
          }


          disableSuplSync=true;
          rgbw->setRGBW(lastRed, lastGreen, lastBlue, (bri*100)/255, 0);
        }
      }

      
      
      

      SuplaDevice.iterate();
    }


    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    /*
    void addToJsonInfo(JsonObject& root)
    {
      int reading = 20;
      //this code adds "u":{"Light":[20," lux"]} to the info object
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");
      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(reading); //value
      lightArr.add(" lux"); //unit
    }
    */


    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
      //root["user0"] = userVar0;
    }


    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
      //suplaSvr = root["Server"] | suplaSvr;
      //suplaMail = root["E-mail"] | suplaMail;
    }


    /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("Supla sync");
      top["Server"] = suplaSvr;
      top["E-mail"] = suplaMail;
      /*top["testInt"] = testInt;
      top["testLong"] = testLong;
      top["testULong"] = testULong;
      top["testFloat"] = testFloat;
      top["testString"] = testString;
      JsonArray pinArray = top.createNestedArray("pin");
      pinArray.add(testPins[0]);
      pinArray.add(testPins[1]); */
    }


    /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
    bool readFromConfig(JsonObject& root)
    {
      // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
      // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

      JsonObject top = root["Supla sync"];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["Server"], suplaSvr);
      configComplete &= getJsonValue(top["E-mail"], suplaMail);

        unsigned char svr[100];
        unsigned char mail[100];
        suplaSvr.getBytes(svr,100);
        suplaMail.getBytes(mail, 100);

        SuplaDevice.setEmail((const char*)mail);
        SuplaDevice.setServer((const char*)svr);

Serial.println("Save conf");

      /*configComplete &= getJsonValue(top["testULong"], testULong);
      configComplete &= getJsonValue(top["testFloat"], testFloat);
      configComplete &= getJsonValue(top["testString"], testString);

      // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
      configComplete &= getJsonValue(top["testInt"], testInt, 42);  
      configComplete &= getJsonValue(top["testLong"], testLong, -42424242);
      configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
      configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);*/

      return configComplete;
    }

   
    /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
    uint16_t getId()
    {
      return USERMOD_ID_EXAMPLE;
    }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};