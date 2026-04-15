# Automated Bell System - Hardware & RTC Module

# Hardware Wiring Guide

## Pin Connections
| Component | ESP32 Pin |
|-----------|-----------|
| RTC SDA   | 21        |
| RTC SCL   | 22        |
| Buzzer    | 25        |
| Button    | 26        |

## RTC Setup
1. Connect DS3231 as above
2. Install CR2032 battery
3. Run test_rtc.ino to verify
