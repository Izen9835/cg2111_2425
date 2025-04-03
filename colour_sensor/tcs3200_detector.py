import RPi.GPIO as GPIO
import time

s2 = 5 #rpi pin 29
s3 = 6 #rpi pin 31
signal = 13 #pin 33

NUM_CYCLES = 10

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(signal, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(s2, GPIO.OUT)
    GPIO.setup(s3, GPIO.OUT)

def read_value(a0, a1):
    GPIO.output(s2, a0)
    GPIO.output(s3, a1)

    time.sleep(0.1)

    print("Waiting for a fall then rise to start reading...")

    # ensure that we start on the end of cycle
    GPIO.wait_for_edge(signal, GPIO.FALLING)
    GPIO.wait_for_edge(signal, GPIO.RISING)

    print("taking readings...")

    # to take an average reading
    start = time.time()
    for oscillation in range(NUM_CYCLES):
        GPIO.wait_for_edge(signal, GPIO.FALLING)

    duration = (time.time() - start)

    return NUM_CYCLES / duration #in Hz

def loop():
    while(True):
        red = read_value(GPIO.LOW, GPIO.LOW)
        time.sleep(0.3)
        green = read_value(GPIO.HIGH, GPIO.HIGH)
        time.sleep(0.3)
        blue = read_value(GPIO.LOW, GPIO.HIGH)
        time.sleep(0.3)

        print("r= ", red)
        print("g= ", green)
        print("b= ", blue)

        if (green > red):
            print("DETECTED COLOUR: GREEN")
        elif (red > green):
            print("DETECTED COLOUR: RED")
        else:
            print("UNKNOWN COLOUR")

if __name__ == "__main__":
    setup()

    try:
        loop()
    except KeyboardInterrupt:
        GPIO.cleanup()
