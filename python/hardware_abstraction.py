import serial
import queue
import logging

class SerialConnection(object):
    def __init__(self, port, baud):
        pass

    def CreateWithHardware(port, baud):
        return HardwareSerialConnection(port, baud)

    def CreateWithFile(port, baud):
        return FileSerialConnection(port, baud)

    def write(self, byte):
        pass

    def read(self):
        """Returns empty char if there is no data to read."""
        pass

class HardwareSerialConnection(SerialConnection):
    def __init__(self, port, baud):
        self.ser = serial.Serial(port, baud, timeout=0, rtscts=True)  # 1s timeout

    def write(self, byte):
        self.ser.write(byte)

    def read(self):
        return self.ser.read()

class FileSerialConnection(SerialConnection):
    def __init__(self, port, baud, incoming_filename=None, outgoing_filename=None):
        if incoming_filename:
            self.incoming_filename = incoming_filename
        else:
            self.incoming_filename = '/tmp/arduinoio_hw_abstraction_a_%s' % port
        if outgoing_filename:
            self.outgoing_filename = outgoing_filename
        else:
            self.outgoing_filename = '/tmp/arduinoio_hw_abstraction_b_%s' % port
        with open(self.incoming_filename, 'w') as fcreate:
            pass
        self.incoming = open(self.incoming_filename, 'rb')
        self.outgoing = open(self.outgoing_filename, 'wb')
        self.port = port
        self.baud = baud

    def write(self, byte):
        self.outgoing.write(byte)
        self.outgoing.flush()
        logging.info('write: %s', byte)

    def read(self):
        result = self.incoming.read(1)
        if result == b'':
            # Avoids EOF getting set and skipping future reads.
            self.incoming.seek(self.incoming.tell())
        return result

    def MakePair(self):
        return FileSerialConnection(self.port, self.baud, self.outgoing_filename, self.incoming_filename)

class QueueSerialConnection(SerialConnection):
    def __init__(self, port, baud):
        self.port = port
        self.baud = baud
        self.incoming = queue.Queue(100)
        self.outgoing = queue.Queue(100)

    def write(self, byte):
        self.outgoing.put(byte)

    def read(self):
        try:
            return self.incoming.get(block=False)
        except queue.Empty:
            return b''

    def MakePair(self):
        new_queue = QueueSerialConnection(self.port, self.baud)
        new_queue.incoming = self.outgoing
        new_queue.outgoing = self.incoming
        return new_queue
