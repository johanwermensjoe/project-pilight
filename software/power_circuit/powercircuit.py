#!/usr/bin/env python

""" Script to communicate with a power switching circuit. """

import subprocess
import time
import RPi.GPIO as gpio

POWER_ALERT_INPUT_PIN = 10

def init():
    """ Initializes GPIO pins and IRQ. """
    gpio.setmode(gpio.BOARD)
    gpio.setwarnings(False)
    gpio.setup(POWER_ALERT_INPUT_PIN, gpio.IN)
    
    # Check initial state.
    while not check_alert():
        # Add interrupt for future signals.
        gpio.wait_for_edge(POWER_ALERT_INPUT_PIN, gpio.RISING)

    shutdown()

def check_alert():
    """ Verifies that an actual alert signal was received. """
    for i in range(0, 10):
        if not gpio.input(POWER_ALERT_INPUT_PIN):
            return False
        time.sleep(0.1)
    return True

def shutdown():
    """ The interrupt callback function, shutting down the RBPi. """
    gpio.cleanup()
    subprocess.call(["/sbin/shutdown", "-h", "now"])

# Start script.
init()
