#!/usr/bin/python
# Satxo Oven Controller
#
#

_version_ = "0v1"
import sys, serial

class Controller:
	def __init__(self, ttydevice):
		self.port = serial.Serial(
			ttydevice,
			baudrate=57600,
			timeout=3.0
		)

	def send_initframe(self):
		self.port.write(chr(INIT))
		self.port.write(chr(INIT))
		# get ack
		r = self.port.read(1)
		if r != ACK:
			return -1
		else:
			return 0




def dump_temp(device, filename):
	f = open(filename, "a")

	line = ""
	timeline = -1
	while True:

		c = device.port.read()
		if c != "\n":
			line += c
			continue

		if line[:3] != "int":
			line = ""
			continue
	
		timeline += 1

		temp = float(line.split("ext_temp=")[-1][:-2])
		print timeline, temp
		f.write("%d,%f\n" % (timeline, temp))
		line = ""





if __name__ == "__main__":
	print "Satxo Oven Controller %s" % _version_
	
	if len(sys.argv) < 2:
		print "Usage: %s <ttydevice> <dump>.csv" % sys.argv[0]
		print
		sys.exit(1)

	ttydevice = sys.argv[1]
	filename  = sys.argv[2]


	device = Controller(ttydevice)
	dump_temp(device, filename)
	sys.exit(0)
