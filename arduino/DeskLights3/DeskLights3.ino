#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_WS2801.h>

//TODO: error on exceeding x,y, or i params
//TODO: Add some sort of security/auth

/*
These are the settings you will likely have to change to match your environment
*/

// WiFi Credentials (set for your environment)
const char ssid[] = "your_ssid";
const char password[] = "your_wifi_password";

// led pixel strip stuff
uint8_t dataPin = 12; // USUALLY yellow on led strips
uint8_t clockPin = 13; // USUALLY green on led strip
#define pixel_count 25
#define x_max 4
#define y_max 4
Adafruit_WS2801 strip = Adafruit_WS2801(x_max+1, y_max+1, dataPin, clockPin, WS2801_GRB);
int defaultPattern = 0;
int debug = 1;
/*
It's unlikely you'll need to change anything below here for basic use.
*/

// init web server globally
ESP8266WebServer server(80);

// web server route handlers
void route_root() {
    String message = "<html><body>\n";
    message += "<a href=\"/off\">off</a><br>\n";
    message += "<a href=\"/getInfo\">getInfo</a><br>\n";
    message += "<a href=\"/setDebug?i=1\">enable debug</a><br>\n";
    message += "<a href=\"/setDebug?i=0\">disable debug</a><br>\n";
    message += "<a href=\"/show\">show</a><br>\n";
    message += "<a href=\"/test\">test</a><br>\n";
    message += "<a href=\"/wipe?h=ff0000&d=250\">wipe red</a><br>\n";
    message += "<a href=\"/wipe?h=00ff00&d=250\">wipe green</a><br>\n";
    message += "<a href=\"/wipe?h=0000ff&d=250\">wipe blue</a><br>\n";
    message += "<a href=\"/wipe?h=ffffff&d=250\">wipe white</a><br>\n";
    message += "<a href=\"/alert?h=ff0000&d=1000\">alert red</a><br>\n";
    message += "<a href=\"/alert?h=00ff00&d=1000\">alert green</a><br>\n";
    message += "<a href=\"/alert?h=0000ff&d=1000\">alert blue</a><br>\n";
    message += "<a href=\"/alert?h=ffffff&d=1000\">alert white</a><br>\n";
    message += "<a href=\"/setDefault?i=1\">setDefault=1</a><br>\n";
    message += "<a href=\"/setDefault?i=2\">setDefault=2</a><br>\n";
    message += "</body></html>\n";
  server.send(200, "text/html", message);
}

void route_NotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void route_setDebug() {
    debug = server.arg("i").toInt();
    server.send(200, "text/plain", "ok\n");
}

void route_getFrame() {
  String message = "";

  for(int i=0; i < strip.numPixels(); i++) {
    uint32_t c = strip.getPixelColor(i);
    message += String(c);
    if (i < pixel_count-1) {
        message += ',';
    }
    message += "\n";
  }

  server.send(200, "text/plain", message);
}

// set all pixels to specified colors in one shot
void route_setFrame() {
    String values = server.arg("h");
    int id=0;

    while(values.length() && id < pixel_count) {
        int stop = values.indexOf(',');
        if (stop < 0) {
            stop = values.length();
        }
        String value = values.substring(0, stop);
        values.remove(0, stop+1);
        log("Setting pixel to: " + value);

        if (server.hasArg("h")) {
          strip.setPixelColor(id, strtoul(value.c_str(), NULL, 16));
      } else if (server.hasArg("c")) {
          strip.setPixelColor(id, value.toInt());
        }
        id++;
    }

  if (!(server.arg("s").equals("0"))) {
    strip.show();
  }

  server.send(200, "text/plain", "ok\n");
}

void route_getInfo() {
  String message = "{\n";
  message += "\"pixel_count\" : " + String(strip.numPixels()) + ",\n";
  message += "\"x_max\" : " + String(x_max) + ",\n";
  message += "\"y_max\" : " + String(y_max) + ",\n";
  message += "\"debug\" : " + String(debug) + "\n";
  message += "}\n";
  server.send(200, "application/json", message);
}

void route_setDefault() {
  defaultPattern = server.arg("i").toInt();
  server.send(200, "text/plain", "ok\n");
}

void route_off() {
  setAll(0);
  strip.show();
  defaultPattern = 0;
  server.send(200, "text/plain", "ok\n");
}

void route_setPixel() {
    uint32_t color = paramColor();

  if (server.hasArg("i")) {
      strip.setPixelColor(server.arg("i").toInt(), color);
  } else if (server.hasArg("x") && server.hasArg("y")) {
      strip.setPixelColor(server.arg("x").toInt(), server.arg("y").toInt(), color);
  } else {
      server.send(400, "text/plain", "Invalid Params\n");
      return;
  }

  if (!(server.arg("s").equals("0"))) {
    strip.show();
  }

  server.send(200, "text/plain", "ok\n");
}

void route_show() {
  strip.show();
  server.send(200, "text/plain", "ok\n");
}

void route_alert() {
  uint32_t frame[pixel_count];
  getFrame(frame);
  setAll(paramColor());
  strip.show();
  delay(server.arg("d").toInt());
  setFrame(frame, pixel_count);
  strip.show();
  server.send(200, "text/plain", "ok\n");
}

void route_setAll() {
  setAll(paramColor());
  if (!(server.arg("s").equals("0"))) {
    strip.show();
  }
  server.send(200, "text/plain", "ok\n");
}

void route_wipe() {
  colorWipe(paramColor(), server.arg("d").toInt());
  server.send(200, "text/plain", "ok\n");
}

void route_test() {
  int wait = server.arg("d").toInt();
  lightTest(wait);
  gridTest(wait);
  server.send(200, "text/plain", "ok\n");
}


/*
helper functions
*/

void log(String input) {
    if (debug) {
        Serial.println(input);
    }
}

// random pixel, random color (short pattern, very responsive)
void p_random (int wait) {
  strip.setPixelColor(
    random(0, strip.numPixels()),
    random(0, 0xffffff)
  );
  strip.show();
  delay(wait);
}

// This takes about a second to run, so new requests will wait
void p_rainbow() {
  int i, j;
  for (j=0; j < 256; j++) {
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }
    strip.show();
  }
}

// show the grid to verify
void gridTest(int wait) {
  int x;
  int y;
  uint32_t on = 0xffffff;
  uint32_t off = 0;

  for ( x = 0; x <= x_max; x++) {
    for ( y = 0; y <= y_max; y++) {
      strip.setPixelColor(x,y, on);
      strip.show();
      delay(wait);
      strip.setPixelColor(x,y, off);
      strip.show();
    }
  }
}

// wipe the major colors through all pixels
void lightTest(int wait) {
  colorWipe(0xff0000, wait);
  colorWipe(0x00ff00, wait);
  colorWipe(0x0000ff, wait);
  colorWipe(0xffffff, wait);
  colorWipe(0x000000, wait);
}

// set all pixels to a "Color" value
void setAll(uint32_t c) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
}

// set all pixels to a "Color" value, one at a time, with a delay
void colorWipe(uint32_t c, uint8_t wait) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// store the entire strip state in an array
void getFrame(uint32_t *frame) {
    for (int i=0; i < strip.numPixels(); i++) {
        frame[i] = strip.getPixelColor(i);
    }
}

// set the entire strip state from an array
void setFrame(uint32_t *frame, int size) {
    for (int i=0; i < size; i++) {
        strip.setPixelColor(i, frame[i]);
    }
}

// get color from either h or c param
uint32_t paramColor() {
    if (server.hasArg("c")) {
        return(server.arg("c").toInt());
    } else if (server.hasArg("h")) {
        return(strtoul(server.arg("h").c_str(), NULL, 16));
    }
    return(0);
}

// Create a 24 bit color value from R,G,B (0...255 each)
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/*
standard arduino setup and loop functions
*/

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  log("Attempting to connect to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  log("\nConnected to:");
  log(ssid);
  log("IP address: ");
  Serial.println(WiFi.localIP());

  // Set route handlers
  server.onNotFound(route_NotFound);
  server.on("/", route_root);
  server.on("/getInfo", route_getInfo);
  server.on("/getFrame", route_getFrame);
  server.on("/setFrame", route_setFrame);
  server.on("/setDefault", route_setDefault);
  server.on("/setAll", route_setAll);
  server.on("/setPixel", route_setPixel);
  server.on("/off", route_off);
  server.on("/wipe", route_wipe);
  server.on("/test", route_test);
  server.on("/alert", route_alert);
  server.on("/show", route_show);
  server.on("/setDebug", route_setDebug);

  server.begin();
  log("HTTP server started");

  strip.begin();
  strip.show();

  // quick dim flash to show we are ready
  setAll(0x555555);
  setAll(0);
}

void loop(void) {
  server.handleClient();

  // run the default pattern
  switch(defaultPattern) {
    case 1:
      p_rainbow();
      break;
    case 2:
      p_random(1000);
      break;
  }
}
