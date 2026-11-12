import os
import subprocess
import shutil
import json
from datetime import datetime

try:
    from rich import print
except ImportError:
    pass

MODEL = "llama3.1"

MEMORY_FILE = ".agent_memory.json"


# ---------- LLM ----------
def ask_llm(prompt):
    if not shutil.which("ollama"):
        return "Error: ollama executable not found in PATH."
    try:
        result = subprocess.run(
            ["ollama", "run", MODEL],
            input=prompt.encode(),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        return result.stdout.decode()
    except Exception as e:
        return f"Error executing ollama: {e}"


# ---------- MEMORY ----------
def load_memory():
    if os.path.exists(MEMORY_FILE):
        try:
            with open(MEMORY_FILE, "r") as f:
                return json.load(f)
        except Exception:
            return []
    return []

def save_memory(memory):
    try:
        with open(MEMORY_FILE, "w") as f:
            json.dump(memory, f, indent=2)
    except Exception as e:
        print(f"[red]Error saving memory: {e}[/red]")


# ---------- FILE TOOLS ----------
def scan_project():
    files = []
    for root, _, filenames in os.walk("."):
        if any(ignored in root for ignored in [".git", "build", "node_modules", ".gemini"]):
            continue
        for f in filenames:
            if f.endswith((".py", ".cpp", ".hpp", ".h", ".c", ".js", ".ts", ".json")):
                files.append(os.path.join(root, f))
    return files


def read_file(path):
    try:
        with open(path, "r", errors="ignore") as f:
            return f.read()
    except:
        return ""


def write_file(path, content):
    with open(path, "w") as f:
        f.write(content)
    print(f"[green]✔ Updated:[/green] {path}")


# ---------- AGENT CORE ----------
def planner(task, memory, files):
    prompt = f"""
You are an autonomous AI coding agent.

Task:
{task}

Project Files:
{files}

Memory:
{memory}

Return ONLY JSON:

{{
  "plan": ["step1", "step2"],
  "actions": [
    {{
      "type": "read_file",
      "path": "file.py"
    }},
    {{
      "type": "write_file",
      "path": "file.py",
      "content": "new code"
    }}
  ]
}}
"""
    response = ask_llm(prompt)

    try:
        return json.loads(response)
    except:
        print("[red]Planner failed, raw output:[/red]\n", response)
        return None


def executor(actions):
    results = []

    for act in actions:
        print(f"[yellow]→ {act['type']}[/yellow]")

        if act["type"] == "read_file":
            content = read_file(act["path"])
            results.append({"file": act["path"], "content": content[:1000]})

        elif act["type"] == "write_file":
            write_file(act["path"], act["content"])
            results.append({"file": act["path"], "status": "written"})

    return results


# ---------- MAIN LOOP ----------
def agent():
    print("[bold green]SZM FORGE AGENT v2 STARTED[/bold green]")

    memory = load_memory()

    while True:
        task = input("\n[Task] > ")

        print("[blue]Scanning project...[/blue]")
        files = scan_project()

        print("[blue]Planning...[/blue]")
        plan_data = planner(task, memory, files)

        if not plan_data:
            continue

        print("\n[cyan]PLAN:[/cyan]")
        for step in plan_data["plan"]:
            print(" -", step)

        print("\n[magenta]EXECUTING...[/magenta]")
        results = executor(plan_data["actions"])

        memory.append({
            "task": task,
            "time": str(datetime.now()),
            "result": results
        })
        save_memory(memory)

        print("[green]✔ Done[/green]")


if __name__ == "__main__":
    agent()
