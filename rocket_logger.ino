#include <Arduino_BMI270_BMM150.h>
#include <Arduino_HS300x.h>
#include <SparkFun_LIS331.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS_PIN  10
#define SAMPLE_MS  100

LIS331 highG;

float vx = 0.0f, vy = 0.0f, vz = 0.0f;
float hvx = 0.0f, hvy = 0.0f, hvz = 0.0f;
unsigned long lastTime = 0;

File logFile;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Wire.begin();

  if (!IMU.begin()) { Serial.println("[ERROR] IMU init failed"); halt(); }
  Serial.println("[OK] BMI270 ready");

  if (!HS300x.begin()) { Serial.println("[ERROR] HS300x init failed"); halt(); }
  Serial.println("[OK] HS300x ready");

  highG.setI2CAddr(0x18);
  if (!highG.begin(LIS331::USE_I2C)) {
    Serial.println("[ERROR] H3LIS331DL init failed");
    halt();
  }
  highG.setFullScale(LIS331::HIGH_RANGE); // ±400g
  highG.setODR(LIS331::DR_1000HZ);
  Serial.println("[OK] H3LIS331DL ready (±400g)");

  if (!SD.begin(SD_CS_PIN)) { Serial.println("[ERROR] SD init failed"); halt(); }
  Serial.println("[OK] SD ready");

  char filename[16];
  for (int i = 0; i < 100; i++) {
    snprintf(filename, sizeof(filename), "data%02d.csv", i);
    if (!SD.exists(filename)) break;
  }

  logFile = SD.open(filename, FILE_WRITE);
  if (!logFile) { Serial.println("[ERROR] File open failed"); halt(); }

  logFile.println("time_ms,"
                  "ax_g,ay_g,az_g,"
                  "gx_dps,gy_dps,gz_dps,"
                  "vx_ms,vy_ms,vz_ms,speed_ms,"
                  "hax_g,hay_g,haz_g,"
                  "hspeed_ms,"
                  "temp_C,humidity_pct");
  logFile.flush();

  Serial.print("[OK] Logging to: "); Serial.println(filename);
  lastTime = millis();
}

void loop() {
  static unsigned long lastSample = 0;
  unsigned long now = millis();
  if (now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  float ax, ay, az, gx, gy, gz;
  if (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) return;
  IMU.readAcceleration(ax, ay, az);
  IMU.readGyroscope(gx, gy, gz);

  int16_t hx_raw, hy_raw, hz_raw;
  highG.readAxes(hx_raw, hy_raw, hz_raw);
  float hax = highG.convertToG(400, hx_raw);
  float hay = highG.convertToG(400, hy_raw);
  float haz = highG.convertToG(400, hz_raw);

  float temp     = HS300x.readTemperature();
  float humidity = HS300x.readHumidity();

  float dt = (now - lastTime) / 1000.0f;
  lastTime = now;

  vx  += ax  * 9.80665f * dt;
  vy  += ay  * 9.80665f * dt;
  vz  += az  * 9.80665f * dt;
  hvx += hax * 9.80665f * dt;
  hvy += hay * 9.80665f * dt;
  hvz += haz * 9.80665f * dt;

  float speed  = sqrtf(vx*vx   + vy*vy   + vz*vz);
  float hspeed = sqrtf(hvx*hvx + hvy*hvy + hvz*hvz);

  logFile.print(now);        logFile.print(',');
  logFile.print(ax, 4);      logFile.print(',');
  logFile.print(ay, 4);      logFile.print(',');
  logFile.print(az, 4);      logFile.print(',');
  logFile.print(gx, 4);      logFile.print(',');
  logFile.print(gy, 4);      logFile.print(',');
  logFile.print(gz, 4);      logFile.print(',');
  logFile.print(vx, 4);      logFile.print(',');
  logFile.print(vy, 4);      logFile.print(',');
  logFile.print(vz, 4);      logFile.print(',');
  logFile.print(speed, 4);   logFile.print(',');
  logFile.print(hax, 4);     logFile.print(',');
  logFile.print(hay, 4);     logFile.print(',');
  logFile.print(haz, 4);     logFile.print(',');
  logFile.print(hspeed, 4);  logFile.print(',');
  logFile.print(temp, 2);    logFile.print(',');
  logFile.println(humidity, 2);

  static uint8_t flushCnt = 0;
  if (++flushCnt >= 10) { logFile.flush(); flushCnt = 0; }

  Serial.print("t="); Serial.print(now);
  Serial.print(" BMI=["); Serial.print(ax,2);
  Serial.print(","); Serial.print(ay,2);
  Serial.print(","); Serial.print(az,2);
  Serial.print("] H3LIS=["); Serial.print(hax,1);
  Serial.print(","); Serial.print(hay,1);
  Serial.print(","); Serial.print(haz,1);
  Serial.print("] spd="); Serial.print(speed,3);
  Serial.print(" T="); Serial.print(temp,1);
  Serial.print(" H="); Serial.println(humidity,1);
}

void halt() {
  while (1) { digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); delay(200); }
}