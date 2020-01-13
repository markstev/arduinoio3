from serial_control import Message, MessageToBytes
from threading import Thread
import queue
import hardware_abstraction
import logging
import time

READY_AND_DONE = 1
READY_AND_SENDING = 2
SENT_LAST = 3
ERROR = 4
UNKNOWN = 5


MAX_BYTES_PER_TRANSMISSION = 15


def FromChar(char):
    ord_to_state = {
            0x50: READY_AND_DONE,
            0x60: READY_AND_SENDING,
            0x70: SENT_LAST,
            0x80: ERROR
            }
    return ord_to_state.get(ord(char) & 0xf0, ERROR), ord(char) & 0x0f


def ToChar(state, num_bytes=0):
    state_to_ord = {
            READY_AND_DONE: 0x50,
            READY_AND_SENDING: 0x60,
            SENT_LAST: 0x70,
            ERROR: 0x80
            }
    if state not in state_to_ord:
        return bytes([state_to_ord[ERROR]])
    return bytes([state_to_ord[state] | num_bytes])


COMMUNICATION_TIMEOUT = 0.2  # seconds


class BidirSerialRXModule(object):
    def __init__(self, serial_connection):
        self.thread = BidirSerialThread(serial_connection, incoming_queue, outgoing_queue)
        self.thread.start()

    def Write(self, address, command, timeout=0):
        """Command is a string. Blocks until this command is sent."""
        self.thread.WriteMessage(address, command)

    def Read(self, timeout=0):
        """Returns a complete message, if incoming transmission is complete."""
        return self.thread.ReadMessage(timeout)

    def Clear(self):
        self.message = Message()

class BidirSerialThread(Thread):
    def __init__(self, serial_connection):
        Thread.__init__(self)
        self.ser = serial_connection
        self.incoming_message = None
        self.rx_state = READY_AND_DONE
        self.tx_state = READY_AND_DONE
        self.last_communication_time = time.time()
        self.rx_queue = queue.Queue(5)
        self.tx_queue = queue.Queue(5)
        self.daemon = True

    def WriteMessage(self, address, command):
        self.tx_queue.put((address, command))

    def ReadMessage(self, timeout):
        try:
            return self.rx_queue.get(timeout=timeout)
        except queue.Empty:
            return None

    def run(self):
        while True:
            incoming_message = self.RxTxLoop()
            if incoming_message:
                # Block on purpose, for flow control.
                self.rx_queue.put(incoming_message)

    def RxTxLoop(self):
        """Called in a loop; handles a single rx/tx pair."""
        # Receive
        byte_read = self.ReadFlushingErrors()
        if not byte_read:
            if time.time() - self.last_communication_time > COMMUNICATION_TIMEOUT:
                self.SendError()
                self.last_communication_time = time.time()
            return
        new_rx_state, num_bytes_to_read = FromChar(byte_read)
        #logging.info('State: %s; incoming: %s', ToChar(self.rx_state), ToChar(new_rx_state))
        finished = False
        if new_rx_state == ERROR:
            self.ResetRx()
        else:
            if (self.rx_state == READY_AND_SENDING and new_rx_state != READY_AND_SENDING):
                logging.info('Unexpected early end of message send')
                self.SendError()
                return None
            if self.rx_state in [READY_AND_SENDING, READY_AND_DONE] and new_rx_state == READY_AND_SENDING:
                if self.rx_state == READY_AND_DONE:
                    # First byte. Reset stuff.
                    self.incoming_message = Message()
                for i in range(num_bytes_to_read):
                    if finished:
                        self.SendError()
                        return None
                    byte = self.ReadWithTimeout()
                    if not byte:
                        logging.info('Still reading, but received no data.')
                        self.SendError()
                        return None
                    finished = self.incoming_message.AddByte(ord(byte))
                if finished:
                    self.rx_state = SENT_LAST
                else:
                    self.rx_state = READY_AND_SENDING
            elif self.rx_state == SENT_LAST:
                if new_rx_state == READY_AND_SENDING:
                    logging.info('Unexpected extra byte after message received.')
                    self.SendError()
                    return None
                self.rx_state = READY_AND_DONE

        # Transmit
        num_bytes_to_send = 0
        if self.tx_state == SENT_LAST:
            self.tx_state = READY_AND_DONE
        elif self.tx_state == READY_AND_DONE:
            # Pop the next message to send, if any.
            try:
                next_transmission = self.tx_queue.get(timeout=0.01)
                address, command = next_transmission
                self.transmit_bytes = MessageToBytes(address, command, send_checksums=False)
                self.transmit_index = 0
                self.tx_state = READY_AND_SENDING
            except queue.Empty:
                # This is expected and common; just nothing to send right now.
                pass
        if self.tx_state == READY_AND_SENDING:
            num_bytes_to_send = min(MAX_BYTES_PER_TRANSMISSION,
                    len(self.transmit_bytes) - self.transmit_index)
        self.ser.write(ToChar(self.tx_state, num_bytes_to_send))
        if self.tx_state == READY_AND_SENDING:
            for i in range(num_bytes_to_send):
                self.ser.write(bytes([ord(self.transmit_bytes[self.transmit_index])]))
                self.transmit_index += 1
            if self.transmit_index == len(self.transmit_bytes):
                self.tx_state = SENT_LAST
        self.last_communication_time = time.time()
        if finished:
            return self.incoming_message
        else:
            return None


    def ResetRx(self):
        """Resets internal tx/rx state."""
        self.transmit_index = 0
        self.rx_state = READY_AND_DONE
        if self.tx_state != READY_AND_DONE:
            self.tx_state = READY_AND_SENDING

    def SendError(self):
        """Sends an error and resets all tx/rx."""
        self.ser.write(ToChar(ERROR))
        self.ResetRx()

    def ReadWithTimeout(self):
        """Reads up until the timeout, returning empty string on timeout."""
        start = time.time()
        while time.time() - start < COMMUNICATION_TIMEOUT:
            byte = self.ser.read()
            if byte:
                return byte
        return None

    def ReadFlushingErrors(self):
       """Returns the first non-error byte, or empty if nothing has been sent."""
       last_read = None
       byte = ERROR
       while byte == ERROR:
           byte = self.ser.read()
           if byte:
               last_read = byte
       return last_read
