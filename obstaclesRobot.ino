#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttUsername = "";
const char* mqttPassword = "";
const char* controlTopic = "robot/control" ;
const char* ultrasonicTopic = "robot/distance" ;
const char* infraredTopic = "robot/infrared";

bool forward = false, backward = false, right = false, left = false;

WiFiClient espClient;
PubSubClient client(espClient);

int trigPin = 32; // trig pin of HC-SR04
int echoPin = 35; // Echo pin of HC-SR04
int infraredPin = 33; // Pin connected to the infrared sensor
int fwd1 = 14; // Forward motion of Left motor
int rev2 = 12; // Reverse motion of Left motor
int fwd3 = 26; // Forward motion of Right motor
int rev4 = 25; // Reverse motion of Right motor
int enable1PinA = 13;
int enable1PinB = 27;
int infraredValue;
long duration;
int distance;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void readUltrasonicSensor()
{
  // send ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // calculate distance
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
  if (distance > 300 && duration < 1196)
  {
    distance = 300;
  }
  else if (distance == 1195)
  {
    distance = 0;
  }
  // wait a short time before sending next pulse
  delay(100);
}

void readInfraredSensor()
{
  infraredValue = map(digitalRead(infraredPin), 0, 1, 1, 0); // Read IR sensor value
  Serial.println(infraredValue);    // Print IR sensor value to serial monitor
  delay(100);    // Wait for 100 milliseconds before reading again
}

void moveForward()
{
  digitalWrite(fwd1, LOW);
  digitalWrite(rev2, HIGH);
  digitalWrite(fwd3, HIGH);
  digitalWrite(rev4, LOW);

  Serial.println("FORWARD");
}

void moveBackward()
{

  digitalWrite(fwd1, HIGH);
  digitalWrite(rev2, LOW);
  digitalWrite(fwd3, LOW);
  digitalWrite(rev4, HIGH);

  Serial.println("BACKWARD");
}

void moveRight()
{
  digitalWrite(fwd1, HIGH);
  digitalWrite(rev2, LOW);
  digitalWrite(fwd3, HIGH);
  digitalWrite(rev4, LOW);
  Serial.println("RIGHT");
}

void moveLeft()
{

  digitalWrite(fwd1, LOW);
  digitalWrite(rev2, HIGH);
  digitalWrite(fwd3, LOW);
  digitalWrite(rev4, HIGH);
  Serial.println("LEFT");
}

void stop()
{
  digitalWrite(fwd1, LOW);
  digitalWrite(rev2, LOW);
  digitalWrite(fwd3, LOW);
  digitalWrite(rev4, LOW);
}

void publishData()
{
  readUltrasonicSensor();
  Serial.println("Publishing message: ");

  char ultrasonicPayload[10];
    sprintf (ultrasonicPayload, "%d",distance);
    client.publish(ultrasonicTopic, ultrasonicPayload);
   Serial.println("Distance:");
   Serial.println(ultrasonicPayload);
}

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char *topic, byte *payload, unsigned int length)
{
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1')
  {
    moveForward();
  }
  else if ((char)payload[0] == '2')
  {
    moveBackward();
  }
  else if ((char)payload[0] == '3')
  {
    moveRight();
  }
  else if ((char)payload[0] == '4')
  {
    moveLeft();
  }
  else if ((char)payload[0] == '5')
  { 
    stop();
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "MobileRobot";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe(controlTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(fwd1, OUTPUT);
  pinMode(rev2, OUTPUT);
  pinMode(fwd3, OUTPUT);
  pinMode(rev4, OUTPUT);

  pinMode(enable1PinA, OUTPUT);
  pinMode(enable1PinB, OUTPUT);

  digitalWrite(enable1PinA,HIGH);
  digitalWrite(enable1PinB,HIGH);

  pinMode(infraredPin, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

 
  Serial.begin(115200);
  Serial.println("Obstacle Avoiding Robot Using Ultrasonic V2");
  delay(random(500, 2000)); // delay for random time
  setup_wifi();
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
}


void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
  }

  publishData();
}