/*
DailyTosh Firmware

*This firmware is a conversion of the original DailyTosh test c++ code into an ESP32-compatible .ino format
*I basically copy and pasted my C++ code and made these conversion changes:
*
*- firmware version uses esp32 wifi stack and httpClient library instead of cURL
*- esp32 uses ArduinoJson to parse API data instead of nlohmann/json
*- esp32 will use an actual e-ink display for displaying the dashboard instead of a SDL2 + SDL_ttf + SDL_image simulation
*- pixel art is now converted to sprite arrays
*- standard c++ to arduino syntax conversions
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>

//my wiring vvv
// DIN/MOSI -> GPIO 23
// CLK/SCK  -> GPIO 18
// CS       -> GPIO 5
// DC       -> GPIO 22
// RST      -> GPIO 21
// BUSY     -> GPIO 4

GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(
  GxEPD2_420(/*CS=*/5, /*DC=*/22, /*RST=*/21, /*BUSY=*/4)
);


const char* WIFI_NAME ="TMOBILE-B11A";
const char* WIFI_PASS = "v4strb6kf9c";

//API keys
String OPENWEATHERMAP_KEY = ""; 
String FINNHUB_KEY = "";

struct StockData {
    String symbol;
    float price = 0;
    float change = 0;
};

struct NewsItem {
    String headline;
    String source;
};

struct DailyToshData {
    float temp = 0;
    String condition = "";
    String suggestion = "";

    std::vector<StockData> stocks;
    std::vector<NewsItem> headlines;
};

//suggested work locations based on weather
String chooseSuggestion(float temp, String condition){
    if (condition == "Rain" || condition == "Snow" || condition == "Thunderstorm"){
        return "It is a good idea to stay indoors and work from home today :)";
    } else if (condition == "Clear"){
        return "The weather is nice! Consider working from a park today :D";
    } else if (condition == "Clouds"){
        return "It is gloomy today. Maybe work from a cool cafe? ;)";
    } else {
        return "The weather is a bit unpredictable...cozy desk work session? :P";
    }
}

String httpGet(String url) {
  HTTPClient http;
  http.begin(url);

  int status = http.GET();
  Serial.print("HTTP status: ");
  Serial.println(status);

  String response = http.getString();
  http.end();

  return response;
}

//connecting to wifi 
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_NAME, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nconnected");
}

//fetching the data from the apis
void fetchWeather(DailyToshData &data) {
  String url = "https://api.openweathermap.org/data/2.5/weather?q=Cleveland,US&units=imperial&appid=" + OPENWEATHERMAP_KEY;

  String response = httpGet(url);

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.println("weather fail");
    return;
  }

  data.temp = doc["main"]["temp"];
  data.condition = doc["weather"][0]["main"].as<String>();
  data.suggestion = chooseSuggestion(data.temp, data.condition);
}

//////

void fetchMarket(String symbol, DailyToshData &data) {
  String url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + FINNHUB_KEY;

  String response = httpGet(url);

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.println("market fail");
    return;
  }

  StockData stock;
  stock.symbol = symbol;
  stock.price = doc["c"];
  stock.change = doc["dp"];

  data.stocks.push_back(stock);
}


String lowerText(String text) {
  text.toLowerCase();
  return text;
}


//filtering the headlines to include only big tech news

bool isBigTechHeadline(String headline) {
  String h = lowerText(headline);

  //these are the keywords used to filter headlines, can add or remove some based on interest
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

  for (String keyword : keywords) {
    if (h.indexOf(keyword) != -1) {
      return true;
    }
  }

  return false;
}

void fetchNews(DailyToshData &data) {
  String url = "https://finnhub.io/api/v1/news?category=technology&token=" + FINNHUB_KEY;

  String response = httpGet(url);

  DynamicJsonDocument doc(20000);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.println("News JSON failed");
    return;
  }

  for (int i = 0; i < doc.size() && data.headlines.size() < 3; i++) {
    String headline = doc[i]["headline"].as<String>();
    String source = doc[i]["source"].as<String>();

    if (isBigTechHeadline(headline)) {
      NewsItem item;
      item.headline = headline;
      item.source = source;
      data.headlines.push_back(item);
    }
  }

  if (data.headlines.size() == 0) {
  NewsItem item;
  item.headline = "No big tech news today!";
  item.source = "";
  data.headlines.push_back(item);  }
}


void drawSprite(const char* sprite[], int rows, int x, int y, int scale) {
  for (int row = 0; row < rows; row++) {
    for (int col = 0; sprite[row][col] != '\0'; col++) {
      if (sprite[row][col] == '1') {
        display.fillRect(x + col * scale, y + row * scale, scale, scale, GxEPD_BLACK);
      }
    }
  }
}


//pixel art vvv

const char* robotSprite[] = {
  "00111100",
  "01000010",
  "10100101",
  "10000001",
  "10111101",
  "01000010",
  "00111100",
  "00011000",
  "01111110",
  "10011001"
};

const char* cloudSprite[] = {
  "000111100000",
  "001000010000",
  "010000001100",
  "100000000010",
  "100000000010",
  "011111111100"
};

const char* citySprite[] = {
  "000010000000",
  "000111000000",
  "001111100000",
  "001010100000",
  "001111100000",
  "111111111100",
  "101010101100",
  "111111111111"
};

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

void drawDashboard(DailyToshData &data) {
  display.setFullWindow();
  display.firstPage();

  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    display.drawRect(4, 4, 392, 292, GxEPD_BLACK);

    drawSprite(robotSprite, 10, 18, 12, 3);

    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(126, 36);
    display.print("DAILYTOSH");

    display.drawLine(12, 56, 388, 56, GxEPD_BLACK);

    display.drawRect(10, 66, 190, 100, GxEPD_BLACK);
    display.drawRect(210, 66, 180, 106, GxEPD_BLACK);
    display.drawRect(10, 180, 380, 110, GxEPD_BLACK);

    drawSprite(cloudSprite, 6, 22, 82, 5);

    display.drawLine(105, 78, 105, 132, GxEPD_BLACK);

    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(122, 100);
    display.print((int)data.temp);
    display.print("F");

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(124, 125);
    display.print(data.condition);

    display.setFont(&FreeMono9pt7b);
    printWrapped(data.suggestion, 18, 148, 28, 10, 2);

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(222, 84);
     display.print("MARKET");

    display.drawLine(222, 92, 377, 92, GxEPD_BLACK);

    display.setFont(&FreeMono9pt7b);

    int marketY = 105;

    for (int i = 0; i < data.stocks.size() && i < 5; i++) {
      StockData stock = data.stocks[i];

      display.setCursor(222, marketY);
      display.print(stock.symbol);

      display.setCursor(268, marketY);
      display.print("$");
      display.print(stock.price, 0);

      display.setCursor(320, marketY);
      if (stock.change >= 0) display.print("+");
      display.print(stock.change, 1);
      display.print("%");

      display.drawLine(222, marketY + 4, 377, marketY + 4, GxEPD_BLACK);

      marketY += 15;
    }

    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(20, 198);
    display.print("BIG TECH NEWS");

    display.drawLine(20, 205, 255, 205, GxEPD_BLACK);

    display.setFont(&FreeMono9pt7b);

    int newsY = 218;

     for (int i = 0; i < data.headlines.size() && i < 3; i++) {
      display.fillRect(20, newsY - 10, 14, 14, GxEPD_BLACK);

      display.setTextColor(GxEPD_WHITE);
      display.setCursor(24, newsY);
      display.print(i + 1);

      display.setTextColor(GxEPD_BLACK);
      printWrapped(data.headlines[i].headline, 43, newsY, 38, 10, 2);

      newsY += 28;
    }

    drawSprite(citySprite, 8, 305, 220, 5);

  } while (display.nextPage());
}

void refreshDailyTosh() {
  data.stocks.clear();
  data.headlines.clear();

  fetchWeather(data);

  //can adjust and put the companies I'm most interested in checking in the market
  fetchMarket("AMD", data);
  fetchMarket("NVDA", data);
  fetchMarket("MSFT", data);
  fetchMarket("META", data);
  fetchMarket("AAPL", data);
  fetchNews(data);

  drawDashboard(data);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  display.init(115200, true, 2, false);
  display.setRotation(0);

connectWiFi();

  refreshDailyTosh();

  display.hibernate();
}

void loop() {
  //refreshed data after 24hrs
  delay(24UL * 60UL * 60UL * 1000UL);

  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  refreshDailyTosh();

  display.hibernate();
}