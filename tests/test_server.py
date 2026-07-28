import json
import sys
import threading
import unittest
import urllib.request
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import server


class ServerTests(unittest.TestCase):
    def setUp(self):
        self.httpd = server.create_server(host="127.0.0.1", port=0)
        self.thread = threading.Thread(target=self.httpd.serve_forever, daemon=True)
        self.thread.start()
        self.base_url = f"http://127.0.0.1:{self.httpd.server_address[1]}"

    def tearDown(self):
        self.httpd.shutdown()
        self.httpd.server_close()
        self.thread.join(timeout=2)

    def fetch(self, path):
        with urllib.request.urlopen(f"{self.base_url}{path}") as response:
            body = response.read().decode("utf-8")
            return response.status, body

    def test_health_endpoint_returns_project_status(self):
        status, body = self.fetch("/health")
        self.assertEqual(status, 200)
        payload = json.loads(body)
        self.assertEqual(payload["status"], "ok")
        self.assertEqual(payload["project"], "SZM Forge")
        self.assertEqual(payload["service"], "web-status")

    def test_root_endpoint_contains_product_name(self):
        status, body = self.fetch("/")
        self.assertEqual(status, 200)
        self.assertIn("SZM Forge", body)

    def test_parse_args_supports_custom_port(self):
        args = server.parse_args(["--host", "0.0.0.0", "--port", "9000"])
        self.assertEqual(args.host, "0.0.0.0")
        self.assertEqual(args.port, 9000)


if __name__ == "__main__":
    unittest.main()
