#!/usr/bin/python
# Satxo Oven Controller
#
#
# Sending a reflow profile to the controller
#
# Protocol:
#  - Send init frame:
#      0xfe, 0xfe
#  - Send write command and store position:
#      0x01, <store position>
#  - Send first temperature point (start of pre-heat)
#      0x00, 0x00, <temp_h>, <temp_l>
#  - Send next temperature point (start of soak)
#      <time_h>, <time_l>, <temp_h>, <temp_l>
#  - Continue sending temperature points
#      ...
#  - End with:
#      0xff, 0xff
#
#  Note: for each temperature point sent, receive
#  one ack byte (0xcc)
#
#
# Receiving a reflow profile
#
# Protocol:
#  - Send init frame:
#      0xfe, 0xfe
#  - Send read command and store position:
#      0x02, <store position>
#  - Receive the first temperature point:
#      0x00, 0x00, <temp_h>, <temp_l>
#  - Send and ack byte:
#      0xcc
#  - Continue receiving temperature points until we receive
#    the end of points flag:
#      0xff, 0xff
#
_version_ = "0v1"
import sys, serial

INIT = 0xfe
ACK  = 0xcc


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



def read_profile(device, filename, mem_pos):
	pass


def write_profile(device, filename, mem_pos):
	f = file(filename, "rb")
	all_points = [s.split(",")[:2] for s in f.readlines()][1:]
	f.close()

	device.send_initframe()
	for point in all_points:
		if len(point) < 2: continue
		point = [int(p) for p in point]
		print "Sending point: %dC @ %d sec." % (point[1], point[0])





if __name__ == "__main__":
	print "Satxo Oven Controller %s" % _version_
	
	if len(sys.argv) < 5:
		print "Usage: %s [write|read] <ttydevice> <reflow_profile>.csv <memory position>" % sys.argv[0]
		print
		sys.exit(1)

	op        = sys.argv[1]
	ttydevice = sys.argv[2]
	filename  = sys.argv[3]
	mem_pos   = sys.argv[4]


	device = Controller(ttydevice)

	if op.upper() == "WRITE":
		write_profile(device, filename, mem_pos)
	elif op.upper() == "READ":
		read_profile(device, filename, mem_pos)
	else:
		print "Invalid operation: %s" % op
		print
		sys.exit(1)
		

	sys.exit(0)
