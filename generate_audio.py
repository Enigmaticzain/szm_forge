import os
from gtts import gTTS
import subprocess
import json

script_parts = [
    {"id": "intro", "text": "As a mechanical engineering student, I realized that building and testing a machine requires physical components, tools, space, time, and money. I wanted to know: what if we could experiment with machines virtually before building them? This is SZM Forge, an AI-powered engineering environment I'm building to bring design, simulation, analysis, and intelligent engineering workflows together."},
    {"id": "cad", "text": "Here, we can design components directly in the browser, instantly interacting with 3D primitives and adjusting their parameters in real-time."},
    {"id": "sim", "text": "We can then test these components using our built-in simulation engine, visualizing stress, heat flux, and boundary conditions instantly."},
    {"id": "ai", "text": "And with the SZM AI Assistant, we can even generate complex designs, like a new motor, simply by asking for it. The AI understands our engineering context and creates the necessary parameters."},
    {"id": "dt", "text": "This is still a prototype, but the goal is to make engineering experimentation more accessible. Instead of immediately spending physical resources on an idea, engineers could design, test, analyze, and improve it virtually first."}
]

def get_duration(filename):
    result = subprocess.run(["ffprobe", "-v", "error", "-show_entries",
                             "format=duration", "-of",
                             "default=noprint_wrappers=1:nokey=1", filename],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT)
    return float(result.stdout)

durations = {}

for part in script_parts:
    tts = gTTS(text=part["text"], lang='en', slow=False)
    filename = f"{part['id']}.mp3"
    tts.save(filename)
    duration = get_duration(filename)
    durations[part["id"]] = duration
    print(f"Generated {filename} (Duration: {duration:.2f}s)")

with open("audio_durations.json", "w") as f:
    json.dump(durations, f)
