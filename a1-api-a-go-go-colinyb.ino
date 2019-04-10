/*A sketch to get the ESP8266 on the network and connect to some open services via HTTP to
 * get our external IP address and (approximate) geolocative information in the getGeo()
 * function. To do this we will connect to http://freegeoip.net/json/, an endpoint which
 * requires our external IP address after the last slash in the endpoint to return location
 * data, thus http://freegeoip.net/json/XXX.XXX.XXX.XXX
 * 
 * This sketch also introduces the flexible type definition struct, which allows us to define
 * more complex data structures to make receiving larger data sets a bit cleaner/clearer.
 * 
 * jeg 2017
 * 
 * updated to new API format for Geolocation data from ipistack.com
 * brc 2019
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> //provides the ability to parse and construct JSON objects
//
//const char* ssid = "CYB";
//const char* pass = "m1ck3yM0us3";
const char* ssid = "University of Washington";
const char* pass = "";
//const char* ipKey = "KNCAW2V3YI";
//const char* newsKey = "01746050a5824dc7b223e27bf923d28a";
const char* aiqKey = "698F1252-DB57-457A-AADF-B4173C6485A1";
const char* locKey = "b4e3e1ef91da097fa72afb9db94629f2";

//FOR FIRST ATTEMPT
//typedef struct { //here we are creating our own geolocation data type to hold our geolocation data
//  String country; 
//  String region;
//  String city;
//} GeoLocation;
//
//GeoLocation location; //creating an instance of a geolocation object named location
//
//typedef struct {
//  String title;
//  String author;
//  String description;
//} NewsArticle;
//
//NewsArticle news[5];

typedef struct {
  String lat;
  String lon;
  String city;
} LatLon;

LatLon location;

typedef struct {
  String aqi;
  String quality;
} AirQual;

AirQual airQuality;

void setup() {
  Serial.begin(115200); //starts serial port
  delay(10); //delays 10ms
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__)); //compiled file
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__)); //time of compiling
  
  Serial.print("Connecting to "); Serial.println(ssid); //prints what wifi is being connected to

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass); //handles connecting to wifi

  while (WiFi.status() != WL_CONNECTED) { //showing the user something is happening until connected
    delay(500);
    Serial.print(".");
  }
  // printing confirmation and assigned internal ip address 
  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());

  getLoc();
  getAir();

  Serial.println();
  Serial.print("You are in " + location.city + ". ");
  Serial.print("The air quality index here is " + airQuality.aqi + ". ");
  Serial.println("This is considered " + airQuality.quality + ".");

//  //THIS WAS MY PRINTING/CALLING OF METHODS FROM THE APIS I CAN'T USE
//
//  getLoc(); //calls the getLoc method
//
//  Serial.println(); 
//  Serial.println("You are currently connted to the internet from " + location.city + ", " + location.region + ", " + location.country + ".");
//
//  getNews();
//
//  Serial.println();
//  Serial.println("Here is the most recent news in your area:");
//  for (int i=0; i<5; i++) {
//    Serial.println("Title: " + news[i].title);
//    Serial.println("Author: " + news[i].author);
//    Serial.println("Description: " + news[i].description);
//  }
}

void loop() {
  //if we put getIP() here, it would ping the endpoint over and over . . . DOS attack?
}

String getIP() {
  HTTPClient theClient;
  String ipAddress;

  theClient.begin("http://api.ipify.org/?format=json"); //Make the request
  int httpCode = theClient.GET(); //get the http code for the request

  if (httpCode > 0) {
    if (httpCode == 200) { //making sure the request was successful

      DynamicJsonBuffer jsonBuffer;

      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    } else { //error message for unsuccessful request
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  return ipAddress; //returning the ipAddress 
}

void getLoc() {
  HTTPClient theClient;
  Serial.println("Making Location HTTP request");
  theClient.begin("http://api.whoapi.com/?apikey=b4e3e1ef91da097fa72afb9db94629f2&r=geo&domain=whoapi.com&ip=" + getIP()); //return IP as .json object
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String jsonData = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(jsonData);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(jsonData);
        return;
      }

      // Here we are adding the data from the API response to the instance of the LatLon data type we constructed earlier
      location.lat = root["geo_latitude"].as<String>();
      location.lon = root["geo_longitude"].as<String>();
      location.city = root["geo_city"].as<String>();
      

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

void getAir() {
  HTTPClient theClient;
  Serial.println("Making Location HTTP request");
  String url = "http://www.airnowapi.org/aq/forecast/latLong/?format=application/json&latitude=" + location.lat + "&longitude=" + location.lon + "&date=2019-04-09&distance=25&API_KEY=" + aiqKey;
  theClient.begin(url); //return IP as .json object
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String jsonData = theClient.getString();
      jsonData =  "{\"start\": "+ jsonData + "}";
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(jsonData);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(jsonData);
        return;
      }

      // Here we are adding the data from the API response to the instance of the LatLon data type we constructed earlier
      airQuality.aqi = root["start"][0]["AQI"].as<String>();
      airQuality.quality = root["start"][0]["Category"]["Name"].as<String>();
      

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}


/* LONG STORY SHORT, I FORGOT THAT HTTPS REQUESTS REQUIRE EXTRA WORK... AND I DON'T HAVE TIME TO FIX IT SO IN PLACE OF THESE TWO I AM GRABBING TWO RANDOM APIS TO USE... 
 *  ..........LIFES ROUGH */

//// Use WS3
//void getLoc() {
//  HTTPClient theClient;
//  Serial.println("Making Location HTTP request");
//  theClient.begin("https://api.ip2location.com/v2/?ip=" + getIP() + "&key=" + ipKey + "&package=WS3"); //return IP as .json object
//  int httpCode = theClient.GET();
//
//  if (httpCode > 0) {
//    if (httpCode == 200) {
//      Serial.println("Received HTTP payload.");
//      DynamicJsonBuffer jsonBuffer;
//      String jsonData = theClient.getString();
//      Serial.println("Parsing...");
//      JsonObject& root = jsonBuffer.parse(jsonData);
//      Serial.println(jsonData);
//
//      // Test if parsing succeeds.
//      if (!root.success()) {
//        Serial.println("parseObject() failed");
//        Serial.println(jsonData);
//        return;
//      }
//
//      // Here we are adding the data from the API response to the instance of the GeoLocation data type we constructed earlier
//      location.country = root["country_name"].as<String>();
//      location.region = root["region_name"].as<String>();
//      location.city = root["city_name"].as<String>();
//
//    } else {
//      Serial.println("Something went wrong with connecting to the endpoint.");
//    }
//  }
//}
//
//// title, author, description
//void getNews() {
//  HTTPClient theClient;
//  Serial.println("Making News HTTP request");
//  theClient.begin("https://newsapi.org/v2/everything?q=" + location.region + "&sortBy=publishedAt&apiKey=" + newsKey); //return news data as .json object
//  int httpCode = theClient.GET();
//
//  if (httpCode > 0) {
//    if (httpCode == 200) {
//      Serial.println("Received HTTP payload.");
//      DynamicJsonBuffer jsonBuffer;
//      String jsonData = theClient.getString();
//      Serial.println("Parsing...");
//      JsonObject& root = jsonBuffer.parse(jsonData);
//      Serial.println(jsonData);
//
//      // Test if parsing succeeds.
//      if (!root.success()) {
//        Serial.println("parseObject() failed");
//        Serial.println(jsonData);
//        return;
//      }
//
//      for (int i=0; i<5; i++) {
//        NewsArticle article;
//        article.title = root["articles"][i]["title"].as<String>();
//        article.author = root["articles"][i]["author"].as<String>();
//        article.description = root["articles"][i]["description"].as<String>();
//        news[i] = article;
//      }
//      
//    } else {
//      Serial.println("Something went wrong with connecting to the endpoint.");
//    }
//  }
//}

