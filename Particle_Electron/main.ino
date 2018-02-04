#include "Particle.h"

// This #include statement was automatically added by the Particle IDE.
#include <SparkJson.h>

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

#include <string>

#include "cellular_hal.h"

#define DHTPIN D2
#define DHTTYPE DHT22

// POWERCHECK
#include "Particle.h"

/**
 * Simple class to monitor for has power (USB or VIN), has a battery, and is charging
 *
 * Just instantiate one of these as a global variable and call setup() out of setup()
 * to initialize it. Then use the getHasPower(), getHasBattery() and getIsCharging()
 * methods as desired.
 */
class PowerCheck {
public:

	PowerCheck();
	virtual ~PowerCheck();

	/**
	 * You must call this out of setup() to initialize the interrupt handler!
	 */
	void setup();

	/**
	 * Returns true if the Electron has power, either a USB host (computer), USB charger, or VIN power.
	 *
	 * Not interrupt or timer safe; call only from the main loop as it uses I2C to query the PMIC.
	 */
	bool getHasPower();

	/**
	 * Returns true if the Electron has a battery.
	 */
	bool getHasBattery();

	/**
	 * Returns true if the Electron is currently charging (red light on)
	 *
	 * Not interrupt or timer safe; call only from the main loop as it uses I2C to query the PMIC.
	 */
	bool getIsCharging();

private:
	void interruptHandler();

	PMIC pmic;
	volatile bool hasBattery = true;
	volatile unsigned long lastChange = 0;
};

PowerCheck::PowerCheck() {
}

PowerCheck::~PowerCheck() {
}

void PowerCheck::setup() {
	// This can't be part of the constructor because it's initialized too early.
	// Call this from setup() instead.

	// BATT_INT_PC13
	attachInterrupt(LOW_BAT_UC, &PowerCheck::interruptHandler, this, FALLING);
}

bool PowerCheck::getHasPower() {
	// Bit 2 (mask 0x4) == PG_STAT. If non-zero, power is good
	// This means we're powered off USB or VIN, so we don't know for sure if there's a battery
	byte systemStatus = pmic.getSystemStatus();
	return ((systemStatus & 0x04) != 0);
}

/**
 * Returns true if the Electron has a battery.
 */
bool PowerCheck::getHasBattery() {
	if (millis() - lastChange < 100) {
		// When there is no battery, the charge status goes rapidly between fast charge and
		// charge done, about 30 times per second.

		// Normally this case means we have no battery, but return hasBattery instead to take
		// care of the case that the state changed because the battery just became charged
		// or the charger was plugged in or unplugged, etc.
		return hasBattery;
	}
	else {
		// It's been more than a 100 ms. since the charge status changed, assume that there is
		// a battery
		return true;
	}
}


/**
 * Returns true if the Electron is currently charging (red light on)
 */
bool PowerCheck::getIsCharging() {
	if (getHasBattery()) {
		byte systemStatus = pmic.getSystemStatus();

		// Bit 5 CHRG_STAT[1] R
		// Bit 4 CHRG_STAT[0] R
		// 00 – Not Charging, 01 – Pre-charge (<VBATLOWV), 10 – Fast Charging, 11 – Charge Termination Done
		byte chrgStat = (systemStatus >> 4) & 0x3;

		// Return true if battery is charging if in pre-charge or fast charge mode
		return (chrgStat == 1 || chrgStat == 2);
	}
	else {
		// Does not have a battery, can't be charging.
		// Don't just return the charge status because it's rapidly switching
		// between charging and done when there is no battery.
		return false;
	}
}

void PowerCheck::interruptHandler() {
	if (millis() - lastChange < 100) {
		// We very recently had a change; assume there is no battey and we're rapidly switching
		// between fast charge and charge done
		hasBattery = false;
	}
	else {
		// Note: It's quite possible that hasBattery will be false when there is a battery; the logic
		// in getHasBattery() takes this into account by checking lastChange as well.
		hasBattery = true;
	}
	lastChange = millis();
}



// Global variables
PowerCheck powerCheck;
unsigned long lastCheck = 0;
char lastStatus[256];

STARTUP(cellular_credentials_set("isp.telus.com", "", "", NULL))

void publishData();
void getParameterHandler(const char *topic, const char *data);

int gas_digital = D0; // Digital pin reading for Gas Sensor
int gas_analog = A0; // Analog pin reading for gas sensor
int dht_5v = D7;
int gas_5v = D3;
int flood_digital = D4;
int loudness_analog = A1;
int speaker_analog = A2;
int motion_digital = D1;

float gas_max, temp_max, humidity_max, loudness_max;
int motion_max, flood_max;

float gas_prev, temp_prev, humidity_prev, loudness_prev;
int motion_prev, flood_prev;

float g, t, h, l;
int m, f;

const char *PUBLISH_EVENT_NAME = "dataUpload";

DHT dht(DHTPIN, DHTTYPE);

FuelGauge fuel;

void getParameterHandler(const char *topic, const char *data) {
    StaticJsonBuffer<256> jsonBuffer;
    char *mutableCopy = strdup(data);
    JsonObject& info = jsonBuffer.parseObject(mutableCopy);
    free(mutableCopy);

    gas_max = atoi(info["gas_max"]);
    temp_max = atoi(info["temp_max"]);
    humidity_max = atoi(info["humidity_max"]);
    loudness_max = atoi(info["loudness_max"]);
    motion_max = atoi(info["motion_max"]);
    flood_max = atoi(info["flood_max"]);
}

void notify(String warning) {
    // Particle.publish("twilio", warning, PRIVATE);
    Serial.printlnf(warning);
    Particle.publish("ifttt_notification", warning, PRIVATE);
}

void publishData() {
    /*
    char data[256];
    snprintf(data, sizeof(data), "{\"motion\":%d,\"gas\":%.2f,\"temp\":%.2f,\"humidity\":%.2f,\"loudness\":%.2f,\"flood\":" + (String) digitalRead(flood_digital) +  "}",
        digitalRead(motion_digital),
        analogRead(gas_analog),
        dht.getTempCelcius(),
        dht.getHumidity(),
        analogRead(loudness_analog));
    */
    char data[256];
    snprintf(data, sizeof(data), "{\"motion\":%d,\"gas\":%.2f,\"temp\":%.2f,\"humidity\":%.2f,\"loudness\":%.2f,\"flood\":" + (String) digitalRead(flood_digital) +  "}",
        m,g,t,h,l);
        //digitalRead(flood_digital));
    Serial.printlnf("Publishing %s", data);
    Particle.publish(PUBLISH_EVENT_NAME, data, PRIVATE);
}


void update_sensors() {
  g = analogRead(gas_analog);
  t = dht.getTempCelcius();
  h = dht.getHumidity();
  l = analogRead(loudness_analog);
  m = digitalRead(motion_digital);
  f = digitalRead(flood_digital);
}


void setup() {
    // Set default max vals
    gas_max = .5;
    temp_max = 30;
    humidity_max = .5;
    loudness_max = 15;
    motion_max = 1;
    flood_max = 1;

  // Setup all the pins
  Particle.keepAlive(30);
  pinMode(gas_digital, INPUT);
  pinMode(gas_analog, INPUT);
  pinMode(motion_digital, INPUT);
  pinMode(loudness_analog, INPUT);
  pinMode(dht_5v, OUTPUT);
  digitalWrite(dht_5v, HIGH);
  pinMode(gas_5v, OUTPUT);
  digitalWrite(gas_5v, HIGH);
  pinMode(flood_digital, INPUT);
  dht.begin();
  pinMode(speaker_analog, OUTPUT);

  update_sensors;

  gas_prev = g;
  temp_prev = t;
  humidity_prev = h;
  loudness_prev = l;
  motion_prev = m;
  flood_prev = f;

  // Subscribe to setting changes
  Particle.subscribe("hook-response/dataParameters", getParameterHandler, MY_DEVICES);

}
void loop() {
    /*
    if (m > motion_max) {
        Serial.printlnf("%d > %d", m, motion_max);
        notify("Motion detected");
    }

    if (g > gas_max) {
        Serial.printlnf("%f > %f", g, gas_max);
        notify("Smoke detected");
    }

    if (t > temp_max) {
        Serial.printlnf("%f > %f", t, temp_max);
        notify("Max temp exceeded");
    }

    if (h > humidity_max) {
        Serial.printlnf("%f > %f", h, humidity_max);
        notify("Max humidity exceeded");
    }

    if (l > loudness_max) {
        Serial.printlnf("%f > %f", l, loudness_max);
        notify("Max sound level exceeded");
    }

    if (f >= flood_max) {
        Serial.printlnf("%d >= %d", f >= flood_max);
        notify("Your house is flooding!");
    }
    */
    Serial.println(digitalRead(flood_digital));
    if (m != motion_prev || g != gas_prev || t != temp_prev || h != humidity_prev || l != loudness_max || f != flood_prev) {
      // update_sensors();
      // publishData();
    }

    if (millis() - lastCheck > 2000) {
      lastCheck = millis();

      char buffer[256];
      snprintf(buffer, sizeof(buffer), "hasPower=%d hasBattery=%d isCharging=%d",
				powerCheck.getHasPower(), powerCheck.getHasBattery(), powerCheck.getIsCharging());

      if (strcmp(lastStatus, buffer) != 0) {
        strcpy(lastStatus, buffer);
        Serial.println("Your house has lost power!");
        notify("Your house has lost power!");
      }
    }
  delay(1000);
}
