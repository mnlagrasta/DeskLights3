# DeskLights2.2

## Overview

DeskLights is a network enabled visual alert system and an experiment in "Ambient Information". It receives instructions through a REST style API and controls the attached LED strips.

For background and hardware details, please see the more complete write up on Instructables: http://www.instructables.com/id/LED-Glass-Desk-v20/

## What's New (June 2018)

 * Controller: Due to the retiring of the Arduino Uno Ethernet and similar modules, I've updated the code to work with the ESP8266 Wifi module. The easiest way to build this project is to purchase one of the Arduino compatible micro controllers that has this built in. I have verified full compatibility with the "Feather Huzzah" from AdaFruit and the ThingDev IoT dev board from SparkFun. It should also work with similar generic modules.
 * Command Line Utility: I
 * Simplified Color Params: I've removed the R,G,B color params in favor of the more commonly used #FFFFFF hex notation. I've also added support for specifying colors as a decimal int with the "c" param.
 * New Commands: getInfo, getFrame, setFrame, setDebug, setDefault (see below)
 * getInfo Command: The desk can now report it's properties. Returns a JSON string
 * getFrame Command: Will return a comma separated list of pixel colors that can be stored and resent via setFrame
 * setFrame Command:

## API

### Parameter Explanation
All exposed function calls take 0 or more single letter parameters:
```
h: hex string representing color (000000 - ffffff)
c: color value specified as an unsigned int in decimal format
x: x coordinate
y: y coordinate
d: delay in milliseconds
i: id of pattern/grid id of pixel (grid 0,0 = 0; grid 1,0 = 1, etc)
s: call show() command automatically (0 or 1, defaults to 1)
```

### Public Functions
```
off : Turn off all pixels
getInfo : returns properties of display, including x and y grid size
setDebug : enables/disables debug output via serial monitoring
setFrame : set all pixels at once by passing in values for entire strip. This is a comma separated list of color values in decimal or hex format (see example below)
getFrame : returns all pixels in same format as setFrame and can be used to save and restore frame state
setDefault : select repeating idle pattern
setAll : set all pixels to same single color param
setPixel : set one pixel to color, pixel may be specified as int offset or x,y coordinate
wipe: like color, but down the strip; optional delay
alert: flash all pixels; takes color and optional delay
show : show any set, but not yet shown pixels

```

### Example

Here's an example using alert...

Set an entire "frame" at once using hex color values
```
http://server/setFrame?h=ffffff,000000,ff0000,00ff00,0000ff
```

Flash bright white for default length of time
```
http://server/alert?h=0000ff
```
or
```
http://server/alert?h=ffffff
```
or with a 1 second duration
```
http://server/alert?h=ffffff&d=1000
```

## Dependancies
 * You'll need a compatible micro controller and ESP8266 WiFi module.
 * Arduino dev tools http://arduino.cc
 * Adafruit WS2801 arduino library: https://github.com/adafruit/Adafruit-WS2801-Library
 * ESP8266 Wifi and Web Server libraries: https://github.com/esp8266/Arduino

## Helper Script Install

The dl3 utility is written in Perl. It has fairly minimal dependencies for the basic functions, but will require the GD library, Image::Resize Perl module, and JSON::Tiny Perl module if you want to use the image to desk functionality.
