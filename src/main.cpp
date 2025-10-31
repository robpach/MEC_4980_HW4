#include <WiFi.h>

// pin definitions
int totalSteps = 200;
int hourStep = 9;
int hourDir = 10;
int minuteStep = 11;
int minuteDir = 12;

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
  pinMode(13, OUTPUT);

  WiFi.begin("NSA Security Van HQ", "windowstothehallway");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void StepHour(int amount, int dir)
{
  for (int i = 1; i <= amount; i++)
  {
    digitalWrite(13,HIGH);
    digitalWrite(hourDir, dir);
    digitalWrite(hourStep, HIGH);
    delay(stepInterval);
    digitalWrite(hourStep, LOW);
    delay(stepInterval);
    digitalWrite(13,LOW);
    if (digitalRead(hourDir) == 1)
    {
      hourSteps++;
    }
    else
    {
      hourSteps--;
    }
  }

  if ((hourSteps % 200) == 0)
  {
    hourSteps = 0;
  }
}

void StepMinute(int amount, int dir)
{
  for (int i = 1; i <= amount; i++)
  {
    digitalWrite(13, HIGH);
    digitalWrite(minuteDir, dir);
    digitalWrite(minuteStep, HIGH);
    delay(stepInterval);
    digitalWrite(minuteStep, LOW);
    delay(stepInterval);
    digitalWrite(13, LOW);
    if (digitalRead(minuteDir) == 1)
    {
      minuteSteps++;
    }
    else
    {
      minuteSteps--;
    }
  }

  if ((minuteSteps % 200) == 0)
  {
    minuteSteps = 0;
  }
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
  desiredHourStep = round((adjustedHour * 200.0) / 12.0);
  if (hourSteps < desiredHourStep)
  {
    StepHour(1, 1);
  }
  else if (hourSteps > desiredHourStep)
  {
    StepHour(1, -1);
  }

  // Minute Movement
  desiredMinuteStep = round((minute * 200.0) / 60.0); // replace minute with seconds to test
  if (minuteSteps < desiredMinuteStep)
  {
    StepMinute(1, 1);
  }
  else if (minuteSteps > desiredMinuteStep)
  {
    StepMinute(1, -1);
  }
}
