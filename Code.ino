//Including the libraries
#include <WiFi.h>
#include <Gesture.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <time.h>

//Definitions for DHT11 sensor
#define DHTPIN D1
#define DHTTYPE DHT11

//Your WiFi SSID and Password
const char* ssid = "SSID";
const char* password = "PASSWORD";

//Initialize the variables
int timeZoneIndex = 0;
int localTimeZoneIndex = 0;
int clockMode = 0;
String gesture = "";
byte prevSec = 0;
bool alarmStatus = false;
byte alarmHour = 0;
byte alarmMin = 0;
byte settingAlarm = 0;
byte timeHour = 0;
byte timeMin = 0;
const String enterExitModeGesture = "1 finger push";
const String alarmOffGesture = "5 finger";
const String toggleBacklightGesture = "4 finger push";
const String toggleSilentModeGesture = "Pinch";
byte worldTimeHour = 0;
byte worldTimeMin = 0;
unsigned long prevMillis = 0;
unsigned long prevMillis2 = 0;
byte worldTimeSec = 0;
String date = "";
String month = "";
String year = "";
long silentMillis = 0;
float temperature = 0.0;
float humidity = 0.0;
bool backlightOn = true;
bool silent = false;

pag7660_gesture_t result;

String timeZones[] = {
  "Etc/GMT", "Etc/UTC", "America/Guayaquil", "Europe/Athens", "Africa/Cairo", "Africa/Nairobi", "Europe/Paris", "Asia/Kolkata", "Europe/London", "Asia/Tokyo", "Australia/Darwin", "Australia/Sydney", "America/St_Johns", "Pacific/Apia", "Pacific/Honolulu", "America/Halifax", "America/Los_Angeles", "America/Denver", "America/Chicago", "America/New_York", "America/Puerto_Rico", "America/Sao_Paulo", "Africa/Maputo"
};

String months[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


//Define the objects
pag7660 Gesture;
LiquidCrystal_I2C lcd(0x27, 16, 2);
HTTPClient http;
DHT dht(DHTPIN, DHTTYPE);

void setup(){

  Serial.begin(9600);

  //Initialize the lcd
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Electronics");
  lcd.setCursor(0, 1);
  lcd.print("Champ");
  delay(3000);
  lcd.clear();

  //Set pin modes
  pinMode(21, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D2, OUTPUT);
  
  //Connect to WiFi
  Serial.print("Connecting to WiFi");
  delay(1000);
  WiFi.begin(ssid, password);
  delay(100);
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi");
  byte tryToConnect = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tryToConnect++;
    if (tryToConnect > 15){
      tryToConnect = 0;
      WiFi.begin(ssid, password);
    }
  }

  lcd.clear();
  lcd.print("WiFi connected");
  delay(2000);
  lcd.clear();

  //Initialize Gesture Sensor
  for (int i = 0; i < 5; i++){
    if (Gesture.init()) {
      Serial.println("PAG7660 initialization complete");
      break;
    } 
    else {
      Serial.println("PAG7660 initialization failed. Retrying...");
    }
    delay(2000);
  }

  setLocalTime();
  digitalWrite(21, LOW);
  prevMillis2 = millis();
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

}











void loop(){

  if (Gesture.getResult(result)) {
    getGesture(result);
  }

  if (gesture.equals("left") && clockMode < 4) {
    clockMode++;
    gesture = "";
    sound();
  }
  else if (gesture.equals("left") && clockMode == 4){
    clockMode = 0;
    gesture = "";
    sound();
  }

  if (gesture.equals("right") && clockMode > 0) {
    clockMode--;
    gesture = "";
    sound();
  }
  else if (gesture.equals("right") && clockMode == 0){
    clockMode = 4;
    gesture = "";
    sound();
  }

  if (gesture.equals(toggleBacklightGesture)) {
    backlightOn = !backlightOn;
    backlightOn ? lcd.backlight() : lcd.noBacklight();
    gesture = "";
    sound();
  }

  if (gesture.equals(toggleSilentModeGesture) && silentMillis+3000 < millis()) {
    silent = !silent;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Silent: ");
    lcd.setCursor(8, 0);
    lcd.print(((silent) ? "ON" : "OFF"));
    delay(5000);
    lcd.clear();
    gesture = "";
    sound();
    silentMillis = millis();
  }
  else if (gesture.equals(toggleSilentModeGesture)){
    gesture = "";
  }

  localTimeMode();
  alarmMode();
  worldClockMode();
  weatherMode();
  editLocalTime();

  delay(300);

  if (timeHour == alarmHour && timeMin == alarmMin && alarmStatus){
    turnOnAlarm();
  }

  if (prevMillis2+5000 < millis()) {
    prevMillis2 = millis();
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
  }

}







// This function is used to edit the local time
void editLocalTime(){

  if (clockMode == 4) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit Local Time");

    if (Gesture.getResult(result)) {
      getGesture(result);
    }

    if (gesture.equals(enterExitModeGesture)) {
      sound();
      lcd.clear();
      gesture = "";
      setLocalTime();
    }

  }

}








// This function is used to display the local time
void localTimeMode(){

  if (clockMode == 0) {
    
    String time = getTime();

    byte sec = time.substring(time.length() - 2).toInt();
    
    if (sec != prevSec) {
      prevSec = sec;
      lcd.clear();
      lcd.setCursor(0, 0);
      // lcd.print(time.substring(0, time.indexOf('|')));
      lcd.print(time.substring(0, 2));
      lcd.print(" " + months[time.substring(3, 5).toInt()-1] + " ");
      lcd.print(time.substring(6, 10));
      lcd.setCursor(0, 1);
      lcd.print(time.substring(time.indexOf('|')+1));
    }

    timeHour = time.substring(time.indexOf('|')+1, time.indexOf('|')+3).toInt();
    timeMin = time.substring(time.indexOf('|')+4, time.indexOf('|')+6).toInt();

  }

}







// This function is used to set alarms
void alarmMode() {

  while (clockMode == 1) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm: ");
    lcd.print((alarmStatus) ? "ON" : "OFF");

    if (alarmStatus) {
      lcd.setCursor(0, 1);
      lcd.print(((alarmHour < 10) ? ("0" + String(alarmHour)) : alarmHour) + ":" + ((alarmMin < 10) ? ("0" + String(alarmMin)) : alarmMin));
    }

    if (Gesture.getResult(result)) {
      getGesture(result);
    }

    if (gesture.equals("left")) {
      clockMode = 2;
      gesture = "";
      sound();
      break;
    }

    else if (gesture.equals("right")) {
      clockMode = 0;
      gesture = "";
      sound();
      break;
    }

    else if (gesture.equals(enterExitModeGesture)) { 

      gesture = "";
      sound();
      delay(1000);

      while (!gesture.equals(enterExitModeGesture)){

        delay(200);

        if (Gesture.getResult(result)) {
          getGesture(result);
        }

        if (gesture.equals("left") || gesture.equals("right")){
          alarmStatus = !alarmStatus;
          gesture = "";
          sound();
          delay(500);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Alarm Status: ");
        lcd.setCursor(0, 1);
        lcd.print((alarmStatus) ? "ON" : "OFF");

      }

      gesture = "";
      delay(1000);

      while (!gesture.equals(enterExitModeGesture) && alarmStatus){
        
        gesture = "";

        if (Gesture.getResult(result)) {
          getGesture(result);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Set Alarm: ");
        lcd.print((settingAlarm == 0) ? "Hour" : "Min");
        lcd.setCursor(0, 1);
        lcd.print(((alarmHour < 10) ? ("0" + String(alarmHour)) : alarmHour) + ":" + ((alarmMin < 10) ? ("0" + String(alarmMin)) : alarmMin));

        byte delayTime = 100;

        if (gesture.equals("cw")){
          gesture = "";
          if (settingAlarm == 0){
            alarmHour++;
            delay(delayTime);
            if (alarmHour > 23){
              alarmHour = 0;
            }
            sound();
          }
          else {
            alarmMin++;
            delay(delayTime);
            if (alarmMin > 59){
              alarmMin = 0;
            }
            sound();
          }
          gesture = "";
        }
        else if (gesture.equals("ccw")){
          gesture = "";
          if (settingAlarm == 0){
            alarmHour--;
            delay(delayTime);
            if (alarmHour > 23){
              alarmHour = 23;
            }
            sound();
          }
          else {
            alarmMin--;
            delay(delayTime);
            if (alarmMin > 59){
              alarmMin = 59;
            }
            sound();
          }
          gesture = "";
        }
        
        delay(200);

        if (gesture.equals("left")) {
          settingAlarm = 1;
          gesture = "";
          sound();
        }
        else if (gesture.equals("right")) {
          settingAlarm = 0;
          gesture = "";
          sound();
        }

      }

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Alarm set");
      lcd.setCursor(0, 1);
      lcd.print("successfully!");
      sound();
      delay(3000);
      lcd.clear();
      settingAlarm = 0;

      if (alarmStatus) {
      
        lcd.setCursor(0, 0);
        lcd.print("Alarm: ");
        lcd.print(((alarmHour < 10) ? ("0" + String(alarmHour)) : alarmHour) + ":" + ((alarmMin < 10) ? ("0" + String(alarmMin)) : alarmMin));
        delay(3000);
        lcd.clear();

      }

    }

    gesture = "";
    delay(200);

  }

}





// This function is used to display the world clock
void worldClockMode() {

  if (clockMode == 2) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("World Clock Mode");

    if (Gesture.getResult(result)) {
      getGesture(result);
    }

    if (gesture.equals(enterExitModeGesture)) {
      
      gesture = "";
      sound();
      delay(500);

      while (!gesture.equals(enterExitModeGesture)) {

        if (timeHour == alarmHour && timeMin == alarmMin && alarmStatus){
          turnOnAlarm();
        }

        gesture = "";

        if (Gesture.getResult(result)) {
          getGesture(result);
        }

        if (gesture.equals("right")){
          timeZoneIndex--;
          gesture = "";
          if (timeZoneIndex < 0)
            timeZoneIndex = 22;
          getTimeFromApi(timeZoneIndex);
          sound();
        } 

        else if (gesture.equals("left")){
          timeZoneIndex++;
          gesture = "";
          if (timeZoneIndex > 22)
            timeZoneIndex = 0;
          getTimeFromApi(timeZoneIndex);
          sound();
        }

        String timeZoneWorldClock = timeZones[timeZoneIndex].substring(timeZones[timeZoneIndex].indexOf("/")+1);
        timeZoneWorldClock.replace("_", " ");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(timeZoneWorldClock);
        lcd.setCursor(13, 0);
        lcd.print(month);
        lcd.setCursor(0, 1);
        lcd.print(((worldTimeHour < 10) ? "0" + String(worldTimeHour) : worldTimeHour) + ":" + ((worldTimeMin < 10) ? "0" + String(worldTimeMin) : worldTimeMin) + ":" + ((worldTimeSec < 10) ? "0" + String(worldTimeSec) : worldTimeSec));
        lcd.print(" " + date + "-" + year);

        delay(200);
        updateWorldTime(200);

      }

      gesture = "";
      sound();
      delay(200);

    }

  }

}







// This function is used to display the weather
void weatherMode() {

  if (clockMode == 3) {
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");

    if (timeHour == alarmHour && timeMin == alarmMin && alarmStatus){
      turnOnAlarm();
    }

  }

}









// This function is used to update time of other locations
void updateWorldTime (int loopDelay) {

  if (worldTimeSec > 57) {
    delay(3000);
    getTimeFromApi(timeZoneIndex);
  }

  if ((prevMillis + (1000 - (loopDelay+25))) < millis()) {
    prevMillis = millis();
    worldTimeSec++;
  }

}









// This function is used to turn on the alarm buzzer
void turnOnAlarm() {

  alarmStatus = false;

  lcd.setCursor(9, 1);
  lcd.print("Alarm");
  
  while (!gesture.equals(alarmOffGesture)) {

    gesture = "";
    
    if (Gesture.getResult(result)) {
      getGesture(result);
    }

    for (int i = 0; i < 4; i++) {
      digitalWrite(21, HIGH);
      digitalWrite(D2, HIGH);
      delay(80);
      digitalWrite(21, LOW);
      digitalWrite(D2, LOW);
      delay(100);
    }

    delay(700);

  }

  digitalWrite(21, LOW);
  digitalWrite(D2, LOW);

}









// This function is used to fetch time from the internet
String getTimeFromApi(int index) {

  String apiUrl = "https://timeapi.io/api/Time/current/zone?timeZone=";
  apiUrl.concat(timeZones[index]);
  http.begin(apiUrl); //Specify the API
  int httpCode = http.GET(); //Make the request

  if (httpCode > 0) { //Check for the returning code
    String payload = http.getString();
    payload.replace("\"", "");
    month = months[payload.substring(payload.indexOf("month")).substring(6, payload.indexOf(",")-2).toInt() - 1];
    payload = payload.substring(payload.indexOf("dateTime:")+9, payload.indexOf("dateTime:")+28);
    http.end(); //Free the resources
    worldTimeHour = payload.substring(payload.indexOf("T")+1, payload.indexOf(":")).toInt();
    worldTimeMin = payload.substring(payload.indexOf(":")+1, payload.indexOf(":")+3).toInt();
    worldTimeSec = payload.substring(payload.indexOf(":")+4, payload.indexOf(":")+6).toInt();
    payload.replace("T", " ");
    year = payload.substring(0, 4);
    date = payload.substring(8, 10);
    return payload;
  }

  else {
    http.end(); //Free the resources
    lcd.clear();
    lcd.print("Updating time...");
    delay(1000);
    return getTimeFromApi(timeZoneIndex);
  }

}










// This function is used to set the local time
void setLocalTime(){

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select your");
  lcd.setCursor(0, 1);
  lcd.print("Time Zone");
  delay(3000);
  lcd.clear();

  while (!gesture.equals(enterExitModeGesture)) {

    if (Gesture.getResult(result)) {
      getGesture(result);
    }

    if (gesture.equals("right")){
      timeZoneIndex--;
      sound();
      gesture = "";
      if (timeZoneIndex < 0)
        timeZoneIndex = 22;
    } 
    else if (gesture.equals("left")){
      timeZoneIndex++;
      sound();
      gesture = "";
      if (timeZoneIndex > 22)
        timeZoneIndex = 0;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    String tempTimeZone = timeZones[timeZoneIndex].substring(timeZones[timeZoneIndex].indexOf("/")+1);
    tempTimeZone.replace("_", " ");
    lcd.print(tempTimeZone);
    delay(200);

  }

  lcd.clear();
  lcd.setCursor(0, 0);
  sound();
  lcd.print("Time Zone set");
  lcd.setCursor(0, 1);
  lcd.print("successfully!");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fetching time...");
  gesture = "";
  setTime(getTimeFromApi(timeZoneIndex));
  Serial.println(getTime());
  localTimeZoneIndex = timeZoneIndex;

}




// This function is used to set the local time in the ESP's memory
void setTime(String dateTime) {

  struct tm t;
  strptime(dateTime.c_str(), "%Y-%m-%d %H:%M:%S", &t);
  time_t t_of_day = mktime(&t);
  timeval now = { .tv_sec = t_of_day };
  settimeofday(&now, NULL);

}






// This function is used to get the local time from the ESP's memory
String getTime() {

  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%d-%m-%Y|%H:%M:%S", &timeinfo);
  return String(timeStr);

}








// This function is used to get the gesture from the sensor
void getGesture(const pag7660_gesture_t& result) {

  const char* cursor_str[] = {
    NULL,
    "Tap",
    "Grab",
    "Pinch",
  };
  
  switch (result.type) {

    case 0:
      switch (result.cursor.type) {
        case 1:
        case 2:
        case 3:
          if (result.cursor.select)
            Serial.println(cursor_str[result.cursor.type]);
            gesture = cursor_str[result.cursor.type];
          break;
        default:
          break;
      }
      break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
      switch (result.type){

        case 1:
          Serial.println(1);
          gesture = "1 finger";
          break;

        case 2:
          Serial.println(2);
          gesture = "2 finger";
          break;

        case 3:
          Serial.println(3);
          gesture = "3 finger";
          break;

        case 4:
          Serial.println(4);
          gesture = "4 finger";
          break;

        case 5:
          Serial.println(5);
          gesture = "5 finger";
          break;

      }
      break;

    case 6:
      Serial.print("Rotate Right ");
      Serial.println(result.rotate);
      gesture = "cw";
      break;

    case 7:
      Serial.print("Rotate Left ");
      Serial.println(result.rotate);
      gesture = "ccw";
      break;

    case 8:
      Serial.println("Swipe Left");
      gesture = "left";
      break;

    case 9:
      Serial.println("Swipe Right");
      gesture = "right";
      break;

    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
      gesture = String(result.type - 19 + 1) + " finger push";
      Serial.print(result.type - 19 + 1);
      Serial.println("-finger push");
      break;
      
    default:
      break;
  }

}

// This function is used to make a sound when a gesture is detected
void sound(){

  if (!silent) {
    digitalWrite(D2, HIGH);
    delay(20);
    digitalWrite(D2, LOW);
  }

}
