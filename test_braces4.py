import sys, re

def check_indent(filename):
    with open(filename, "r") as f:
        content = f.read()

    clean_content = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), content, flags=re.DOTALL)
    clean_content = re.sub(r"//.*", lambda m: " " * len(m.group(0)), clean_content)
    clean_content = re.sub(r"\"(\\\\\"|[^\"])*\"", lambda m: " " * len(m.group(0)), clean_content)
    
    stack = 0
    lines = clean_content.split("\n")
    for i, line in enumerate(lines):
        line_stripped = line.strip()
        if not line_stripped: continue
        
        stack -= line_stripped.count("}")
        stack += line_stripped.count("{")

    print(f"Final stack depth: {stack}")

check_indent("src/WebAPI/FastAPIBridge.cpp")
