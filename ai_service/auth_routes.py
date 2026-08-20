"""Auth routes: /auth/register, /auth/login, /auth/me, /auth/projects"""
from fastapi import APIRouter, HTTPException, Depends
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from pydantic import BaseModel, Field
from typing import Optional
from auth import (
    register_user, authenticate_user, create_token, verify_token,
    get_user_projects, add_user_project, remove_user_project,
)

router = APIRouter(prefix="/auth", tags=["auth"])
_bearer = HTTPBearer(auto_error=False)


# ── Request / Response models ──────────────────────────────────────────────────

class RegisterRequest(BaseModel):
    username: str = Field(min_length=3, max_length=32)
    password: str = Field(min_length=6)

class LoginRequest(BaseModel):
    username: str
    password: str

class TokenResponse(BaseModel):
    access_token: str
    token_type: str = "bearer"
    username: str

class ProjectRequest(BaseModel):
    project_name: str


# ── Dependency: resolve current user from Bearer token ────────────────────────

def get_current_user(
    creds: Optional[HTTPAuthorizationCredentials] = Depends(_bearer),
) -> Optional[str]:
    if not creds:
        return None
    return verify_token(creds.credentials)


def require_user(
    creds: Optional[HTTPAuthorizationCredentials] = Depends(_bearer),
) -> str:
    username = get_current_user(creds)
    if not username:
        raise HTTPException(status_code=401, detail="Invalid or expired token")
    return username


# ── Endpoints ──────────────────────────────────────────────────────────────────

@router.post("/register", response_model=TokenResponse)
def register(req: RegisterRequest):
    if not register_user(req.username, req.password):
        raise HTTPException(status_code=409, detail="Username already taken")
    token = create_token(req.username)
    return TokenResponse(access_token=token, username=req.username)


@router.post("/login", response_model=TokenResponse)
def login(req: LoginRequest):
    if not authenticate_user(req.username, req.password):
        raise HTTPException(status_code=401, detail="Invalid credentials")
    token = create_token(req.username)
    return TokenResponse(access_token=token, username=req.username)


@router.get("/me")
def me(username: str = Depends(require_user)):
    return {"username": username, "projects": get_user_projects(username)}


@router.get("/projects")
def list_projects(username: str = Depends(require_user)):
    return {"projects": get_user_projects(username)}


@router.post("/projects")
def create_project(req: ProjectRequest, username: str = Depends(require_user)):
    add_user_project(username, req.project_name)
    return {"projects": get_user_projects(username)}


@router.delete("/projects/{project_name}")
def delete_project(project_name: str, username: str = Depends(require_user)):
    remove_user_project(username, project_name)
    return {"projects": get_user_projects(username)}
