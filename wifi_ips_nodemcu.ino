#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Set these to your desired credentials. */
const char *ss_id = "nodeB";  //ENTER YOUR WIFI SETTINGS
const char *password = "12345678";

HTTPClient http;    //Declare object of class HTTPClient

String knownSsid[4] = {"nodeA", "nodeB", "nodeC", "nodeD"}; 

//Web/Server address to read/write from 
String host = "http://13.233.254.201";

//String host = "http://192.168.43.231";


void setup() {
  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(115200);
  //disconnects any previous conn
  WiFi.disconnect();
  delay(1000);
  //scan for number of nearby networks & print SSIDs
  Serial.print("Nearby networks found  :");
  Serial.println(WiFi.scanNetworks());
  delay(500);
  
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ss_id, password);     //Connect to your WiFi router
  Serial.println("");
 
  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ss_id);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}



void loop() {
  
  String ssid, rssi;
  Serial.println("\nScanning networks…:");

  String* rssiArr = new String[getKnownSsidSize()];

  // Get count of all available WiFi networks
  int n = WiFi.scanNetworks();

  int knownSsidCount = 0;
  
  for (int i = 0; i < n; i++)
  {
    // Print the details on console
    Serial.print("SSID: ");
    Serial.print(WiFi.SSID(i));
    Serial.print(", RSSI: ");
    Serial.print(WiFi.RSSI(i));
    ssid=WiFi.SSID(i);
    rssi=WiFi.RSSI(i);
    
    int ssidPos = getKnownSsidPos(ssid);
    // If the SSID is a known SSID send the data to server
    if(ssidPos != -1){
      Serial.print(" --> KNOWN NETWORK");
      rssiArr[ssidPos] = rssi;
      knownSsidCount++;
      //sendDataToServer(ssid, rssi);
    }   
    Serial.println();
  }

  if(knownSsidCount >= 3) {
    sendDataToServer(rssiArr);
  }
  else {
    Serial.println("Known SSIDs < 3: No API call made");  
  }
  
  delay(5000);  //Post Data at every 5 seconds
}

/**
 * Method to send SSID and RSSI data to server
 */
void sendDataToServer(String* rssiArr){
  
    //String getData = "?ssid=" + ssid + "&rssi=" + rssi ;
    String getData = "?data=";
    
    String jsonArrStr = "[";
    for(int i=0; i < getKnownSsidSize(); i++){
        String jsonObj = "{\"ssid\":\""+ knownSsid[i] +"\",\"rssi\":\""+rssiArr[i]+"\"}";
        jsonArrStr += jsonObj;

        if(i != (getKnownSsidSize() - 1))
          jsonArrStr += ",";
    }

    jsonArrStr += "]";

    getData += jsonArrStr;

    String link= host + "/wifi/updateRssi2.php" + getData;         //Specify request destination
    Serial.println("url = "+link);
    Serial.println();
    http.begin(link);
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload
   
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
   
    http.end();  //Close connection
}

/**
 * Method to get position of known SSID if exist
 */
int getKnownSsidPos(String ssid){
    int position = -1;
    for(int i = 0; i < getKnownSsidSize(); i++){
       if(ssid == knownSsid[i]){
          position = i;
          break;
       }
    }
    return  position;
}


/**
 * Method to get known SSID array size
 */
int getKnownSsidSize(){
  return sizeof(knownSsid) / sizeof(knownSsid[0]);
}
