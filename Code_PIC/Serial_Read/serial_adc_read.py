import serial

# Adjust this to match your COM port (e.g., "COM3" on Windows, "/dev/ttyUSB0" on Linux/Mac)
ser = serial.Serial('COM3', 9600, timeout=1)

print("Reading ADC values...")

while True:
    if ser.in_waiting:
        data = ser.read(1)  # read 1 byte
        adc_value = int.from_bytes(data, byteorder='big')
        print(f"ADC (8-bit): {adc_value}")