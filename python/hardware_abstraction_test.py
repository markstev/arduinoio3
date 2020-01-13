import unittest
import hardware_abstraction

class HardwareAbstractionTest(unittest.TestCase):
    def setUp(self):
        pass

    def assertSerialWorks(self, serial_connection):
        serial_pair = serial_connection.MakePair()
        self.assertEqual(serial_connection.read(), b'')
        self.assertEqual(serial_pair.read(), b'')
        serial_connection.write(b'z')
        self.assertEqual(serial_connection.read(), b'')
        self.assertEqual(serial_pair.read(), b'z')
        self.assertEqual(serial_pair.read(), b'')
        serial_pair.write(b'y')
        self.assertEqual(serial_pair.read(), b'')
        self.assertEqual(serial_connection.read(), b'y')
        self.assertEqual(serial_connection.read(), b'')

    def test_queue_serial(self):
        queue_serial = hardware_abstraction.QueueSerialConnection('ha_test', 9600)
        self.assertSerialWorks(queue_serial)

    def test_file_serial(self):
        file_serial = hardware_abstraction.FileSerialConnection('ha_test', 9600)
        self.assertSerialWorks(file_serial)

if __name__ == '__main__':
        unittest.main()
