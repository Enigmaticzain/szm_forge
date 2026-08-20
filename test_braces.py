import sys

def check_braces(filename):
    with open(filename, "r") as f:
        content = f.read()

    # super naive: just strip out comments and string literals
    import re
    # remove block comments
    content = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), content, flags=re.DOTALL)
    # remove line comments
    content = re.sub(r"//.*", lambda m: " " * len(m.group(0)), content)
    # remove strings (replace with spaces to keep index)
    content = re.sub(r"\"(\\\\\"|[^\"])*\"", lambda m: " " * len(m.group(0)), content)
    
    stack = []
    lines = content.split("\n")
    for i, line in enumerate(lines):
        for j, char in enumerate(line):
            if char == "{":
                stack.append(i + 1)
            elif char == "}":
                if stack:
                    stack.pop()
                else:
                    print(f"Unmatched }} at line {i + 1}")
                    return

    if stack:
        print("Unmatched { at:")
        for loc in stack:
            print(f"Line {loc}")
    else:
        print("Braces are balanced.")

check_braces("src/WebAPI/FastAPIBridge.cpp")
