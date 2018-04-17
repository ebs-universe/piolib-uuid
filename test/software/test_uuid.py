
from __future__ import print_function

from ebs.modbus.device import ModbusDevice
from ebs.modbus.hotplug import ModbusHotplugMixin
from ebs.ucdm.descriptor import ModbusDescriptorMixin
from ebs.ucdm.descriptor import _parse_string


class TestDevice(ModbusDescriptorMixin,
                 ModbusHotplugMixin,
                 ModbusDevice):
    _custom_descriptor_tags = {
        138: ('uuid_ns', _parse_string),
        139: ('uuid1', _parse_string),
        140: ('uuid2', _parse_string),
        141: ('uuid3', _parse_string),
        142: ('uuid4', _parse_string),
        143: ('uuid5', _parse_string),
        144: ('uuid6', _parse_string)
    }

    _modbus_address_regen = 15
    _versions = [1, 3, 4, 5, 6]

    def __init__(self, *args, **kwargs):
        super(TestDevice, self).__init__(*args, **kwargs)
        self._registry['descriptor_tags'].update(self._custom_descriptor_tags)

    def regen_uuid(self, ver):
        if ver not in self._versions:
            raise ValueError
        self.write_register(self._modbus_address_regen, ver)

    def print_uuid(self, ver):
        if ver not in self._versions:
            raise ValueError
        print('uuid{0}     : {1}'.format(
            ver, getattr(device, 'uuid{0}'.format(ver))
        ))

    def print_uuids(self):
        print('namespace : {0}'.format(device.uuid_ns))
        for ver in device._versions:
            self.print_uuid(ver)


if __name__ == '__main__':
    device = TestDevice(5, method='rtu', port='/dev/ttyACM4')
    device.connect()
    device.read_descriptors()
    device.print_uuids()

    print("Regenerating UUIDs")

    for ver in device._versions:
        device.regen_uuid(ver)

    device.read_descriptors()
    device.print_uuids()
