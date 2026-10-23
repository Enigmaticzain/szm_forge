"""Digital twin / IoT protocol bridge (Group 9)."""
from __future__ import annotations

import json
import logging
import threading
import time
from typing import Any, Callable, Dict, List, Optional

logger = logging.getLogger("IoTBridge")

_HAS_MQTT = False
_HAS_OPCUA = False
_HAS_MODBUS = False

try:
    import paho.mqtt.client as mqtt
    _HAS_MQTT = True
except ImportError:
    mqtt = None

try:
    from opcua import Client as OpcuaClient
    _HAS_OPCUA = True
except ImportError:
    OpcuaClient = None

try:
    from pymodbus.client import ModbusTcpClient
    _HAS_MODBUS = True
except ImportError:
    ModbusTcpClient = None


class IoTBridge:
    def __init__(self):
        self._mqtt_client = None
        self._callbacks: List[Callable[[Dict[str, Any]], None]] = []
        self._running = False

    def available_protocols(self) -> Dict[str, bool]:
        return {"mqtt": _HAS_MQTT, "opcua": _HAS_OPCUA, "modbus": _HAS_MODBUS}

    def on_message(self, callback: Callable[[Dict[str, Any]], None]) -> None:
        self._callbacks.append(callback)

    def _dispatch(self, payload: Dict[str, Any]) -> None:
        for cb in self._callbacks:
            try:
                cb(payload)
            except Exception as exc:
                logger.error("IoT callback error: %s", exc)

    def start_mqtt_subscriber(
        self,
        broker: str = "localhost",
        port: int = 1883,
        topic: str = "szm/telemetry/#",
    ) -> bool:
        if not _HAS_MQTT:
            return False

        def on_connect(client, userdata, flags, rc, properties=None):
            client.subscribe(topic)

        def on_msg(client, userdata, msg):
            try:
                data = json.loads(msg.payload.decode())
            except json.JSONDecodeError:
                data = {"raw": msg.payload.decode(), "topic": msg.topic}
            data["topic"] = msg.topic
            data["timestamp"] = time.time()
            self._dispatch(data)

        self._mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self._mqtt_client.on_connect = on_connect
        self._mqtt_client.on_message = on_msg
        try:
            self._mqtt_client.connect(broker, port, 60)
            threading.Thread(target=self._mqtt_client.loop_forever, daemon=True).start()
            self._running = True
            return True
        except Exception as exc:
            logger.warning("MQTT connect failed: %s", exc)
            return False

    def read_opcua(self, endpoint: str, node_id: str) -> Dict[str, Any]:
        if not _HAS_OPCUA:
            return {"status": "error", "message": "opcua not installed"}
        client = OpcuaClient(endpoint)
        try:
            client.connect()
            val = client.get_node(node_id).get_value()
            return {"status": "success", "node_id": node_id, "value": val}
        finally:
            client.disconnect()

    def read_modbus(self, host: str, port: int = 502, address: int = 0, count: int = 1) -> Dict[str, Any]:
        if not _HAS_MODBUS:
            return {"status": "error", "message": "pymodbus not installed"}
        client = ModbusTcpClient(host, port=port)
        try:
            client.connect()
            result = client.read_holding_registers(address, count)
            regs = result.registers if not result.isError() else []
            return {"status": "success", "registers": regs}
        finally:
            client.close()


_bridge: Optional[IoTBridge] = None


def get_iot_bridge() -> IoTBridge:
    global _bridge
    if _bridge is None:
        _bridge = IoTBridge()
    return _bridge
