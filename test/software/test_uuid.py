
from __future__ import print_function
from six import iteritems

from pymodbus.mei_message import ReadDeviceInformationRequest

try:
    from pymodbus_ebs import ModbusClient
except ImportError:
    from pymodbus.client.sync import ModbusSerialClient as ModbusClient


class DummyResponse(object):
    more_follows = 0xFF
    next_object_id = 0


def get_descriptors(mc):
    rr = DummyResponse
    rval = {}
    while rr.more_follows:
        rq = ReadDeviceInformationRequest(unit=5, read_code=0x03,
                                          object_id=rr.next_object_id)
        rr = mc.execute(rq)
        rval.update(rr.information)
    return rval


if __name__ == '__main__':
    mclient = ModbusClient(method='rtu', port='/dev/ttyACM1')
    mclient.connect()

    device_information = get_descriptors(mclient)

    for k, v in iteritems(device_information):
        print((k - 0x80), v)
