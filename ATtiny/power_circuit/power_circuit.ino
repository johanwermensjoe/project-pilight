/*  
 *  power_circuit.ino
 *  Program to control power to a RbPi 2 with safe shutdown.  
 *
 *  Created on: Dec 15, 2015
 *  Author: Johan Wermensjö
 */

// Ports.
#define POWER_MC_INPUT_PIN (uint8_t) 0
#define POWER_MC_OUTPUT_PIN (uint8_t) 1
#define POWER_BUTTON_LED_OUTPUT_PIN (uint8_t) 2
#define POWER_SWITCH_OUTPUT_PIN (uint8_t) 3
#define POWER_BUTTON_INPUT_PIN (uint8_t) 4

// Timings.
#define POWER_BUTTON_TIMEOUT (uint16_t) 3000
#define POWER_BUTTON_DELAY (uint16_t) 50
#define POWER_BUTTON_LED_BLINK_DELAY 500

// State values.
// Output.
#define POWER_STATE_ON HIGH
#define POWER_STATE_OFF LOW
#define POWER_BUTTON_LED_ON HIGH
#define POWER_BUTTON_LED_OFF LOW
#define POWER_ALERT_ON HIGH
#define POWER_ALERT_OFF LOW

// Input.
#define POWER_BUTTON_UP LOW
#define POWER_BUTTON_DOWN HIGH
#define POWER_MC_READY HIGH
#define POWER_MC_NOT_READY LOW

void setup() {
    
	pinMode(POWER_MC_INPUT_PIN, INPUT);
	pinMode(POWER_MC_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_BUTTON_LED_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_SWITCH_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_BUTTON_INPUT_PIN, INPUT);
	
	// Initialize to power on.
	digitalWrite(POWER_SWITCH_OUTPUT_PIN, POWER_STATE_ON);
	
	// Turn on power led.
	digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_ON);

	// Initialize no shutdown alert.
	digitalWrite(POWER_MC_OUTPUT_PIN, POWER_ALERT_OFF);
}

void loop() {

	/*****************************/
	
	/** Power on - Turning off. **/
	
	// Wait until button input goes low (button pressed).
	while(digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_UP) {}
	
	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DELAY);

	// Alert MC to begin shutdown procedure.
	digitalWrite(POWER_MC_OUTPUT_PIN, POWER_ALERT_ON);

	// Initiate button timeout.
	uint16_t timeout = POWER_BUTTON_DELAY;
	uint16_t ledToggleTime = POWER_BUTTON_DELAY;
	bool ledState = true;

	while (timeout < POWER_BUTTON_TIMEOUT) {
		
		// Toggle led after the set time.
		if (ledToggleTime >= POWER_BUTTON_LED_BLINK_DELAY) {
			if (ledState) {
				digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_OFF);
			} else {
				digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_ON);
			}
			ledState = !ledState;
			ledToggleTime = 0;
		}

		// Listen for MC ready signal.
		if (digitalRead(POWER_MC_INPUT_PIN) == POWER_MC_READY) {
			break;
		}

		// If button is held for timeout then force shutdown.
		if (digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_DOWN) {
			// Button is still pressed, add delay to counter.
			timeout += POWER_BUTTON_DELAY;
		} else {
			// Button is released midpress, reset counter.
			timeout = 0;
		}
		
		// Add delay to toggle time.
		ledToggleTime += POWER_BUTTON_DELAY;

		// Wait to next check.
		delay(POWER_BUTTON_DELAY);
	}

	// Turn off power.
	digitalWrite(POWER_SWITCH_OUTPUT_PIN, POWER_STATE_OFF);
	
	// Turn off power led.
	digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_OFF);
	
	// Turn off MC alert.
	digitalWrite(POWER_MC_OUTPUT_PIN, POWER_ALERT_OFF);

	// Wait until button is released.
	while(digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_DOWN) {}

	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DELAY);
	
	/*****************************/
	
	/** Power off - Turning on. **/
	
	// Wait until button input goes low (button pressed).
	while(digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_UP) {}
	
	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DELAY);
	
	// Turn on power at reset.
	digitalWrite(POWER_SWITCH_OUTPUT_PIN, POWER_STATE_ON);
	
	// Turn on power led.
	digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_ON);

	// Wait until button is released.
	while(digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_DOWN) {}
	
	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DELAY);
}
