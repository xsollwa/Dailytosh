/*DailyTosh firmware!
 *This code connects the ESP32 to WiFi, 
 *fetches weather, stock and tech news, 
 *displays it on the e-ink screen, 
 *then puts the ESP32 into sleep until the next day
 *(translated to .ino syntax from the initial dailytosh-api-test.cpp code for ESP32 compatability)*/

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <vector>
#define SECONDS_TO_SLEEP 86400 //1 day

//wifi info
const char* WIFI_SSID ="TMOBILE-B11A";
const char* WIFI_PASS = "v4strb6kf9c";


//API keys
String OPENWEATHERMAP_KEY = " "; 
String FINNHUB_KEY = " ";

String WEATHER_CITY = "Cleveland";
String WEATHER_COUNTRY = "US";

struct StockData{
  String symbol;
  float price = 0;
  float changePercent = 0;
};

struct NewsItem {
  String headline;
};


struct DailyToshData {
  int tempF = 63;
  String condition = "Clouds";
  String suggestion = "It is gloomy today. Maybe work from a cool cafe? ;)";

  std::vector<StockData> stocks;
  std::vector<NewsItem> headlines;
};

DailyToshData data;

//Waveshare e-ink display wiring
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/5, /*DC=*/22, /*RST=*/21, /*BUSY=*/4)
);

const uint16_t BG = GxEPD_WHITE;
const uint16_t FG = GxEPD_BLACK;


const uint16_t BADGE_FILL = GxEPD_BLACK;
const uint16_t BADGE_TEXT = GxEPD_WHITE;

//my art converted to binary

const int CITY_W = 64;
const int CITY_H = 64;

const char* citySprite[] = {
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000000100000000000000000000000000",
  "0000000000000000000000000000000000011111000000000000000000000000",
  "0000000000000000000000000000000001111111100000000000000000000000",
  "0000000000000000000000000000000111000000111000000000000000000000",
  "0000000000000000000000000000001100000000111100000000000000000000",
  "0000000000000000000000000000011111111111111110000000000001100111",
  "0000000000000000000000000000010000000000000010000000000000110111",
  "0000000000000000000000000000010110110110110010000000000000011100",
  "0000000000000000000000000000010110110110110010000000000000000000",
  "0000000000000000000000000000010110110110110010000000000000000000",
  "0000001111100000000000000000010110110110110010000000000000000000",
  "0000001111110000000000000000010110110110110010011000000000000000",
  "0001111111111000000000000000010110110110110010001101110000000000",
  "0001111001111000000000000000010000000000000010000111000000000000",
  "0001100000011100000000000000010101010101101010000000000000000000",
  "0011000010011111000000000000010000000000000010000000000000000000",
  "0011000010001111100000000000010101010101101010000000000000000000",
  "0011000010000011100000000000010000000000000010000000000000000000",
  "0111000010000001110000000000010000000000000010000000000000000000",
  "1100000111000000110000000000010110110110110010000000011000110000",
  "1101000111000001110000000000010110110110110010000000001111110000",
  "1100100011000001110000000000010110110110110010000000000011000000",
  "1100110011000001110000000000010110110110110010000000000000000000",
  "1100010011000001110000000000010110110110110010000000000000000000",
  "1110010011000011100000000000010110110110110010000000000000000000",
  "0110110011000011100000000000010000000000000010000000000000000000",
  "0110110010000001100000000000010000000000000010000000000000000000",
  "0100100010000001100011111111111111111111111110000000000000000000",
  "0100110010010001110010000000000001000000000010000000000000000000",
  "1100010110010000111010110011001101011111111010000000000000000000",
  "1100011110010000111010110011001101011110011010000000000000000000",
  "1100001110110000111010000000000001011111111010000000000000000000",
  "1000000110100000111010000000000001000000000010000000000000000000",
  "1000000110100001110010110011001101011111111010000000000000000000",
  "1000000111000011110010110011001101000000000010000000000000000000",
  "1111000111000011000010000000000001011111111010000000000000000000",
  "0111100111000011000010000000000001000000000010000000000000000000",
  "0001110110001110000010110011001101011111111010000000000000000000",
  "0000011111111110000010110011001101010011101010000000000000000000",
  "0000000110000000000010000000000001011111111010000011110000000000",
  "0000000110000000000010000000000001000000000010000011111000000000",
  "0000000110000000000010110011001101011111111010111110001111000000",
  "0000000110000000000010110011001101000000000011100010000001100000",
  "0000000110000000000010000000000001011111111011111111111111100000",
  "0000000110000000000010000000000001000000000010000000011000100000",
  "0000000110000000011110110011001101000011000010101010010110100000",
  "0000000110000000110010110011001101000111100010000000010000100000",
  "0000000111000001100010000000000001000111100010101010010110111000",
  "0000000111000001000010000000000001000111100010000000010000101110",
  "0001111111000011011010001111110001000111100010101010010110100011",
  "1111111111111111111111111111111111111111111111111111111111111111",
  "0000001110111000000000000000000000000000000000000000000000000000",
  "0000001011000000000000000000000000000000000000000000000000000000",
  "0001110001100000000001111111000000001100000011111111111100000000",
  "0000000000100000000000000000111111110000000000000000000000000000",
  "0000000000000000000000000000000000000000000000000000000000000000",
  "0000000000000001111100000000000000000000000000000000000000000000"
};

const int CLOUD_W = 62;
const int CLOUD_H = 29;

const char* cloudSprite[] = {
  "00000000000000000000000000001111111110000000000000000000000000",
  "00000000000000000000000001111111111111100000000000000000000000",
  "00000000000000000000000111111111111111110000000000000000000000",
  "00000000000000000000001111111111111111111000000000000000000000",
  "00000000000000000000011111111111111111111100000000000000000000",
  "00000000000000000000011111111111111111111110000000000000000000",
  "00000000000011110000111111111111111111111111000000000000000000",
  "00000000001111111101111111111111111111111111000000000000000000",
  "00000000001100001101111111111111111111111111100000000000000000",
  "00000000011000000111111111111111111111111111100000000000000000",
  "00000000011000000011111111111111111111111111110000000000000000",
  "00000011110000000011111111111111111111111111111000000000000000",
  "00000111100000000000111111111111111111111111111110000000000000",
  "00001100000000000000111111111111111111111110000111100000000000",
  "00011000000000000000000011111111111111111000000011100000000000",
  "00010000000000000000000001111111111111110000000000111000000000",
  "01111000000000000000000001111111111111110000000000011000000000",
  "11100000000000000000000011001111111111100000000000001100000000",
  "11000000000000000000000110000011111011000000000000001111100000",
  "11000000000000000000011100000001100001000000000000001000110000",
  "11100110000000000001111100000001000000000000000000000000010000",
  "01111100111111111111001000000011000000000000000000000000011000",
  "00000111111111111110000000000010000000000000000000000000001000",
  "00000000000000001110000000000011000001000000000000000100001110",
  "00000000000000001100000111100001100111100000000000001110000011",
  "00000000000000011111111111111111111111111111111111111110000001",
  "00000000000011111111111111111111111111111111111111111111000011",
  "00000000000000000000000000000000000000000000000000000001100110",
  "00000000000000000000000000000000000000000000000000000000111100"
};

const int ROBOT_W = 32;
const int ROBOT_H = 32;

const char* robotSprite[] = {
  "00000000000000111110000000000000",
  "00000000000000111110000000000000",
  "00000000000000111110000000000000",
  "00000000000000010100000000000000",
  "00000000000000010100000000000000",
  "00000001100111111111110011000000",
  "00000001111100000000011111000000",
  "00000001100000000000000011000000",
  "00000011000011000001100001100000",
  "00001110001111100011111000111000",
  "00011010000111000001110000101100",
  "00011010000000000000000000101100",
  "00011010110000000000000110101100",
  "00011010000000000000000000101100",
  "00001110000010000000100000111000",
  "00000010000001111111000000100000",
  "00000011100000000000000011100000",
  "00000000111100000000011110000000",
  "00000000001111111111111000000000",
  "00000000111100000000011110000000",
  "00000111100001001011000011110000",
  "00001101000001001000000001011000",
  "00011001000001111001000001001100",
  "00110001000001001001000001000110",
  "00100011000000000000000001100010",
  "00100101000001111111000001010010",
  "01111101000011111111100001011111",
  "01111111001111111111111001111111",
  "01111111001111111111111001111111",
  "01111101001111111111111001011111",
  "00111001101111111111111011001110",
  "00000001111111111111111111000000"
};

//helpers I use to draw sprites, wrap text and format data

void drawSprite(const char* sprite[], int rows, int x, int y, int scale) {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; sprite[row][col] != '\0'; col++) {
      if (sprite[row][col] == '1') {
        display.fillRect(x + col * scale, y + row * scale, scale, scale, FG);
      }
    }
  }
}

void drawSpriteResized(const char* sprite[], int srcW, int srcH, int x, int y, int targetW, int targetH) {
  for (int ty = 0; ty < targetH; ty++) {
    int sy = (long)ty * srcH / targetH;

    for (int tx = 0; tx < targetW; tx++) {
      int sx = (long)tx * srcW / targetW;

      if (sprite[sy][sx] == '1') {
        display.drawPixel(x + tx, y + ty, FG);
      }
    }
  }
}

void useTinyText() {
  display.setFont();
  display.setTextSize(1);
  display.setTextColor(FG);
}

void printWrapped(String text, int x, int y, int maxChars, int lineHeight, int maxLines) {
  int line = 0;

  while (text.length() > 0 && line < maxLines) {
    int cut = min(maxChars, (int)text.length());

    if (text.length() > maxChars) {
      int lastSpace = text.lastIndexOf(' ', maxChars);
      if (lastSpace > 0) cut = lastSpace;
    }

    display.setCursor(x, y + line * lineHeight);
    display.print(text.substring(0, cut));

    text = text.substring(cut);
    text.trim();
    line++;
  }
}

//suggestions: clear sky=work outside, cloudy=work at a cafe, rain/snow=work indoors
String chooseSuggestion(int tempF, String condition) {
  if (condition == "Rain" || condition == "Snow" || condition == "Thunderstorm") {
    return "It is a good idea to stay indoors and work from home today :)";
  } 
  else if (condition == "Clear") {
    return "Nice weather! Consider working from a park :D";
  } 
  else if (condition == "Clouds") {
    return "It is gloomy today. Maybe work from a cool cafe? ;)";
  } 
  else {
    return "Weather is unpredictable... cozy desk work session? :P";
  }
}

bool connectWiFi() {
  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int tries = 0;

  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("WiFi failed");
  return false;
}

String httpGet(String url) {
  HTTPClient http;
  WiFiClientSecure secureClient;

  Serial.println("Requesting:");
  Serial.println(url);

  if (url.startsWith("https://")) {
    secureClient.setInsecure();
    http.begin(secureClient, url);
  } else {
    http.begin(url);
  }

  int status = http.GET();

  Serial.print("HTTP status: ");
  Serial.println(status);

  if (status <= 0) {
    Serial.print("HTTP error: ");
    Serial.println(http.errorToString(status));
    http.end();
    return "";
  }

  String response = http.getString();
  http.end();

  return response;
}

String formatPrice(float price) {
  return "$" + String(price, 2);
}

String formatChange(float changePercent) {
  String text = "";

  if (changePercent >= 0) {
    text += "+";
  }

  text += String(changePercent, 2);
  text += "%";

  return text;
}

void fetchMarket(String symbol, DailyToshData &data) {

  String url = "https://finnhub.io/api/v1/quote?symbol=";
  url += symbol;
  url += "&token=";
  url += FINNHUB_KEY;

  String response = httpGet(url);

  if (response == "") {
    Serial.println("Empty");
    return;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("Stock JSON parse failed for ");
    Serial.print(symbol);
    Serial.print(": ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("error")) {
    Serial.print("Finnhub stock error for ");
    Serial.print(symbol);
    Serial.print(": ");
    Serial.println(doc["error"].as<String>());
    return;
  }

  float currentPrice = doc["c"] | 0.0;
  float percentChange = doc["dp"] | 0.0;

  StockData stock;
  stock.symbol = symbol;
  stock.price = currentPrice;
  stock.changePercent = percentChange;

  data.stocks.push_back(stock);

  Serial.print("Stock parsed: ");
  Serial.print(stock.symbol);
  Serial.print(" ");
  Serial.print(stock.price);
  Serial.print(" ");
  Serial.print(stock.changePercent);
  Serial.println("%");
}

String toLowerCase(String text) {
  text.toLowerCase();
  return text;
}

//a bunch of key words to filter through the headlines and get only big tech news
bool isBigTechHeadline(String headline) {
  String h = toLowerCase(headline);

   String keywords[] = {
    "openai",
    "chatgpt",
    "gpt",
    "gpt-4",
    "gpt-5",
    "sam altman",
    "sora",
    "operator",
    "codex",
    "anthropic",
    "claude",
    "dario amodei",
    "constitutional ai",
    "nvidia",
    "nvda",
    "jensen huang",
    "cuda",
    "blackwell",
    "hopper",
    "h100",
    "h200",
    "b100",
    "b200",
    "dgx",
    "grace blackwell",
    "amd",
    "ryzen",
    "epyc",
    "radeon",
    "instinct",
    "mi300",
    "mi325",
    "mi350",
    "lisa su",
    "microsoft",
    "msft",
    "azure",
    "copilot",
    "github copilot",
    "satya nadella",
    "meta",
    "facebook",
    "instagram",
    "threads",
    "whatsapp",
    "llama",
    "mark zuckerberg",
    "zuckerberg",
    "google",
    "alphabet",
    "googl",
    "gemini",
    "deepmind",
    "sundar pichai",
    "veo",
    "imagen",
    "apple",
    "aapl",
    "apple intelligence",
    "tim cook",
    "siri",
    "amazon",
    "aws",
    "bedrock",
    "andy jassy",
    "amazon web services",
    "tesla",
    "optimus",
    "xai",
    "grok",
    "elon musk",
    "robotics",
    "robot",
    "humanoid",
    "figure ai",
    "figure",
    "agility robotics",
    "boston dynamics",
    "unitree",
    "sanctuary ai",
    "mistral",
    "cohere",
    "perplexity",
    "midjourney",
    "stability ai",
    "runway",
    "hugging face",
    "semiconductor",
    "chip",
    "chips",
    "chipmaker",
    "chipmakers",
    "gpu",
    "gpus",
    "cpu",
    "cpus",
    "accelerator",
    "ai accelerator",
    "artificial intelligence",
    " ai ",
    "genai",
    "machine learning",
    "deep learning",
    "large language model",
    "llm",
    "foundation model",
    "agentic ai",
    "reasoning model",
    "multimodal",
    "datacenter",
    "data center",
    "cloud computing",
    "cloud",
    "open source ai",
    "venture capital",
    "startup",
    "silicon valley",
    "y combinator",
    "yc",
    "tech giant",
    "big tech"
  };

  int keywordCount = sizeof(keywords) / sizeof(keywords[0]);

  for (int i = 0; i < keywordCount; i++) {
    if (h.indexOf(keywords[i]) != -1) {
      return true;
    }
  }

  return false;
}

void fetchNews(DailyToshData &data) {

  String url = "https://finnhub.io/api/v1/news?category=technology&token=";
  url += FINNHUB_KEY;

  String response = httpGet(url);

  if (response == "") {
    Serial.println("Empty news");
    return;
  }

  DynamicJsonDocument doc(32768);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("News JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  if (!doc.is<JsonArray>()) {
    Serial.println("News response was not an array");
    return;
  }

  for (int i = 0; i < doc.size() && data.headlines.size() < 3; i++) {
    String headline = doc[i]["headline"] | "";

    if (headline == "") {
      continue;
    }

    if (isBigTechHeadline(headline)) {
      NewsItem item;
      item.headline = headline;
      data.headlines.push_back(item);

      Serial.print("News headline added: ");
      Serial.println(headline);
    }
  }

  if (data.headlines.size() == 0) {
    NewsItem item;
    item.headline = "No big tech news today!";
    data.headlines.push_back(item);
  }
}

void fetchWeather(DailyToshData &data) {

  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += WEATHER_CITY;
  url += ",";
  url += WEATHER_COUNTRY;
  url += "&units=imperial&appid=";
  url += OPENWEATHERMAP_KEY;

  Serial.println("Requesting weather:");
  Serial.println(url);

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();

  Serial.print("HTTP status: ");
  Serial.println(httpCode);

  if (httpCode != 200) {
    Serial.println("Weather failed");
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  float temp = doc["main"]["temp"] | 63.0;
  const char* conditionText = doc["weather"][0]["main"] | "Clouds";

  data.tempF = (int)round(temp);
  data.condition = String(conditionText);
  data.suggestion = chooseSuggestion(data.tempF, data.condition);

  Serial.println("Weather parsed:");
  Serial.print("Temp F: ");
  Serial.println(data.tempF);
  Serial.print("Condition: ");
  Serial.println(data.condition);
  Serial.print("Suggestion: ");
  Serial.println(data.suggestion);
}


void drawTrendIcon(float changePercent, int x, int y) {
  if (changePercent >= 0) {
    display.fillTriangle(x + 4, y, x, y + 7, x + 8, y + 7, FG);
  } else {
    display.fillTriangle(x, y, x + 8, y, x + 4, y + 7, FG);
  }
}

void drawMarketRow(String symbol, String price, String change, float changePercent, int y) {
  useTinyText();

  display.setCursor(224, y);
  display.print(symbol);

  display.setCursor(270, y);
  display.print(price);

  display.setCursor(328, y);
  display.print(change);

  drawTrendIcon(changePercent, 370, y - 1);
}

void drawNewsItem(int number, const String& headline, int y) {
  useTinyText();

  display.fillRect(24, y - 8, 13, 13, BADGE_FILL);

  display.setTextColor(BADGE_TEXT);
  display.setCursor(28, y - 6);
  display.print(number);

  display.setTextColor(FG);
  printWrapped(headline, 46, y - 8, 40, 9, 2);
}

// draws the final dashboard layout

void drawDashboard() {
  display.fillScreen(BG);
  display.setTextColor(FG);

  display.drawRect(6, 6, 388, 288, FG);

  // header
  drawSprite(robotSprite, ROBOT_H, 22, 8, 1);

  display.setFont();
  display.setTextSize(3);
  display.setTextColor(FG);
  display.setCursor(118, 17);
  display.print("DAILYTOSH");
  display.setTextSize(1);

  display.drawLine(14, 44, 386, 44, FG);

  display.drawRect(14, 48, 194, 114, FG);   // weather
  display.drawRect(216, 48, 170, 114, FG);  // market

  display.drawRect(14, 166, 372, 122, FG);  

  // weather section
  drawSpriteResized(cloudSprite, CLOUD_W, CLOUD_H, 22, 68, 88, 42);

  display.drawLine(120, 66, 120, 124, FG);

  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(FG);
  display.setCursor(134, 83);
  display.print(data.tempF);
  display.print("F");

  display.setFont(&FreeMonoBold9pt7b);
  display.setCursor(130, 111);
  display.print(data.condition);

  useTinyText();
  printWrapped(data.suggestion, 24, 136, 27, 9, 2);

  display.setFont(&FreeMono9pt7b);
  display.setTextColor(FG);
  display.setCursor(226, 68);
  display.print("MARKET");
  display.drawLine(226, 78, 374, 78, FG);

  if (data.stocks.size() > 0) {
    int marketY = 92;

    for (int i = 0; i < data.stocks.size() && i < 4; i++) {
      StockData stock = data.stocks[i];

      drawMarketRow(
        stock.symbol,
        formatPrice(stock.price),
        formatChange(stock.changePercent),
        stock.changePercent,
        marketY
      );

      marketY += 17;
    }
  } else {
    drawMarketRow("AMD",  "$137.36", "+4.85%", 4.85, 92);
    drawMarketRow("NVDA", "$218.59", "+2.95%", 2.95, 109);
    drawMarketRow("MSFT", "$379.39", "+0.12%", 0.12, 126);
    drawMarketRow("AAPL", "$295.81", "+0.69%", 0.69, 143);
  }

  display.setFont(&FreeMono9pt7b);
  display.setTextColor(FG);
  display.setCursor(24, 186);
  display.print("BIG TECH NEWS");
  display.drawLine(24, 194, 290, 194, FG);

  if (data.headlines.size() > 0) {
    int newsY = 210;

    for (int i = 0; i < data.headlines.size() && i < 3; i++) {
      drawNewsItem(i + 1, data.headlines[i].headline, newsY);
      newsY += 29;
    }
  } else {
    drawNewsItem(1, "AI trade leaves hyperscalers behind.", 210);
    drawNewsItem(2, "Could data centers work in space?", 239);
    drawNewsItem(3, "AI spending eats into buybacks.", 266);
  }

  drawSpriteResized(citySprite, CITY_W, CITY_H, 292, 199, 88, 88);
}

///////

void setup() {
  Serial.begin(115200);
  delay(2000);

  bool wifiOK = connectWiFi();

  if (wifiOK) {
    fetchWeather(data);

    data.stocks.clear();
    fetchMarket("AMD", data);
    fetchMarket("NVDA", data);
    fetchMarket("MSFT", data);
    fetchMarket("AAPL", data);

    data.headlines.clear();
    fetchNews(data);
  }

  display.init(115200, true, 2, false);
  display.setRotation(0);
  display.setFullWindow();

  display.firstPage();
  do {
    drawDashboard();
  } while (display.nextPage());

  display.hibernate();
  Serial.println("ESP32 going to sleep");
  delay(1000);

  esp_sleep_enable_timer_wakeup((uint64_t)SECONDS_TO_SLEEP * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
}
