/*
 * Esp8266 WiFi Weather/clock/ham bands info
 * Adapted from a sketch by Nick of Educ8s.tv
 *
 * 3/2/2019 by Chris Gozzard ZS1CDG
 */
#include <ESP8266WiFi.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <time.h>
#include <TextFinder.h>

#define D0 3 // GPIO3 maps to Ardiuno D0
#define D1 1 // GPIO1 maps to Ardiuno D1
#define D2 16 // GPIO16 maps to Ardiuno D2
#define D3 5 // GPIO5 maps to Ardiuno D3
#define D4 4 // GPIO4 maps to Ardiuno D4
#define D5 0 // GPIO14 maps to Ardiuno D5
#define D6 2 // GPIO12 maps to Ardiuno D6
#define D7 14 // GPIO13 maps to Ardiuno D7
#define D8 12 // GPIO0 maps to Ardiuno D8
#define D9 13 // GPIO2 maps to Ardiuno D9
#define D10 15 // GPIO15 maps to Ardiuno D10

const char* ssid     = "";      // SSID of local network
const char* password = "41394342";   // Password on network
String APIKEY = ""; // Get a API key from openweathermap.org
//String CityID = "6942529"; //Durbanville / ZA
String CityID = "3369157"; //Cape Town / ZA
const char* CallSign = ""; //Your amateur radio  callsign

WiFiClient client;
char servername[]="api.openweathermap.org";  // remote server we will connect to
char server[]="www.hamqsl.com";
String result;

int  counter = 60;

String weatherDescription ="";
String weatherLocation = "";
String Country;
float Temperature;
float Humidity;
float Pressure;
float Kvalue;
float Avalue;
float SFIvalue;
float Sunspots;
float Wind;
float WindAngle;

LiquidCrystal lcd(D8,D9,D4,D5,D6,D7); 

void setup() {
  Serial.begin(115200);
  int cursorPosition=0;
  lcd.begin(16, 2);
  lcd.print("   ");
  lcd.print(CallSign);
  lcd.print("   ");
  delay (500);
  lcd.clear();
  lcd.print("   Connecting");  
  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(cursorPosition,2); 
    lcd.print(".");
    cursorPosition++;
  }
  lcd.clear();
  lcd.print("   Connected!");
  Serial.println("Connected");
  delay(1000);

  
configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  lcd.clear();
  lcd.setCursor(0,0);
  //lcd.println("\nWaiting for time");
  while (!time(nullptr)) {
    lcd.print(".");
    delay(1000);
}
}

void loop() {
    if(counter == 60) //Get new data every 10 minutes
    {
      counter = 0;
      displayGettingData();
      delay(1000);
      getWeatherData();
      configTime(2 * 3600, 0, "pool.ntp.org", "time.nist.gov");
      getHamData();
    }else
    {
      counter++;
      displayWeather(weatherLocation,weatherDescription);
      delay(5000);
      displayConditions(Temperature,Humidity,Pressure);
      delay(5000);
      displayTime();
      delay(5000);
      displayWind(Wind, WindAngle);
      delay(5000);
      displayHam(Kvalue,Avalue,SFIvalue,Sunspots);
      delay(5000);
      displayBands(SFIvalue);
      delay(5000);
    }
}

void getHamData()
{
if (client.connect(server, 80)) {
    Serial.println("connected");
    client.println("GET /solarxml.php HTTP/1.1");
    client.println("Host: www.hamqsl.com");
    client.println("Connection: close");
    client.println();
    } 
    else {
    Serial.println("connection failed"); 
    }
    while(client.connected() && !client.available()) delay(1); //waits for data
    TextFinder  finder(client); 
    finder.find("<solarflux>");
    SFIvalue = finder.getValue();
    finder.find("<aindex>");
    Avalue = finder.getValue();
    finder.find("<kindex>");
    Kvalue = finder.getValue();
    finder.find("<sunspots>");
    Sunspots = finder.getValue();
    client.stop(); //stop client
   
}

void getWeatherData() //client function to send/receive GET request data.
{
  if (client.connect(servername, 80)) {  //starts client connection, checks for connection
    client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } 
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  while(client.connected() && !client.available()) delay(1); //waits for data
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
      result = result+c;
    }

  client.stop(); //stop client
  result.replace('[', ' ');
  result.replace(']', ' ');
  Serial.println(result);

char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';

StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);
if (!root.success())
{
  Serial.println("parseObject() failed");
}

String location = root["name"];
String country = root["sys"]["country"];
float temperature = root["main"]["temp"];
float humidity = root["main"]["humidity"];
String weather = root["weather"]["main"];
String description = root["weather"]["description"];
float pressure = root["main"]["pressure"];
float wind = root["wind"]["speed"];
float windAngle = root["wind"]["deg"];

weatherDescription = description;
weatherLocation = location;
Country = country;
Temperature = temperature;
Humidity = humidity;
Pressure = pressure;
WindAngle = windAngle;
Wind = wind;
}

void displayWeather(String location,String description)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(location);
  lcd.print(", ");
  lcd.print(Country);
  lcd.setCursor(0,1);
  lcd.print(description);
}

void displayConditions(float Temperature,float Humidity, float Pressure)
{
  lcd.clear();
  lcd.print("T:"); 
 lcd.print(Temperature,0);
 lcd.print((char)223);
 lcd.print("C ");
 
 //Printing Humidity
 lcd.print(" H:");
 lcd.print(Humidity,0);
 lcd.print("%");
 
 //Printing Pressure
 lcd.setCursor(0,1);
 lcd.print("P: ");
 lcd.print(Pressure,0);
 lcd.print(" hPa");

}

void displayGettingData()
{
  lcd.clear();
  lcd.print("Getting data");
}

void displayTime()
{
  lcd.clear();
  lcd.setCursor(0,0);
  time_t now = time(nullptr);
  lcd.println(ctime(&now));;
}

void displayWind(float wind,float windAngle)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wind:");
  lcd.print(wind,0);
  lcd.print("m/s");
  lcd.setCursor(0,1);
  lcd.print("Direction:");
  lcd.print(windAngle,0);
  lcd.print((char)223);
} 


void displayHam(float Kvalue, float Avalue, float SFIvalue, float Sunspots)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SFI:");
  lcd.print(SFIvalue,0);
  lcd.print(" K:");
  lcd.print(Kvalue,0);
  lcd.print(" A:");
  lcd.print(Avalue,0);
  lcd.setCursor(0,1);
  lcd.print("Sunspots:");
  lcd.print(Sunspots,0);
}

void displayBands (float SFIvalue){
  lcd.clear();
  lcd.setCursor(0,0);
  if (SFIvalue > 64 && SFIvalue < 71 )
  {
  lcd.print("Bands above 40m");
  lcd.setCursor(0,1);
  lcd.print("unusable");
  }
  if (SFIvalue > 70 && SFIvalue < 91 )
  {
  lcd.print("all bands up ");
  lcd.setCursor(0,1);
  lcd.print("through 20m");
  }
  if (SFIvalue > 90 && SFIvalue < 121 )
  {
  lcd.print("Fair conditions");
  lcd.setCursor(0,1);
  lcd.print("up through 15m");
  }
  if (SFIvalue > 120 && SFIvalue < 151 )
  {
  lcd.print("good conditions ");
  lcd.setCursor(0,1);
  lcd.print("up through 10m");
  }
  if (SFIvalue > 150 && SFIvalue < 201 )
  {
  lcd.print("Excellent up");
  lcd.setCursor(0,1);
  lcd.print("through 10m ");
  }
  if (SFIvalue > 200 && SFIvalue < 300 )
  {
  lcd.println("Reliable comms");
  lcd.setCursor(0,1);
  lcd.print("up to 6m ");
  }


}
