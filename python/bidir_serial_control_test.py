from bidir_serial_control import BidirSerialRXModule, BidirSerialThread, ERROR, READY_AND_DONE, READY_AND_SENDING, ToChar
from hardware_abstraction import QueueSerialConnection
import serial_control
import time
import unittest
import logging

class SingleEndedTest(unittest.TestCase):
    def setUp(self):
        self.serial_connection = QueueSerialConnection('create', 9600)
        self.thread = BidirSerialThread(self.serial_connection)

    def test_create(self):
        pass

    def assertCharsSent(self, chars):
        self.assertTrue(self.serial_connection.incoming.empty())
        for char in chars:
            self.assertTrue(not self.serial_connection.outgoing.empty())
            self.assertEqual(self.serial_connection.outgoing.get(), bytes([ord(char)]))
        self.assertTrue(self.serial_connection.outgoing.empty())

    def test_run_rx_without_pair(self):
        self.thread.RxTxLoop()
        time.sleep(0.21)
        self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(ERROR)])

    def test_run_rx_tx_without_pair(self):
        self.thread.WriteMessage(42, 'MOTOR_RUN')
        self.thread.RxTxLoop()
        time.sleep(0.21)
        self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(ERROR)])

    def test_run_rx_incoming(self):
        self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
        self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(READY_AND_DONE)])

    def test_run_rx_incoming_tx_outgoing(self):
        self.thread.WriteMessage(14, 'MOTOR_RUN')
        self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
        self.thread.RxTxLoop()
        my_message = serial_control.MessageToBytes(14, 'MOTOR_RUN', send_checksums=False)
        self.assertCharsSent([ToChar(READY_AND_SENDING, len(my_message))] + my_message)

    def test_run_rx_full_message(self):
        my_message = serial_control.MessageToBytes(9, 'Yo', send_checksums=False)
        self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
        for byte in my_message:
            self.assertFalse(self.thread.RxTxLoop())
            self.assertCharsSent([ToChar(READY_AND_DONE)])

            self.serial_connection.incoming.put(ToChar(READY_AND_SENDING, 1))
            self.serial_connection.incoming.put(str(byte))

        message = self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(READY_AND_DONE)])
        self.assertTrue(message)
        self.assertTrue(message.command, 'Yo')
        self.assertTrue(message.address, 9)

    def test_run_tx_full_message(self):
        self.thread.WriteMessage(14, 'MOTOR_RUN')
        my_message = serial_control.MessageToBytes(14, 'MOTOR_RUN', send_checksums=False)
        self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
        self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(READY_AND_SENDING, len(my_message))] + my_message)

        self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
        self.thread.RxTxLoop()
        self.assertCharsSent([ToChar(READY_AND_DONE)])

    # TODO
    #ef test_run_rx_tx_full_message_same_time(self):
    #   read_messages = (serial_control.MessageToBytes(9, 'Yo', send_checksums=False) + ['SPACE'] +
    #                    serial_control.MessageToBytes(9, 'Im readin', send_checksums=False))
    #   send_messages = (serial_control.MessageToBytes(14, 'MOTOR_RUN', send_checksums=False) + ['SPACE'] +
    #                    serial_control.MessageToBytes(14, 'hi', send_checksums=False))
    #   logging.info(read_messages)
    #   logging.info(serial_control.MessageToBytes(9, 'Yo', send_checksums=False))
    #   self.thread.WriteMessage(14, 'MOTOR_RUN')
    #   self.thread.WriteMessage(14, 'hi')
    #   r_w_zipped = zip(read_messages, send_messages)
    #   messages = []
    #   for r, w in r_w_zipped:
    #       logging.info('yo: %s', r)
    #       if r == 'SPACE':
    #           self.serial_connection.incoming.put(ToChar(READY_AND_DONE))
    #       else:
    #           self.serial_connection.incoming.put(ToChar(READY_AND_SENDING, 1))
    #           self.serial_connection.incoming.put(str(r))
    #       message = self.thread.RxTxLoop()
    #       if message:
    #           messages.append(message)
    #       logging.info("w = %s", w)
    #       if w == 'SPACE':
    #           self.assertCharsSent([ToChar(READY_AND_DONE)])
    #       else:
    #           self.assertCharsSent([ToChar(READY_AND_SENDING), str(w)])
    #   self.assertEqual(2, len(messages))
    #   self.assertEqual(9, messages[0].address)
    #   self.assertEqual(9, messages[1].address)
    #   self.assertEqual('Yo', ''.join([chr(c) for c in messages[0].command]))
    #   self.assertEqual('Im readin', ''.join([chr(c) for c in messages[1].command]))


class DoubleEndedTest(unittest.TestCase):
    def setUp(self):
        self.serial_connection = QueueSerialConnection('create', 9600)
        self.thread_a = BidirSerialThread(self.serial_connection)
        self.thread_b = BidirSerialThread(self.serial_connection.MakePair())

    def test_send_to_each_other(self):
        self.thread_a.start()
        self.thread_b.start()
        self.thread_a.WriteMessage(3, 'hello')
        self.thread_b.WriteMessage(7, 'world!')
        m0 = self.thread_b.ReadMessage(1)
        m1 = self.thread_a.ReadMessage(1)
        m2 = self.thread_b.ReadMessage(0.1)
        m3 = self.thread_a.ReadMessage(0.1)
        self.assertTrue(m0.command, 'hello')
        self.assertTrue(m1.command, 'world!')
        self.assertEquals(None, m2)
        self.assertEquals(None, m3)


    def test_send_to_each_other_timing_tweaks(self):
        self.thread_a.start()
        time.sleep(0.3)
        self.thread_b.start()
        self.thread_a.WriteMessage(3, 'hello')
        time.sleep(0.1)
        self.thread_b.WriteMessage(7, 'world!')
        self.thread_a.WriteMessage(3, 'm2')
        self.thread_b.WriteMessage(3, 'm3')
        m0 = self.thread_b.ReadMessage(1)
        m1 = self.thread_a.ReadMessage(1)
        m2 = self.thread_b.ReadMessage(0.1)
        m3 = self.thread_a.ReadMessage(0.1)
        self.assertTrue(m0.command, 'hello')
        self.assertTrue(m1.command, 'world!')
        self.assertTrue(m2.command, 'm2')
        self.assertTrue(m3.command, 'm3')

if __name__ == '__main__':
        unittest.main()
