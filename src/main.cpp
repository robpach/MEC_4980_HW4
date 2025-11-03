#include <WiFi.h>

// pin definitions
int totalSteps = 200;
int hourStep = 9;
int hourDir = 10;
int minuteStep = 11;
int minuteDir = 12;
int hourLimit = 5;
int minuteLimit = 6; // change if necessary
bool hourLimitState = false;
bool minuteLimitState = false;

int hourSteps = 0;
int minuteSteps = 0;
int extraSteps = 0;
int stepInterval = 1;
int desiredHourStep;
int desiredMinuteStep;

const char *ntpServer = "us.pool.ntp.org";
const long gmtOffset_sec = -3600 * 6;
const int daylightOffset_sec = 3600;
struct tm timeinfo;
int hour;
float adjustedHour;
float minute;
int seconds;
unsigned long lastUpdate = 0;

void StepHour(int amount, int dir)
{
  for (int i = 1; i <= amount; i++)
  {
    digitalWrite(hourDir, dir);
    digitalWrite(hourStep, HIGH);
    delay(stepInterval);
    digitalWrite(hourStep, LOW);
    delay(stepInterval);
    if (dir == 1)
    {
      hourSteps++;
    }
    else
    {
      hourSteps--;
    }
  }

  hourSteps = hourSteps % 200;
}

void StepMinute(int amount, int dir)
{
  for (int i = 1; i <= amount; i++)
  {
    digitalWrite(minuteDir, dir);
    digitalWrite(minuteStep, HIGH);
    delay(stepInterval);
    digitalWrite(minuteStep, LOW);
    delay(stepInterval);
    if (dir == 1)
    {
      minuteSteps++;
    }
    else
    {
      minuteSteps--;
    }
  }

  minuteSteps = minuteSteps % 200;
}

// Homing Function
void Home()
{
  hourLimitState = digitalRead(hourLimit);
  minuteLimitState = digitalRead(minuteLimit);

  while (!hourLimitState)
  {
    hourLimitState = digitalRead(hourLimit);
    StepHour(1,1);
    delay(20);
  }
  hourSteps = 0;

  while (!minuteLimitState)
  {
    minuteLimitState = digitalRead(minuteLimit);
    StepMinute(1,1);
    delay(20);
  }
  minuteSteps = 0;
}


void setup()
{
  delay(500);
  Serial.begin(9600);
  delay(500);
  pinMode(hourStep, OUTPUT);
  pinMode(hourDir, OUTPUT);
  digitalWrite(hourDir, HIGH);
  pinMode(minuteStep, OUTPUT);
  pinMode(minuteDir, OUTPUT);
  digitalWrite(minuteDir, HIGH);
  pinMode(hourLimit, INPUT_PULLDOWN);
  pinMode(minuteLimit, INPUT_PULLDOWN);

  WiFi.begin("NSA Security Van HQ", "windowstothehallway");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Create a homing function
  // Home();
}


void loop()
{
  if (millis() - lastUpdate >= 1000)
  {
    lastUpdate = millis();

    getLocalTime(&timeinfo);
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    seconds = timeinfo.tm_sec;

    if (minute < 10.0)
    {
      Serial.printf("%i:%i%i:%i\n", (int)hour, 0, (int)minute, seconds);
    }
    else if (minute >= 10.0)
    {
      Serial.printf("%i:%i:%i\n", (int)hour, (int)minute, seconds);
    }
  }

  // Hour Movement
  adjustedHour = (float)(hour % 12);
  desiredHourStep = (int)(200 - round((adjustedHour * 200.0) / 12.0)) % 200;
  int diffHour = (desiredHourStep - hourSteps + 200) % 200;
  if (diffHour == 0)
  {
  }
  else if (diffHour < 100)
  {
    StepHour(1, 1);
  }
  else
  {
    StepHour(1, 0);
  }

  // Minute Movement
  desiredMinuteStep = (int)(200 - round((seconds * 200.0) / 60.0)) % 200; // replace minute with seconds to test
  int diffMin = (desiredMinuteStep - minuteSteps + 200) % 200;
  if (diffMin == 0)
  {
    // do nothing, already in place
  }
  else if (diffMin < 100)
  {
    StepMinute(1, 1); // move forward
  }
  else
  {
    StepMinute(1, 0); // move backward
  }
}
