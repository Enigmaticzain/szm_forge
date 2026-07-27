#!/usr/bin/env python3
"""Lightweight web-status server for SZM Forge.

This module is intentionally dependency-free so it can be used in minimal
environments or as a simple health endpoint for local development.
"""

from __future__ import annotations

import argparse
import json
import os
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Optional, Sequence

DEFAULT_HOST = os.getenv("SZM_FORGE_HOST", "127.0.0.1")
DEFAULT_PORT = int(os.getenv("SZM_FORGE_PORT", "8000"))


class ForgeRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self) -> None:  # noqa: N802
        if self.path == "/health":
            self._write_json(build_health_payload())
            return

        if self.path in {"/", "/status"}:
            self._write_html(build_status_page())
            return

        self.send_error(404, "Not Found")

    def log_message(self, format: str, *args) -> None:  # noqa: A003
        return

    def _write_json(self, payload: dict, status: int = 200) -> None:
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _write_html(self, html: str, status: int = 200) -> None:
        body = html.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def build_health_payload() -> dict:
    return {
        "status": "ok",
        "project": "SZM Forge",
        "service": "web-status",
        "version": "0.3.0",
    }


def build_status_page() -> str:
    payload = build_health_payload()
    return f"""<!DOCTYPE html>
<html lang=\"en\">
  <head>
    <meta charset=\"utf-8\">
    <title>{payload['project']} Status</title>
    <style>
      body {{ font-family: Arial, sans-serif; margin: 2rem; }}
      code {{ background: #f4f4f4; padding: 0.15rem 0.3rem; }}
    </style>
  </head>
  <body>
    <h1>{payload['project']}</h1>
    <p>The lightweight status service is running.</p>
    <p>Health endpoint: <code>/health</code></p>
    <p>Status: <code>{payload['status']}</code></p>
  </body>
</html>"""


def parse_args(argv: Optional[Sequence[str]] = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run the lightweight SZM Forge status server")
    parser.add_argument("--host", default=DEFAULT_HOST, help="Host interface to bind to")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help="Port to listen on")
    return parser.parse_args(list(argv) if argv is not None else None)


def create_server(host: str = DEFAULT_HOST, port: int = DEFAULT_PORT) -> ThreadingHTTPServer:
    return ThreadingHTTPServer((host, port), ForgeRequestHandler)


def main(argv: Optional[Sequence[str]] = None) -> None:
    args = parse_args(argv)
    httpd = create_server(host=args.host, port=args.port)
    print(f"Starting SZM Forge status server on http://{args.host}:{args.port}")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down SZM Forge status server")
    finally:
        httpd.server_close()


if __name__ == "__main__":
    main()
