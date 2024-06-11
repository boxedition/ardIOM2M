#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <WiFiS3.h>
#include "arduino_secrets.h"
#include "vars.h"

const char *serverAddress = "m2m.boxdev.site";
const int port = 80;
const char *resource = "/cse-in";
const char *ae = "AirTemp"; // AE Name

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

// PROTO
void connectToWiFi();
float readSensor();
void createAE();
void createContainer(String name);
void makeHTTPRequest(String payload, String resource, String type, String origin);
void updateCIN(String container, String value);

void setup()
{
  Serial.begin(9600); // Initialize serial communication at a baud rate of 115200
  delay(1024);        // Pause program execution for 1024 milliseconds

  // Connect to WiFi network
  connectToWiFi();

  // Create Application Entity (AE)
  createAE();

  // Loop to create multiple containers
  for (int i = 0; i < nContainers; i++)
  {
    // Serial.println("Processing Container: " + containers[i]);
    createContainer(containers[i]);
  }
}

void loop()
{

  // Loop to create multiple containers
  for (int i = 0; i < nContainers; i++)
  {
    float sensorData = readSensor();
    updateCIN(containers[i], String(sensorData));
  }

  delay(2500);
}

void connectToWiFi()
{
  Serial.println("Connecting to WiFi...");

  for (int i = 0; i < numNetworks; ++i)
  {
    WiFi.begin(knownNetworks[i][0], knownNetworks[i][1]);
    Serial.print("Attempting to connect to ");
    Serial.println(knownNetworks[i][0]);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10)
    {
      delay(250);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\nWiFi connected");
      return;
    }
    else
    {
      Serial.println("\nConnection failed");
    }
  }

  Serial.println("Unable to connect to any known networks");
}

void createAE()
{
  Serial.println("Creating Application Entity (AE)");
  String apiName = 'N' + String(ae);
  String origin = "CAdmin" + String(ae);
  String aePayload = "{\"m2m:ae\": {\"rn\": \"" + String(ae) + "\", \"api\": \"" + apiName + "\", \"rr\": true, \"srv\": [\"3\"]}}";
  makeHTTPRequest(aePayload, resource, "2", origin);
}

void createContainer(String name)
{
  String cntPayload = "{\"m2m:cnt\": {\"rn\": \"" + name + "\"}}";
  String origin = "CAdmin";
  String path = String(resource) + "/" + String(ae);
  Serial.println(path);
  makeHTTPRequest(cntPayload, path, "3", origin);
}

void updateCIN(String container, String value)
{
  String cntPayload = "{\"m2m:cin\": {\"con\": \"" + value + "\",\"cnf\": \"text/plain:0\",\"lbl\": [\"" + container + "\",\"" + String(ae) + "\"]}}";
  String origin = "CAdmin";
  String path = String(resource) + "/" + String(ae) + "/" + String(container);
  makeHTTPRequest(cntPayload, path, "4", origin);
}

void makeHTTPRequest(String payload, String path, String type, String origin)
{
  String content = "application/json;ty=" + String(type);
  Serial.print("\nPayload: " + String(payload));
  Serial.print("\nContent-Type: " + String(content));
  Serial.print("\nX-M2M-Origin: " + String(origin));

  client.beginRequest();
  client.post(path);
  client.sendHeader("Accept", "application/json");
  client.sendHeader("Content-Type", content);
  client.sendHeader("X-M2M-Origin", origin);
  client.sendHeader("X-M2M-RI", "123");
  client.sendHeader("X-M2M-RVI", "3");
  client.sendHeader("Content-Length", payload.length());
  client.beginBody();
  client.print(payload);
  client.endRequest();

  String responseBody = client.responseBody();
  Serial.print("Response body: ");
  Serial.println(responseBody);
}

float readSensor()
{
  return random(0, 100);
}