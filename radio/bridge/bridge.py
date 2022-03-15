import asyncio
import serial_asyncio
from ports import serial_ports, get_radio
from pythonosc.dispatcher import Dispatcher
from patch import names
from pythonosc.osc_server import AsyncIOOSCUDPServer
from typing import List, Any
import time

# OSC Server IP
SERVER_IP = "127.0.0.1"
SERVER_PORT = 5005
# UDP Message filter
UDP_FILTER = "/mask/*"
# Serial baudrate
BAUD = 115200

def std_out(msg, who):
    print (f'[{who}]: {msg}')

# Run this to make a forever loop with no sleep
async def loop():
    while True:
        await asyncio.sleep(0)

class Bridge(object):
    """
        Makes a bridge between an OSC-UDP async server and a serial
        device. The UDP input is forwarded with some
    """
    def __init__(self):
        super(Bridge, self).__init__()

    async def main(self, dispatcher):

        server = AsyncIOOSCUDPServer((SERVER_IP, SERVER_PORT), dispatcher,
                                     asyncio.get_event_loop())
        transport, protocol = await server.create_serve_endpoint()  # Create datagram endpoint and start serving

        # Make async serial connection
        self.reader, self.writer = await serial_asyncio.open_serial_connection(url=get_radio().device, baudrate=115200)

        received = await self.recv(self.reader)
        await loop()
        transport.close()  # Clean up serve endpoint

    def send(self, *args: List[Any]) -> None:
        std_out (f'{args[0]}\n', 'BRIDGE')
        std_out (time.strftime("%H:%M:%S %d-%m-%Y"), 'BRIDGE')

        # We just remove the UDP_FILTER
        msg = args[0].strip(UDP_FILTER[:-1]).upper()
        address = msg[0:msg.index('/')]
        if address in names:
            msg = msg.replace(address, str(names[address]))

        std_out(f'Sending message: {msg}', 'BRIDGE')
        # TODO make it compatible with multiple
        # masks at the same time
        self.writer.write(f'{msg}\n'.encode('utf-8'))

    async def recv(self, r):
        while True:
            msg = await r.readline()
            try:
                decmsg = msg.rstrip().decode()
                std_out (decmsg, 'DEVICE')
            except:
                pass

bridge = Bridge()
dispatcher = Dispatcher()
# Filter OSC messages by "/mask"
dispatcher.map(UDP_FILTER, bridge.send)
# Run main bridge loop
asyncio.run(bridge.main(dispatcher))
