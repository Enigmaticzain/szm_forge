import sys, re

def check_indent(filename):
    with open(filename, "r") as f:
        content = f.read()

    clean_content = re.sub(r"/\*.*?\*/", lambda m: " " * len(m.group(0)), content, flags=re.DOTALL)
    clean_content = re.sub(r"//.*", lambda m: " " * len(m.group(0)), clean_content)
    clean_content = re.sub(r"\"(\\\\\"|[^\"])*\"", lambda m: " " * len(m.group(0)), clean_content)
    
    stack = 0
    lines = clean_content.split("\n")
    orig_lines = content.split("\n")
    for i, line in enumerate(lines):
        line_stripped = line.strip()
        if not line_stripped: continue
        
        # update stack for closing braces on this line before checking indent
        closing_count = line_stripped.count("}")
        
        expected_indent = (stack - closing_count) * 4
        
        # calculate actual indent
        actual_indent = len(orig_lines[i]) - len(orig_lines[i].lstrip(" "))
        
        if expected_indent >= 0 and abs(expected_indent - actual_indent) > 0:
            if not orig_lines[i].strip().startswith("public:") and not orig_lines[i].strip().startswith("private:") and not orig_lines[i].strip().startswith("case "):
                print(f"Line {i+1} mismatch: expected {expected_indent}, got {actual_indent} -> {orig_lines[i].strip()}")
        
        # update stack for opening braces on this line
        stack -= closing_count
        stack += line_stripped.count("{")

check_indent("src/WebAPI/FastAPIBridge.cpp")
