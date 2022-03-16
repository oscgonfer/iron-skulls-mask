import asyncio
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from pythonosc.udp_client import SimpleUDPClient
from patch import channels_freq, ranges_freq
from typing import List, Any
import time
import sys

# OSC Server IP
SERVER_IP = "127.0.0.1"
BRIDGE_SERVER_PORT = 3032
DLIGHT_SERVER_PORT = 7000
UDP_FILTER = '/eos/user/0/chan/*'

client = SimpleUDPClient(SERVER_IP, DLIGHT_SERVER_PORT)  # Create client
print ('Client active')

def scale(val, src, dst):
    """
    Scale the given value from the scale of src to the scale of dst.
    """
    return ((val - src[0]) / (src[1]-src[0])) * (dst[1]-dst[0]) + dst[0]

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
        self.state = 0

    async def main(self, dispatcher):

        server = AsyncIOOSCUDPServer((SERVER_IP, BRIDGE_SERVER_PORT), dispatcher,
                                    asyncio.get_event_loop())
        transport, protocol = await server.create_serve_endpoint()  # Create datagram endpoint and start serving

        print ('Bridge open')
        await loop()
        protocol.close()  # Clean up serve endpoint

    def send(self, *args: List[Any]) -> None:
        msg = args[0].replace(UDP_FILTER[:-1], "").upper()
        argument = args[1]

        if msg == 'STATE':
            self.state = int(argument)
            print (f'Bridge state: {self.state}')
        else:
            if self.state:
                for chan in channels_freq[self.state]:
                    if int(msg) == chan:
                        for k in channels_freq[self.state][chan]:
                            message = round(scale(args[1], (0, 100),\
                                (ranges_freq[chan][0], ranges_freq[chan][1])))
                            client.send_message(f"/circ/{k}/level", message)

bridge = Bridge()
dispatcher = Dispatcher()
# Filter OSC messages by "UDP_FILTER"
dispatcher.map(UDP_FILTER, bridge.send)
# Run main bridge loop
asyncio.run(bridge.main(dispatcher))
