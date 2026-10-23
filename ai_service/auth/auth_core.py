"""JWT authentication utilities for SZM Forge AI Service."""
import os
import hashlib
import hmac
import json
import time
import base64
from typing import Optional, Dict, Any

SECRET_KEY = os.getenv("SZM_JWT_SECRET", "szm-forge-dev-secret-change-in-production")
TOKEN_EXPIRE_SECONDS = int(os.getenv("SZM_JWT_EXPIRE", "86400"))  # 24h

# ── In-memory user store (replace with DB in production) ──────────────────────
# { username: { "password_hash": str, "projects": [str] } }
_users: Dict[str, Dict[str, Any]] = {}


def _hash_password(password: str) -> str:
    salt = os.urandom(16).hex()
    h = hmac.new(salt.encode(), password.encode(), hashlib.sha256).hexdigest()
    return f"{salt}:{h}"


def _verify_password(password: str, stored: str) -> bool:
    try:
        salt, h = stored.split(":", 1)
        return hmac.compare_digest(
            hmac.new(salt.encode(), password.encode(), hashlib.sha256).hexdigest(), h
        )
    except Exception:
        return False


def _b64url(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()


def _sign(msg: str) -> str:
    return _b64url(hmac.new(SECRET_KEY.encode(), msg.encode(), hashlib.sha256).digest())


def create_token(username: str) -> str:
    header = _b64url(json.dumps({"alg": "HS256", "typ": "JWT"}).encode())
    payload = _b64url(json.dumps({"sub": username, "exp": int(time.time()) + TOKEN_EXPIRE_SECONDS}).encode())
    sig = _sign(f"{header}.{payload}")
    return f"{header}.{payload}.{sig}"


def verify_token(token: str) -> Optional[str]:
    """Returns username if valid, else None."""
    try:
        header, payload, sig = token.split(".")
        if not hmac.compare_digest(_sign(f"{header}.{payload}"), sig):
            return None
        data = json.loads(base64.urlsafe_b64decode(payload + "=="))
        if data.get("exp", 0) < time.time():
            return None
        return data.get("sub")
    except Exception:
        return None


# ── User CRUD ──────────────────────────────────────────────────────────────────

def register_user(username: str, password: str) -> bool:
    """Returns False if username already taken."""
    if username in _users:
        return False
    _users[username] = {"password_hash": _hash_password(password), "projects": []}
    return True


def authenticate_user(username: str, password: str) -> bool:
    user = _users.get(username)
    if not user:
        return False
    return _verify_password(password, user["password_hash"])


def get_user_projects(username: str) -> list:
    return _users.get(username, {}).get("projects", [])


def add_user_project(username: str, project_name: str) -> bool:
    user = _users.get(username)
    if not user:
        return False
    if project_name not in user["projects"]:
        user["projects"].append(project_name)
    return True


def remove_user_project(username: str, project_name: str) -> bool:
    user = _users.get(username)
    if not user:
        return False
    user["projects"] = [p for p in user["projects"] if p != project_name]
    return True
