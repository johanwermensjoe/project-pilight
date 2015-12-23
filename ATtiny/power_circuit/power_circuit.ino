/*  
 *  power_circuit.ino
 *  Program to control power to a RbPi 2 with safe shutdown.  
 *
 *  Created on: Dec 15, 2015
 *  Author: Johan Wermensjö
 */
 
// Definitions.
#define AUTO_ON

// Ports.
#define POWER_CTRL_INPUT_PIN (uint8_t) 1 // Analog pin number (digital pin 2)
#define POWER_CTRL_OUTPUT_PIN (uint8_t) 1
#define POWER_BUTTON_LED_OUTPUT_PIN (uint8_t) 0
#define POWER_SWITCH_OUTPUT_PIN (uint8_t) 4
#define POWER_BUTTON_INPUT_PIN (uint8_t) 3

// Timings.
#define POWER_BUTTON_TIMEOUT (uint16_t) 3000
#define POWER_BUTTON_DELAY (uint16_t) 50
#define POWER_BUTTON_LED_BLINK_DELAY 500

// Output states.
#define POWER_STATE_ON HIGH
#define POWER_STATE_OFF LOW
#define POWER_BUTTON_LED_ON HIGH
#define POWER_BUTTON_LED_OFF LOW
#define POWER_ALERT_ON HIGH
#define POWER_ALERT_OFF LOW

// Input states.
#define POWER_BUTTON_UP LOW
#define POWER_BUTTON_DOWN HIGH
#define POWER_CTRL_READY_THRESHOLD (uint16_t) 512 // Corresponds to 2.5V

// Functions.
static void waitForButtonState(uint8_t state);
static bool isControlReady();
static void enablePower(bool power);

// Variables.
static bool powerOn;

void setup() {

	pinMode(POWER_CTRL_INPUT_PIN, INPUT);
	pinMode(POWER_CTRL_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_BUTTON_LED_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_SWITCH_OUTPUT_PIN, OUTPUT);
	pinMode(POWER_BUTTON_INPUT_PIN, INPUT);
	
	// Initialize power state.
	#ifdef AUTO_ON
		enablePower(true);
	#else
		enablePower(false);
	#endif
	
	// Initialize no shutdown alert.
	digitalWrite(POWER_MC_OUTPUT_PIN, POWER_ALERT_OFF);
}

void loop() {
	
	// Make sure button is released between state changes.
	waitForButtonState(POWER_BUTTON_UP);

	// Wait for button press to toggle power.
	waitForButtonState(POWER_BUTTON_DOWN);
	
	if (powerOn) {
		/*****************************/
		/** Power on - Turning off. **/

		// Alert to begin shutdown procedure.
		digitalWrite(POWER_CTRL_OUTPUT_PIN, POWER_ALERT_ON);
	
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
	
			// Listen for ready signal.
			if (isControlReday()) {
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
		enablePower(false);
		
		// Turn off alert.
		digitalWrite(POWER_CTRL_OUTPUT_PIN, POWER_ALERT_OFF);
		
	} else {
		/*****************************/
		/** Power off - Turning on. **/
		
		// Turn on power.
		enablePower(true);
	}
}

static void waitForButtonState(uint_8 state) {

	// Wait until button has requested state.
	while(digitalRead(POWER_BUTTON_INPUT_PIN) != state) {}

	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DELAY);
}

static void isControlReady() {
	return analogRead(POWER_CTRL_INPUT_PIN) >= POWER_CTRL_READY_THRESHOLD
}

static void enablePower(bool power) {
	// Set power state to on.
    digitalWrite(POWER_SWITCH_OUTPUT_PIN, power ?
                        POWER_STATE_ON : POWER_STATE_OFF);

	// Turn on power led.
	digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, power ?
	                    POWER_BUTTON_LED_ON : POWER_BUTTON_LED_OFF);

	// Set state.
	powerOn = power;
}