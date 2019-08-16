import serial
import time
ser = serial.Serial("COM13",115200,timeout=0)


while 1:
    guardar = open("archivo.csv", "a")
    time.sleep(2)
    info = ser.readline()
    guardar.write(info)
    print(info)


