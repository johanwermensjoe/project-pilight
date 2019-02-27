/*  
 *  power_circuit.ino
 *  Program to control power to a RbPi 2 with safe shutdown.  
 *
 *  Created on: Dec 15, 2015
 *  Author: Johan Wermensjö
 */

// Definitions.
//#define AUTO_ON

// Ports.
#define POWER_CTRL_INPUT_PIN (uint8_t) 3 // Analog pin number (digital pin 3)
#define POWER_CTRL_OUTPUT_PIN (uint8_t) 4
#define POWER_BUTTON_LED_OUTPUT_PIN (uint8_t) 2
#define POWER_SWITCH_OUTPUT_PIN (uint8_t) 0
#define POWER_BUTTON_INPUT_PIN (uint8_t) 1

// Timings.
#define POWER_BUTTON_HOLD_TIME (uint16_t) 3000
#define POWER_BUTTON_POLL_DELAY (uint16_t) 50
#define POWER_BUTTON_DEBOUNCE_DELAY (uint16_t) 200
#define POWER_BUTTON_LED_BLINK_DELAY (uint16_t) 500
#define POWER_BUTTON_LED_FAST_BLINK_DELAY (uint16_t) 250
#define POWER_CTRL_READY_HOLD_TIME (uint16_t) 2000
#define POWER_CTRL_READY_POST_TIME (uint16_t) 8000

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
#define POWER_CTRL_READY_THRESHOLD (uint16_t) 256 // Corresponds to 1.25V

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
	digitalWrite(POWER_CTRL_OUTPUT_PIN, POWER_ALERT_OFF);
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
		uint16_t powerButtonnHeldTime = 0;
		uint16_t ledToggleTime = 0;
		uint16_t ctrlReadyTime = 0;
		bool ledState = true;
		bool ctrlReadyReceived = false;
	
		while (powerButtonnHeldTime < POWER_BUTTON_HOLD_TIME) {
			// Wait to next check.
			delay(POWER_BUTTON_POLL_DELAY);
			
			// Add delay to toggle time.
			ledToggleTime += POWER_BUTTON_POLL_DELAY;
	
			// Toggle led after the set time.
			if (ledToggleTime >= POWER_BUTTON_LED_BLINK_DELAY ||
					(ctrlReadyReceived && ledToggleTime >= POWER_BUTTON_LED_FAST_BLINK_DELAY)) {
				if (ledState) {
					digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_OFF);
				} else {
					digitalWrite(POWER_BUTTON_LED_OUTPUT_PIN, POWER_BUTTON_LED_ON);
				}
				ledState = !ledState;
				ledToggleTime = 0;
			}
	
			// Listen for ready signal.
			if (isControlReady()) {
				ctrlReadyTime += POWER_BUTTON_POLL_DELAY;
				
				// Set ready signal as recived when held for required time.
				if (ctrlReadyTime >= POWER_CTRL_READY_HOLD_TIME) {
					ctrlReadyReceived = true;

					// Check if required time after accepted signal has passed.
					if (ctrlReadyTime >= POWER_CTRL_READY_POST_TIME) {
						break;
					}
				}
			} else {
				// Reset time and flag if not ready anymore.
				ctrlReadyReceived = false;
				ctrlReadyTime = 0;
			}
	
			// If button is held for timeout then force shutdown.
			if (digitalRead(POWER_BUTTON_INPUT_PIN) == POWER_BUTTON_DOWN) {
				// Button is still pressed, add delay to counter.
				powerButtonnHeldTime += POWER_BUTTON_POLL_DELAY;
			} else {
				// Button is released midpress, reset counter.
				powerButtonnHeldTime = 0;
			}
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

static void waitForButtonState(uint8_t state) {

	// Wait until button has requested state.
	while(digitalRead(POWER_BUTTON_INPUT_PIN) != state) {}

	// Additional delay to avoid multiple toggles.
	delay(POWER_BUTTON_DEBOUNCE_DELAY);
}

static bool isControlReady() {
	return analogRead(POWER_CTRL_INPUT_PIN) < POWER_CTRL_READY_THRESHOLD;
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
