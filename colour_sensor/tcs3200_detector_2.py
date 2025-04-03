import RPi.GPIO as GPIO
import time
import matplotlib.pyplot as plt
from collections import deque

s2 = 5  # rpi pin 29
s3 = 6  # rpi pin 31
signal = 13  # pin 33

NUM_CYCLES = 10
BUFFER_SIZE = 50  # Number of points to display

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(signal, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(s2, GPIO.OUT)
    GPIO.setup(s3, GPIO.OUT)

def read_value(a0, a1):
    GPIO.output(s2, a0)
    GPIO.output(s3, a1)
    
    time.sleep(0.1)
    GPIO.wait_for_edge(signal, GPIO.FALLING)
    GPIO.wait_for_edge(signal, GPIO.RISING)
    
    start = time.time()
    for _ in range(NUM_CYCLES):
        GPIO.wait_for_edge(signal, GPIO.FALLING)
    
    duration = time.time() - start
    return NUM_CYCLES / duration  # in Hz

def main():
    setup()
    
    plt.ion()
    fig, ax = plt.subplots()
    
    x_vals = deque(maxlen=BUFFER_SIZE)
    r_vals = deque(maxlen=BUFFER_SIZE)
    g_vals = deque(maxlen=BUFFER_SIZE)
    b_vals = deque(maxlen=BUFFER_SIZE)
    index = 0
    
    try:
        while True:
            red = read_value(GPIO.LOW, GPIO.LOW)
            green = read_value(GPIO.HIGH, GPIO.HIGH)
            blue = read_value(GPIO.LOW, GPIO.HIGH)
            
            x_vals.append(index)
            r_vals.append(red)
            g_vals.append(green)
            b_vals.append(blue)
            index += 1
            
            ax.clear()
            ax.plot(x_vals, r_vals, 'r-', label='Red')
            ax.plot(x_vals, g_vals, 'g-', label='Green')
            ax.plot(x_vals, b_vals, 'b-', label='Blue')
            
            ax.legend()
            ax.set_title("Real-time Color Sensor Readings")
            ax.set_xlabel("Time Step")
            ax.set_ylabel("Frequency (Hz)")
            plt.pause(0.1)
            
    except KeyboardInterrupt:
        GPIO.cleanup()
        plt.ioff()
        plt.show()

if __name__ == "__main__":
    main()
