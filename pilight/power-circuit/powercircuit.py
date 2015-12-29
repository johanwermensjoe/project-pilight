#!/usr/bin/env python

""" Script to communicate with a power switching circuit. """

import subprocess
import RPi.GPIO as gpio

POWER_ALERT_INPUT_PIN = 10

def init():
    """ Initializes GPIO pins and IRQ. """
    gpio.setmode(gpio.BOARD)
    gpio.setup(POWER_ALERT_INPUT_PIN, gpio.RISING, callback=shutdown, \
                bouncetime=200)

def loop():
    """ Main script loop. """
    raw_input()

def shutdown(pin):
    """ The interrupt callback function, shutting down the RBPi. """
    subprocess.call(["shutdown", "-h", "now"])

# Start script.
init()
loop()
