import serial
import glob
import sys
import serial.tools.list_ports

def get_radio():
    p = serial.tools.list_ports.comports()

    controller = None
    for port in p:
        if 'Feather M0' in port.description:
            # TODO Maybe make something else to detect that is a controller via serial
            print (f'[BRIDGE] Controller {port.description} found')
            controller = port
    return controller

def serial_ports():
    """Lists serial ports
    :raises EnvironmentError:
        On unsupported or unknown platforms
    :returns:
        A list of available serial ports
    """
    if sys.platform.startswith('win'):
        ports = ['COM' + str(i + 1) for i in range(256)]

    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this is to exclude your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')

    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')

    else:
        raise EnvironmentError('Unsupported platform')

    result = []

    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

