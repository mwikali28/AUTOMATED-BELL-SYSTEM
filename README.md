
Automated School Bell System

ESP32-based automatic bell system for schools with RTC, display, and manual button control.

Quick Hardware Connections
ESP32	Component	Pin
GPIO5	ST7735	CS
GPIO16	ST7735	RST
GPIO17	ST7735	DC
GPIO23	ST7735	MOSI
GPIO18	ST7735	SCK
GPIO21	DS3231M	SDA
GPIO22	DS3231M	SCL
GPIO25	Buzzer	(+)
GPIO26	Button	One leg
3.3V	Display/RTC	VCC
GND	All	GND

FEATURES

    ⏰ Automatic bell at scheduled times

    🔘 Manual override button

    📟 ST7735 color display (160x80)

    🕒 RTC DS3231M with battery backup

    🔔 Two ring durations (2 sec / 12 sec)

Required Libraries

Install via Arduino Library Manager:

    RTClib by Adafruit

    Adafruit GFX Library

    Adafruit ST7735 Library

CCONFIGURING

Edit this array in the code:

     cpp

     BellEvent schedule[] = {
    {8, 30,  "Warning", false},   // Hour, Minute, Name, LongRing?
    {8, 35,  "Start",   false},
    {10, 15, "Recess",  true},    // true = 12 sec, false = 2 sec
    {12, 0,  "Lunch",   true},
    {15, 0,  "Closing", true}
    };

SETTING RTC TIME

Uncomment this line in setup(), upload once, then re-comment:
   cpp

   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

HOW TO USE

Normal operation-Bell rings automatically at scheduled times
Press button (idle)	-Manual ring for 2 seconds
Press button (ringing)-	Stops bell immediately




TROUBLESHOOTING

No display-Check INITR_MINI160x80 in tft.initR()
Wrong time-Replace RTC battery (CR2032)
RTC error	-Add 4.7kΩ pull-ups on SDA/SCL
Button not working	-Check GND connection
No buzzer-Check polarity (+ to GPIO25)



CUSTOMIZATION

-Change ring duration (in ringBell() function):
     cpp

     ringDuration = longRing ? 12000 : 2000;  // 12sec / 2sec

-Change display rotation (in setup()):
    cpp

    tft.setRotation(1);  // 0-3 for different orientations



