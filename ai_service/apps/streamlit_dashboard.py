"""Streamlit dashboard for simulation results (Group 14)."""
from __future__ import annotations

try:
    import streamlit as st
except ImportError:
    raise SystemExit("streamlit required: pip install streamlit")

import json
import requests

st.set_page_config(page_title="SZM Forge Dashboard", layout="wide")
st.title("SZM Forge Engineering Dashboard")

bridge_url = st.sidebar.text_input("Bridge URL", "http://127.0.0.1:8003")

if st.button("Check Stack Health"):
    try:
        r = requests.get(f"{bridge_url}/stack/health", timeout=5)
        data = r.json()
        st.metric("Tools Available", f"{data.get('available', 0)}/{data.get('total_tools', 0)}")
        st.json(data)
    except Exception as exc:
        st.error(str(exc))

st.subheader("Quick FEA")
force = st.number_input("Force (N)", value=10000.0)
if st.button("Run FEA"):
    try:
        r = requests.post(
            f"{bridge_url}/simulation/fea/run",
            json={"component_id": 1, "force_n": force, "area_m2": 0.01, "thickness_m": 0.05},
            timeout=30,
        )
        st.json(r.json())
    except Exception as exc:
        st.error(str(exc))
