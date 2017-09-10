/*
    LEDClock with WS8212 60 LED Ring

    MAWISoft 2017

    Thanks to Daniel Garcia for FastLED
    and to Fabrice Weinberg for his 'NTPClient'
    and the whole team of arduino.cc
    and some inspiration by wol clock from Jon Fuge.

    This Program ist published under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.
    It is distrubited in the hope that it will be useful,
    but without any warranty,
    see <http://www.gnu.org/licenses/>.

*/

//#define DEBUG           // uncomment if necessary

#include "clock.h"


void setup() {

  // start serial if in debug mode
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial);                                 // wait for serial attach
  Serial.println("Configuring WiFi,RTC, NTP and WS8212 ...");
#endif

  // start WiFi Server
  IPAddress wifi_ip;

#ifdef WIFIMODE_AP
  WiFi.softAPConfig(ip, ip, netmask);       // configure ip address for softAP
  if (WiFi.softAP(ssid, pwd)) wifi_ip = WiFi.softAPIP();
#endif

#ifdef WIFIMODE_STA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pw);
  while (WiFi.status() != WL_CONNECTED) delay(100);
  wifi_ip = WiFi.localIP();
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);        // this works only in station mode
#endif

  // start TCP server
  server.begin();   // start TCP server

#ifdef DEBUG
  Serial.println("TCP Server started.");
  Serial.print("Connect to ");
  Serial.println(ssid);
  Serial.println(wifi_ip);
  Serial.println(port);
#endif

  // start NTP client
  ntpClient.begin();

  // start I2C
  Wire.begin(SDA, SCL);

  // start RTC
  if (!Rtc.begin()) {
#ifdef DEBUG
    Serial.println("Error: Could not find RTC!");
#endif
  }

  if (Rtc.lostPower()) {
#ifdef DEBUG
    Serial.println("RTC lost power, call NTP ...");
#endif
    if (ntpClient.update()) Rtc.adjust(ntpClient.getEpochTime());
  }

  // Setup FastLED Lib
  FastLED.addLeds<LED_TYPE, DATA, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLT, MAX_MILLIAMP);

  // set PIR pin
  pinMode(PIR, INPUT_PULLUP);

  // show some fancy
  rainbowCycle(8);
  
  // wait 2 seconds to settle
  delay(2000);

  // set colour scheme
  defineColorScheme(scheme);

  blackWipe(5);

#ifdef DEBUG
  Serial.println("Setup ready. Start loop ...");
#endif

  delay(1000);
}


void loop() {

  // ask for WiFi TCP Input
  if (!client) client = server.available(); // if no client present connect to server
  else {                                    // if client present
#ifdef DEBUG
    Serial.println("Client.connected");
#endif
    yield();
    while (client.available()) {             // and input available,
      char c1 = (char)client.read();          // read chars from client
      char c2 = (char)client.read();
#ifdef DEBUG
      Serial.printf("Client.read %c %c", (char)c1, (char)c2);
#endif
      if (c2 == '\n' || c2 == '!') {                       // if second char = newline or !, i.e. end of TCP command
        switch (c1) {                                       // process user input

          case 'x':
#ifdef DEBUG
            Serial.printf("Download %s ... \n", updateServer);
#endif
            colorWipe(rgbDivisions, 3);                        // give the user some feedback
            blackWipe(3);

            if (ESPhttpUpdate.update(updateServer) == HTTP_UPDATE_FAILED) {
#ifdef DEBUG                
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
#endif        
 
                colorWipe(RGB(200,0,0), 5);                        // give the user some feedback in red
                blackWipe(3);                                      // update Error
            }
            break;
          case 'u':
            char strColor[48];
            for (byte i = 0; i < 48; i++) {
              char col = (char)client.read();
              if (col == -1) strColor[i] = '0';
              else strColor[i] = col;
            }
            if (setCustomColors(strColor)) {

            }
            scheme = USER;
            defineColorScheme(scheme);
            break;
          case 'n':
            if (ntpClient.update()) {                             // according to NTP server
              Rtc.adjust(ntpClient.getEpochTime());
#ifdef DEBUG
              Serial.println("Clock updated through NTP.");
#endif
              colorWipe(RGB(0,200,0), 1);                        // give the user some feedback in green
              blackWipe(1);
            }            
            break;
          case 'i':
            SHUTOFF
            state = INSTANTOFF;
#ifdef DEBUG
            Serial.println("Clock instant off.");
#endif
            break;
          case 'o':
            if(state != ON) state = ON;
            else SHUTOFF
            break;            
          case 'c':
            mode = CLASSIC;
            break;
          case 'm':
            mode = MINARC;
            break;
          case 'a':
            mode = ARC;
            break;
          case 'r':
            if (scheme < USER) scheme++;
            else scheme = DAY;
            defineColorScheme(scheme);
            break;
          default:
            mode = CLASSIC;
            scheme = DAY;
            break;
            yield();
        }

        colorWipe(rgbDivisions, 3);                        // give the user some feedback
        blackWipe(3);


      }
    }
      
    client.write("OK\n");
    client.stop();                                     // close client and leave while loop
#ifdef DEBUG
    Serial.printf("Client closed. Mode: %i Scheme: %i\n", (int)mode, (int)scheme);
#endif
  
  }


  // get time from RTC
  DateTime now = Rtc.now();

  byte s = now.second();  	      // get seconds
  byte m = now.minute();  	      // get minutes
  int h = now.hour();    	        // get hours
  int y = now.year();     	      // get year
  byte mo = now.month();  	      // get month
  byte d = now.day();     	      // get day
  byte dow = now.dayOfTheWeek(); 	// get day of week

  // Calculate Hour
  if (checkDst(d, mo, dow)) h++;  // Add 1 hour due to detected DST
  if (h == 24) h = 0;             // show 24h as 0h
  if (h > 11) h -= 12;            // this clock shows only 12 hours

#ifdef DEBUG
  // monitor DateTime
  Serial.print(days[dow]);
  Serial.print(", ");
  Serial.print(d, DEC);
  Serial.print(". ");
  Serial.print(months[mo]);
  Serial.print(" ");
  Serial.print(y);
  Serial.println();
  Serial.print(h);
  Serial.print(" Uhr ");
  Serial.print(m, DEC);
  Serial.print(" und ");
  Serial.print(s, DEC);
  Serial.println(" Sekunden");
  Serial.println();
  yield();
#endif

  int h_m = (int)(h * 5 + m / 12);                        // transform hours to minutes to show hour hand on clockface

  if (d == 8 && h_m == 8 && m == 8 && s == 8) {           // adjust RTC time once a month
      if (ntpClient.update()) {                           // according to NTP server
          Rtc.adjust(ntpClient.getEpochTime());
#ifdef DEBUG
          Serial.println("Clock updated through NTP.");
#endif
      }
  }

  // check for presence of person
  if (digitalRead(PIR) == HIGH) {                          // detect person
    m_pir = m_delta;
#ifdef DEBUG
    Serial.println("Person detected.");
#endif
  }

  if (s == 20) {
    if (m_pir > 0) {
      if(state != INSTANTOFF) state = ON;
      m_pir--;                                            // decrease minute counter
    }
    else {                                                // save electricity
      if (state == ON) SHUTOFF
      }
  }

#ifdef DEBUG
  Serial.printf("Minutes until shutoff: %i \n", m_pir);
#endif


  if (state == ON) {                   // show clockface only if state == ON

    if (s == 40) {                // adjust brightness once every minute
      FastLED.setBrightness(adjustBrightness());
    }

    if (m == 0 && s == 0)  {      // fancy wipe every full hour
      if (mode == CLASSIC) colorWipe(rgbMinute, 5);
      blackWipe(3);
    }

    // draw clock according to choice
    if (mode == CLASSIC) drawClock_classic (h_m, m , s);
    if (mode == MINARC) drawClock_minarc (h_m, m , s);
    if (mode == ARC) drawClock_arc (h_m, m, s);

    yield();                       // let the ESP do his stuff and then
    FastLED.show();                // show all
  }

  delay(990);
}




