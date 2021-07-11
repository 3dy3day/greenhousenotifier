#include <WiFi.h>
#include <HTTPClient.h>
#include <SHT3x.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>

SHT3x Sensor;
WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "jp.pool.ntp.org", 32400, 0);//set up to Japan

const char* ssid = "your ssid";
const char* password = "your password";

//Your Domain name with URL path or IP address with path
const char* serverName = "url(where to post)";
unsigned long long lastTime = 0;
unsigned long long timerDelay = 3600000; //1hour in ms

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  Sensor.Begin();//sensor starts
  timeClient.begin();
 
  Serial.println("After the first loop, the next will start in 1hour(set in timerDelay)");
}

void loop() {
  //The token for the first loop
  static bool onceflag=true;//NEVER EVER EVER ERASE!!!!
  
  if ((millis() - lastTime) > timerDelay || onceflag) {
    onceflag = false;// NEVER ERASE!!!!!If it gets erased, http post will happen numerous time

    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      //WiFiClient client;
      HTTPClient http;
    
      // connecting to the server
      http.begin(serverName);

      Sensor.UpdateData();
      timeClient.update();
      float temperature, humidity;
      String acquisition_time;
      String formattedDate;
      String dayStamp;
      String timeStamp;
      
      temperature = Sensor.GetTemperature();
      humidity = Sensor.GetRelHumidity();
      formattedDate = timeClient.getFormattedDate(); //use NTPclient of version in 2017, the latest one doesn't have this 
      
      int splitT = formattedDate.indexOf("T");
      dayStamp = formattedDate.substring(0, splitT);
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
      
      acquisition_time = dayStamp + " " + timeStamp;
      
      Serial.print("Acquisition_time: ");
      Serial.println(acquisition_time);
      Serial.print("Temperature: ");
      Serial.print(temperature);  
      Serial.print("\xC2\xB0"); //The Degree symbol
      Serial.println("C");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%");      
      String postdata = String("{\"acquisition_time\":") + "\"" + acquisition_time + "\"" + ",\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
      
      http.addHeader("x-api-key", "put api key here");//For those who use AWS
      //the header if sending json file
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Content-Length", String(postdata.length()));
      
      int httpResponseCode = http.POST(postdata);
      Serial.println(postdata);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
