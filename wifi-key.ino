/*
---------------------------------------------------------------------------------------------------
This Sketch enables your board to act as wifi-lock.
It provides a WiFi hotspot and checks if a device's MAC-address is whitelisted.
You can modify it use your smartphone as a key to open and close your garage door.

Board: ESP8266
INSTRUCTION:
	- put allowed MAC-addresses in the array "macWhitelist"
	- connect your device to WiFi "ESP-Access" and enter the password "password"
Authors:
	- Manuel Kienlein	(@ManuelK2000)
	- Richard Lang		(@Michlbauer)
---------------------------------------------------------------------------------------------------
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
extern "C" {
  #include "user_interface.h"
}

// WiFi network name and password (minimum of 8 characters)
const char* ssid = "ESP-Access";
const char* password = "password";

int oldStatus = 0;
int Status = 0;  // 0= no connections, 1= connected, 2= client successful conected (mac checked)

// Whitelist of MAC-addresses
String macWhitelist[2] = {
	"40:45:AD:7D:BF:71",
	"BC:FE:D9:15:35:EB"
};

void setup() {
	Serial.begin(9600);
	
	// Set led pins as output
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(16, OUTPUT);
	
	// Start WiFi access point
	WiFi.softAP(ssid, password);
	
	Serial.println("----------------------------");
	Serial.print("WiFi name:   ");
	Serial.println(ssid);
	Serial.print("Router IP: ");
	Serial.println(WiFi.softAPIP()); 
	Serial.println("----------------------------");
	
	Serial.println("Setup completed");
}

void loop() {
	
	// Update status
	oldStatus = Status;
	Status = 0;
	
	// Print list of clients
	printConnectedClients();
	
	
	if(oldStatus != Status){
		if(Status > oldStatus){
			Serial.println("RISING EDGE");
		}
		if(Status < oldStatus){
			Serial.println("FALLEN EDGE");
		}
		
		// Do something when status has changed
		if(Status == 2){
		   Serial.println("logged in");
		}
		if(Status == 0){
		   Serial.println("logged out");
		}
	}
	
	/*
	*  Status 0: No connections
	*  Status 1: Client successful connected
	*  Status 2: Client contained in whitelist
	*/
	
	// Turn LED_BUILTIN on when MAC-address is whitelisted
	if(Status == 2){
		digitalWrite(LED_BUILTIN, LOW);
	}
	if(Status == 0){
		digitalWrite(LED_BUILTIN, HIGH);
	}
	
	
	// Turn led on GIPO 16 on, if device is connected
	if(Status == 0){
		digitalWrite(16, LOW);
	}else{
		digitalWrite(16, HIGH);
	}
	
	delay(1000);
}

boolean hasAccess(String mac){
	for (int i = 0; i < sizeof(macWhitelist); i = i + 1){
		if(mac == macWhitelist[i]){
			return true;
		}
	}
	return false;
}

void printConnectedClients(){
	Serial.println("-------- Network Client List --------"); 
	Serial.println("Connected clients: " + String(wifi_softap_get_station_num()));
	
	// Load station info
	struct station_info *station_list = wifi_softap_get_station_info();
	
	// If struct is not empty, output data
	while(station_list != NULL){
		
		// Determine MAC-address
		char client_mac[18] = {0};
		sprintf(client_mac, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station_list->bssid));
		
		// Determine IP-address
		String client_ip = IPAddress((&station_list->ip)->addr).toString();
		
		// Check if MAC is contained in whitelist
		bool access = hasAccess(String(client_mac));
		String access_message = (access)?"ACCESS GRANTED":"ACCESS DENIED";
		
		Status = 1;
		if(access == true){
			Status = 2;
		}
		
		// Output data
		Serial.println("MAC-address: " + String(client_mac) + " | IP-address: " + client_ip + " | Access: " + access_message);
		
		// Continue with next client
		station_list = STAILQ_NEXT(station_list, next);
		
	}
	
	// Reset station info
	wifi_softap_free_station_info();
	
	Serial.println("-------------------------------------");
	Serial.println();
	Serial.println();
}