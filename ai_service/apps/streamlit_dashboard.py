"""Streamlit dashboard for simulation results (Group 14)."""
from __future__ import annotations

try:
    import streamlit as st
except ImportError:
    raise SystemExit("streamlit required: pip install streamlit")

import ipaddress
import os
from urllib.parse import urlparse
import requests


def _validate_bridge_url(raw_url: str) -> str:
    parsed = urlparse(raw_url.strip())
    if parsed.scheme not in {"http", "https"} or not parsed.hostname:
        raise ValueError("Bridge URL must be a valid http:// or https:// URL")
    host = parsed.hostname.lower()
    allowed_hosts = {"127.0.0.1", "localhost", "::1"}
    extra = os.environ.get("SZM_ALLOWED_BRIDGE_HOSTS", "")
    for item in extra.split(","):
        if item.strip():
            allowed_hosts.add(item.strip().lower())
    try:
        ip = ipaddress.ip_address(host)
        if ip.is_link_local or ip.is_unspecified:
            raise ValueError(f"Disallowed bridge host IP: {host}")
    except ValueError as exc:
        if "Disallowed" in str(exc):
            raise
    if host not in allowed_hosts:
        raise ValueError(f"Bridge host '{host}' is not in the allowlist")
    return raw_url.strip().rstrip("/")


st.set_page_config(page_title="SZM Forge Dashboard", layout="wide")
st.title("SZM Forge Engineering Dashboard")

bridge_url_input = st.sidebar.text_input("Bridge URL", "http://127.0.0.1:8003")

if st.button("Check Stack Health"):
    try:
        bridge_url = _validate_bridge_url(bridge_url_input)
        r = requests.get(f"{bridge_url}/stack/health", timeout=5)
        r.raise_for_status()
        data = r.json()
        st.metric("Tools Available", f"{data.get('available', 0)}/{data.get('total_tools', 0)}")
        st.json(data)
    except Exception as exc:
        st.error(str(exc))

st.subheader("Quick FEA")
force = st.number_input("Force (N)", value=10000.0)
if st.button("Run FEA"):
    try:
        bridge_url = _validate_bridge_url(bridge_url_input)
        r = requests.post(
            f"{bridge_url}/simulation/fea/run",
            json={"component_id": 1, "force_n": force, "area_m2": 0.01, "thickness_m": 0.05},
            timeout=30,
        )
        r.raise_for_status()
        st.json(r.json())
    except Exception as exc:
        st.error(str(exc))
