import RPi.GPIO as GPIO
import time
from math import sqrt

s0 = 5
s1 = 6
s2 = 19 #rpi pin 
s3 = 26 #rpi pin 
signal = 13 #pin 

NUM_CYCLES = 10

# Colour Constants
class colour:
    def __init__(self, r, g, b):
        self.r = r
        self.g = g
        self.b = b

redAstro = colour(33.0, 13.0, 17.0)
greAstro = colour(15.0, 26.0, 25.0)
DETECT_THRESH = 9.0


def euc_dist(r1, g1, b1, r2, g2, b2):
    R = (r1-r2)**2
    G = (g1-g2)**2
    B = (b1-b2)**2
    return sqrt(R + G + B)

def closest(r, g, b):
    distToRed = euc_dist(r, g, b, redAstro.r, redAstro.g, redAstro.b)
    distToGreen = euc_dist(r, g, b, greAstro.r, greAstro.g, greAstro.b)

    # if both colours are very far off then we take it as no astronaut within the grabber
    if (distToRed > DETECT_THRESH and distToGreen > DETECT_THRESH):
        return "no colour"
    elif (distToRed < distToGreen):
        return "red"
    else:
        return "green"
    

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(signal, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(s2, GPIO.OUT)
    GPIO.setup(s3, GPIO.OUT)

    # set 20% frequency scaling
    GPIO.setup(s0, GPIO.OUT)
    GPIO.setup(s1, GPIO.OUT)
    GPIO.output(s0, GPIO.LOW)
    GPIO.output(s1, GPIO.HIGH)


def read_value(a0, a1):
    GPIO.output(s2, a0)
    GPIO.output(s3, a1)

    time.sleep(0.1)

    #print("Waiting for a fall then rise to start reading...")

    # ensure that we start on the end of cycle
    GPIO.wait_for_edge(signal, GPIO.FALLING)
    GPIO.wait_for_edge(signal, GPIO.RISING)

    #print("taking readings...")

    # to take an average reading
    start = time.time()
    for oscillation in range(NUM_CYCLES):
        GPIO.wait_for_edge(signal, GPIO.FALLING)

    duration = (time.time() - start) * 10

    return NUM_CYCLES / duration #in Hz

"""
def loop():
    while(True):
        red = read_value(GPIO.LOW, GPIO.LOW)
        time.sleep(0.1)
        green = read_value(GPIO.HIGH, GPIO.HIGH)
        time.sleep(0.1)
        blue = read_value(GPIO.LOW, GPIO.HIGH)
        time.sleep(0.1)

        print("r= ", red)
        print("g= ", green)
        print("b= ", blue)

        if (green > red):
            print("DETECTED COLOUR: GREEN")
        elif (red > green):
            print("DETECTED COLOUR: RED")
        else:
            print("UNKNOWN COLOUR")
"""

def loop():
    no_of_readings = 10
    while True:
        red = 0
        green = 0
        blue = 0
        print("measuring avg readings...")
        for bruh in range(no_of_readings):
            red += read_value(GPIO.LOW, GPIO.LOW)
            time.sleep(0.1)
            green += read_value(GPIO.HIGH, GPIO.HIGH)
            time.sleep(0.1)
            blue += read_value(GPIO.LOW, GPIO.HIGH)
            time.sleep(0.1)
    

        redAvg = red/no_of_readings
        greAvg = green/no_of_readings
        bluAvg = blue/no_of_readings

        print("r= ", redAvg)
        print("g= ", greAvg)
        print("b= ", bluAvg)

        colourGuess = closest(redAvg, greAvg, bluAvg)

        if colourGuess == "no colour":
            print("no astro")
        elif colourGuess == "red":
            print("red detected")
        else:
            print("green detected")

        input("press <ENTER> to cont")










if __name__ == "__main__":
    setup()

    try:
        loop()
    except KeyboardInterrupt:
        GPIO.cleanup()
