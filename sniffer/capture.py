import serial
import datetime
import time
from serial.tools.list_ports import comports

LISTEN_TIME = 60 #time to listen before exit in seconds

print("------------------------------------------")
ports = comports()
for index in range(len(ports)):
    print(f"{index} - {ports[index]}")

port_index = input(f"Select com port of the sniffer board [0-{len(ports)-1}] ")

try:
    port_index = int(port_index)
except:
    print("Invalid selection")
    exit(0)

try:
    ser = serial.Serial(ports[port_index].device, baudrate=921600)
    ser.timeout = LISTEN_TIME
except:
    print("Failed to open serial port, please verify com port")
    exit(0)
filename = "capture_%s.pcap" % datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

print("Creating capture file: %s" % filename)
f = open(filename, 'wb')

def write_hex(f, hex_string):
    f.write(bytes.fromhex(hex_string))
    f.flush()

# PCAP file header
header = 'd4c3b2a1' + '0200' + '0400' + '00000000' + '00000000' + 'c4090000' + '69000000'
write_hex(f, header)

print("Waiting for packets...")
print(f"Listening for {LISTEN_TIME} seconds")
start = time.time()
endTime = start + LISTEN_TIME
try:
    while time.time() < endTime:
        line = ser.readline()
        try:
            line = line.decode().strip()
        except:
            continue
        if line.startswith("DATA:"):
            data = line[5:]
            write_hex(f, data)
            print(data)
        else:
            print(line)

except KeyboardInterrupt:
    print("Stopping")
except ValueError:
    print("Value error" + data)
    
f.close()
ser.close()
print("Capture file: %s" % filename)
print("Done")