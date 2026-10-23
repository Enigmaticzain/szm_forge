from iot_bridge import get_iot_bridge


def test_iot_protocols():
    bridge = get_iot_bridge()
    protos = bridge.available_protocols()
    assert "mqtt" in protos
