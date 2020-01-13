# Tests integration between C++ and python serial modules, sending data both
# directions.
#
# The C++ side will emulate sending information back; in this case, just an
# incrementing counter. They python side will listen, with the ability to reset
# the counter to an arbitrary number.
#
# This will let us test:
# 1) C++ -> Python
# 2) Python -> C++
# 3) Overall response loop
#
# CPP address: 0
# Python address: 1

import unittest
import logging
import subprocess
import time

from bidir_serial_control import BidirSerialRXModule, BidirSerialThread
from hardware_abstraction import FileSerialConnection

class IntegrationTest(unittest.TestCase):
    def createSerials(self, name):
        self.serial_connection = FileSerialConnection(name + str(time.time()), 9600)
        self.thread = BidirSerialThread(self.serial_connection)
        self.cc_subprocess = subprocess.Popen([
            "serial_link/bidir_serial_main_for_test",
            self.serial_connection.outgoing_filename,
            self.serial_connection.incoming_filename,
            '0'])
        self.thread.start()

    def testCommunication(self):
        self.createSerials('testCommunication')
        for count in range(0, 5):
            m0 = self.thread.ReadMessage(1)
            self.assertEqual(m0.command, [ord(c) for c in 'COUNT'] + [count])
        self.thread.WriteMessage(0, 'SET_COUNT\xf0')
        m0 = self.thread.ReadMessage(1)
        self.assertEqual(m0.command, [ord(c) for c in 'COUNT'] + [5])
        found = False
        for i in range(20):
            m0 = self.thread.ReadMessage(1)
            if m0.command == [ord(c) for c in 'COUNT'] + [0xf1]:
                found = True
                break
            logging.info(m0.command)
        self.assertTrue(found)
        m0 = self.thread.ReadMessage(1)
        self.assertEqual(m0.command, [ord(c) for c in 'COUNT'] + [0xf2])

    def testLongCommands(self):
        """We're going to use a command as follows:
        REWRITE + find char + replace char + text.
        """
        self.createSerials('testLongCommands')
        self.thread.WriteMessage(0, 'REWRITE-_some-long-string-of-text-to-change')
        found = False
        for i in range(20):
            incoming = self.thread.ReadMessage(1)
            if incoming.command[:5] == [ord(c) for c in 'COUNT']:
                continue
            found = True
            self.assertEqual(incoming.command, [ord(c) for c in 'REWRITE-_some_long_string_of_text_to_change'])
        self.assertTrue(found)

if __name__ == '__main__':
        unittest.main()
