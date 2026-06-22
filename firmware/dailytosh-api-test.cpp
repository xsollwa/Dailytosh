/*
DailyTosh API and Dashboard Test Code
*
*I created this file to test the functionality of the APIs and to experiment with the dashboard design before implementing it on the actual e-ink display. 
*Here is what it does:
*
*1. fetches weather data from OpenWeatherMap API
*2. fetches stock market data from Finnhub API
*3. uses an algorithm to filter and display top 3 big tech news headlines
*4. simulates a 400x300 pixel "e-ink" dashboard using SDL2
*5. shows the potential layout and design of the final dashboard
*6. refreshes every 24hrs
*
*this will be converted into ESP32 firmware for the physical device
*/


#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <chrono>
#include <thread>

using namespace std;
using json = nlohmann::json;

//API keys
string OPENWEATHERMAP_KEY = ""; 
string FINNHUB_KEY = "";

struct StockData {
    string symbol;
    float price = 0;
    float change = 0;
};

struct NewsItem {
    string headline;
    string source;
};

struct DailyToshData {
    float temp = 0;
    string condition = "";
    string suggestion = "";

    vector<StockData> stocks;
    vector<NewsItem> headlines;
};

//suggested work locations based on weather
string chooseSuggestion(float temp, string condition){
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


size_t writeCallback(void* contents, size_t size, size_t nmemb, string* output){
    output->append((char*)contents, size * nmemb);

    return size * nmemb;

}




string httpGet(string url){
    CURL* curl = curl_easy_init();
    string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode result = curl_easy_perform(curl);

    if(result != CURLE_OK){
        cout << "fail: " << curl_easy_strerror(result) << endl;
    }

    curl_easy_cleanup(curl);

    return response;

}

//functions for fetching the data from the two apis

void fetchWeather(DailyToshData &data){
    cout << "\n TESTING WEATHER \n";

    string url = "https://api.openweathermap.org/data/2.5/weather?q=Cleveland,US&units=imperial&appid=" + OPENWEATHERMAP_KEY;

    string response = httpGet(url);
    json doc = json::parse(response);

    if(doc.contains("cod") && doc["cod"] != 200){
        cout << "weather error: " << doc["message"] << endl;
        return;
    }

    data.temp = doc["main"]["temp"];
    data.condition = doc["weather"][0]["main"];
    data.suggestion = chooseSuggestion(data.temp, data.condition);
}

void fetchMarket(string symbol, DailyToshData &data){
    cout << "\n TESTING MARKET \n";

    string url = "https://finnhub.io/api/v1/quote?symbol=" + symbol + "&token=" + FINNHUB_KEY;

    string response = httpGet(url);
    json doc = json::parse(response);

    if(doc.contains("error")){
        cout << "market error: " << doc["error"] << endl;
        return;
    }

    StockData stock;
    stock.symbol = symbol;
    stock.price = doc["c"];
    stock.change = doc["dp"];

    data.stocks.push_back(stock);
}

string toLowerCase(string text){
    transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

void fetchNews(DailyToshData &data){
    cout << "\n TESTING NEWS \n";

    string url = "https://finnhub.io/api/v1/news?category=technology&token=" + FINNHUB_KEY;

    string response = httpGet(url);
    json doc = json::parse(response);

    if (!doc.is_array() || doc.size() == 0){
        return;
    }

    //filtering the headlines to include only tech related stuff

    vector<string> keywords = {

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

    //I will probably add more to this list in the future
};


    for (int i = 0; i < doc.size() && data.headlines.size()< 3; i++){
        string headline = doc[i]["headline"];
        string source = doc[i]["source"];
        string lowerHeadline = toLowerCase(headline);
        bool isBigTech = false;

        for (string keyword : keywords){
            if (lowerHeadline.find(keyword) != string::npos){
                isBigTech = true;
                break;
            }
        }
        if (isBigTech){
            NewsItem item;
            item.headline = headline;
            item.source = source;
            data.headlines.push_back(item);

        }
    }

    if (data.headlines.size() == 0){
        data.headlines.push_back({"No big tech news today!", ""});
    }

}

void drawText(SDL_Renderer* renderer, TTF_Font* font, string text, int x, int y){
    SDL_Color black = {0, 0, 0, 255};

    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), black);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

}


void drawTextColor(SDL_Renderer* renderer, TTF_Font* font, string text, int x, int y, SDL_Color color){
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void drawWrappedText(SDL_Renderer* renderer, TTF_Font* font, string text, int x, int y, int maxWidth, int lineHeight){
    string line = "";
    string word = "";

    for (int i = 0; i <= text.length(); i++){
        if (i == text.length() || text[i] == ' '){
            string testLine = line;
            if (testLine.length() > 0) testLine += " ";
            testLine += word;

            int w, h;
            TTF_SizeText(font, testLine.c_str(), &w, &h);

            if (w > maxWidth && line.length() > 0){
                drawText(renderer, font, line, x, y);
                y += lineHeight;
                line = word;
            } else {
                line = testLine;
            }

            word = "";
        } else {
            word += text[i];
        }
    }

    if (line.length() > 0){
        drawText(renderer, font, line, x, y);
    }
}



void drawImage(
    SDL_Renderer* renderer,
    string filename,
    int x,
    int y,
    int w,
    int h
){
    SDL_Surface* surface = IMG_Load(filename.c_str());

    if(!surface){
        cout << "Failed to load: " << filename << endl;
        return;
    }

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dest = {x, y, w, h};

    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void drawScreen(DailyToshData &data){
    const int W = 400;
    const int H = 300;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    TTF_Font* titleFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 22);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 11);
    TTF_Font* smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 8);

    if(!titleFont || !font || !smallFont){
        cout << "font fail\n";
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
        "DailyTosh E-ink Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        W,
        H,
        0
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Color white = {255, 255, 255, 255};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    SDL_Rect border = {4, 4, W - 8, H - 8};
    SDL_RenderDrawRect(renderer, &border);

    // heder
    drawImage(renderer, "assets/robot.png", 18, 12, 32, 32); //little robot art
    drawText(renderer, titleFont, "DAILYTOSH", 126, 17);
    SDL_RenderDrawLine(renderer, 12, 56, 388, 56);

    // boxes
    SDL_Rect weatherBox = {10, 66, 190, 100};
    SDL_Rect marketBox  = {210, 66, 180, 106};
    SDL_Rect newsBox    = {10, 180, 380, 110};

    SDL_RenderDrawRect(renderer, &weatherBox);
    SDL_RenderDrawRect(renderer, &marketBox);
    SDL_RenderDrawRect(renderer, &newsBox);

    // weather
    drawImage(renderer, "assets/cloud.png", 22, 76, 72, 58); //cloud art
    SDL_RenderDrawLine(renderer, 105, 78, 105, 132);

    drawText(renderer, titleFont, to_string((int)data.temp) + "F", 122, 78);
    drawText(renderer, font, data.condition, 124, 110);

    drawWrappedText(renderer, smallFont, data.suggestion, 18, 136, 175, 9);

    // market
    drawText(renderer, font, "THE MARKET TODAY", 222, 74);
    SDL_RenderDrawLine(renderer, 222, 91, 377, 91);

    int marketY = 99;

    for (StockData stock : data.stocks){
        string arrow = stock.change >= 0 ? "^" : "v";
        string changeText = to_string(stock.change).substr(0, 5) + "%";
        string priceText = "$" + to_string(stock.price).substr(0, 6);

        drawText(renderer, smallFont, stock.symbol, 222, marketY);

        drawText(renderer, smallFont, priceText, 265, marketY);

        drawText(renderer, smallFont, changeText + " " + arrow, 325, marketY);

        if (marketY < 150) {
            SDL_RenderDrawLine(renderer, 222, marketY + 12, 377, marketY + 12);
        }

        marketY += 14;
    }

    // news
    drawText(renderer, font, "BIG TECH NEWS", 20, 188);
    SDL_RenderDrawLine(renderer, 20, 205, 255, 205);

    int newsY = 212;

    for(int i = 0; i < data.headlines.size() && i < 3; i++){
        SDL_Rect badge = {20, newsY, 15, 15};
        SDL_RenderFillRect(renderer, &badge);

        drawTextColor(renderer, smallFont, to_string(i + 1), 25, newsY + 2, white);

        drawWrappedText(
            renderer,
            smallFont,
            data.headlines[i].headline,
            43,
            newsY,
            235,
            9
        );

        newsY += 27;
    }

    
    //city art
    drawImage(renderer, "assets/city.png", 296, 215, 80, 68);

    SDL_RenderPresent(renderer);

    bool running = true;
    SDL_Event event;

    while (running){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = false;
            }
        }
        SDL_Delay(20);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_CloseFont(titleFont);
    TTF_CloseFont(font);
    TTF_CloseFont(smallFont);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


int main(){
    while (true){
        DailyToshData data;

        fetchWeather(data);

        //these can be changed to other companies of interest vvv
        fetchMarket("AMD", data);
        fetchMarket("NVDA", data);
        fetchMarket("MSFT", data);
        fetchMarket("META", data);
        fetchMarket("AAPL", data);
        fetchNews(data);

        drawScreen(data);

        //refreshing after 24hrs
        this_thread::sleep_for(chrono::hours(24));

    }
    
    
    return 0;
}