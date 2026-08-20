import sys
import re

def find_mismatch(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Strip comments and string literals
    content = re.sub(r'//.*', '', content)
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    content = re.sub(r'R"\((.*?)\)"', '', content, flags=re.DOTALL)
    content = re.sub(r'"(\\.|[^"\\])*"', '""', content)
    
    lines = content.split('\n')
    depth = 0
    for i, line in enumerate(lines):
        for char in line:
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
                if depth < 0:
                    print(f"Extra }} found at line {i+1}")
                    return

find_mismatch('src/WebAPI/FastAPIBridge.cpp')
