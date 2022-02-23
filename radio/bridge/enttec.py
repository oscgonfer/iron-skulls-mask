import asyncio
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from typing import List, Any
from DMXEnttecPro import Controller
from DMXEnttecPro.utils import get_port_by_serial_number
import time
from patch import patch, personality

# OSC Server IP
SERVER_IP = "127.0.0.1"
SERVER_PORT = 6000
# UDP Message filter
CHANNEL_FILTER = '/circ/'
LED_FILTER = '/device/'
ENTTEC_SN = 'EN210410'



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

        port = get_port_by_serial_number(ENTTEC_SN)
        self.dmx = Controller(port, auto_submit=True)
        self.dmx.set_dmx_parameters(output_rate=0)

        # Make async serial connection
        await loop()
        transport.close()  # Clean up serve endpoint

    def send(self, *args: List[Any]) -> None:

        if CHANNEL_FILTER in args[0]:
            if 'prepalevel' not in args[0]:
                channel = int(args[0].strip(CHANNEL_FILTER).strip('/prepalevel'))
                intensity = args[1]
                if channel in patch:
                    output = patch[channel]
                    # print (f'Setting {output} at {intensity}')
                    self.dmx.set_channel(int(output), int(intensity))


        elif LED_FILTER in args[0]:
            msg = (args[0].strip(LED_FILTER).split('/'))
            channel = int(msg[0])
            colour = msg[2]
            intensity = round(args[1]*256/65536)
            if colour in personality:
                if channel in patch:
                    output = patch[channel]+personality[colour]
                    # print (f'Setting {output} at {intensity}')
                    self.dmx.set_channel(int(output), int(intensity))

bridge = Bridge()
dispatcher = Dispatcher()
# Filter OSC messages by "UDP_FILTER"
dispatcher.map('/*', bridge.send)
# Run main bridge loop
asyncio.run(bridge.main(dispatcher))
