import os
import subprocess
from rich import print

MODEL = "llama3.1"

def ask_llm(prompt):
    result = subprocess.run(
        ["ollama", "run", MODEL],
        input=prompt.encode(),
        stdout=subprocess.PIPE
    )
    return result.stdout.decode()

def read_project():
print(f"[blue]Reading: {path}[/blue]")
    files_data = ""
    for root, _, files in os.walk("."):
        for f in files:
            if f.endswith((".py", ".js", ".ts", ".json")):
                path = os.path.join(root, f)
                try:
                    with open(path, "r", errors="ignore") as file:
                        content = file.read()
                        files_data += f"\nFILE: {path}\n{content[:1000]}\n"
                except:
                    pass
    return files_data

def agent_loop():
    print("[green]SZM Forge Agent Started[/green]")

    while True:
        task = input("\n[Task] > ")

        print("[yellow]Step 1: Reading project files...[/yellow]")
        project_data = read_project()

        print("[yellow]Step 2: Sending to LLM...[/yellow]")

        prompt = f"""
You are a senior AI software architect.

Task:
{task}

Project Snapshot:
{project_data}

Steps:
1. Understand
2. Plan
3. Show step-by-step reasoning
4. Suggest exact code changes

IMPORTANT:
Always respond in this format:

[PLAN]
...

[FILES TO MODIFY]
...

[CODE]
...
"""

        response = ask_llm(prompt)

        print("[yellow]Step 3: Response received[/yellow]\n")
        print("[cyan]" + response + "[/cyan]")
