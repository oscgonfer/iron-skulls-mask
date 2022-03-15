import asyncio
from pythonosc.dispatcher import Dispatcher
from pythonosc.osc_server import AsyncIOOSCUDPServer
from pythonosc.udp_client import SimpleUDPClient
from typing import List, Any
import time
import sys

# OSC Server IP
SERVER_IP = "127.0.0.1"
BRIDGE_SERVER_PORT = 3032
DLIGHT_SERVER_PORT = 7000

UDP_FILTER = '/eos/user/0/chan/*'

# Associate frequencies in sound2light to channels
channels_freq = {
    1: [50, 59, 68, 77, 86, 95, 104, 113, 122],
    2: [30, 31, 32, 33, 34, 35]
}

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

    async def main(self, dispatcher):

        server = AsyncIOOSCUDPServer((SERVER_IP, BRIDGE_SERVER_PORT), dispatcher,
                                    asyncio.get_event_loop())
        transport, protocol = await server.create_serve_endpoint()  # Create datagram endpoint and start serving

        print ('Bridge open')
        await loop()
        protocol.close()  # Clean up serve endpoint

    def send(self, *args: List[Any]) -> None:
        msg = args[0].strip(UDP_FILTER[:-1]).upper()
        argument = args[1]
        message = round(scale(args[1], (0, 100), (100, 250)))
        for chan in channels_freq:
            if int(msg) == chan:
                for k in channels_freq[chan]:
                    client.send_message(f"/circ/{k}/level", message)

bridge = Bridge()
dispatcher = Dispatcher()
# Filter OSC messages by "UDP_FILTER"
dispatcher.map(UDP_FILTER, bridge.send)
# Run main bridge loop
asyncio.run(bridge.main(dispatcher))
