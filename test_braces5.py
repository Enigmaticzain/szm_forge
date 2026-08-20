import sys, re

def check_braces(filename):
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
        if stack == 0 and i > 22 and i < 1160:
            print(f"Stack dropped to 0 at line {i+1}: {line}")
        stack += line_stripped.count("{")

check_braces("src/WebAPI/FastAPIBridge.cpp")
