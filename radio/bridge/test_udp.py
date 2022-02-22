import asyncio
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from typing import List, Any
from DMXEnttecPro import Controller

# OSC Server IP
SERVER_IP = "127.0.0.1"
SERVER_PORT = 5005
# UDP Message filter
UDP_FILTER = '/mask/*'
ENTTEC_SN = ''

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

        # port = get_port_by_serial_number(ENTTEC_SN)
        # self.dmx = Controller(port, auto_submit=True)

        # Make async serial connection
        await loop()
        transport.close()  # Clean up serve endpoint

    def send(self, *args: List[Any]) -> None:

        message = args[0].strip(UDP_FILTER[:-1]).strip('/prepalevel')
        argument = args[1]

        print (f'Getting {message} with arg {argument}')
        # self.dmx.set_channel(channel, intensity)

bridge = Bridge()
dispatcher = Dispatcher()
# Filter OSC messages by "UDP_FILTER"
dispatcher.map(UDP_FILTER, bridge.send)
# Run main bridge loop
asyncio.run(bridge.main(dispatcher))
