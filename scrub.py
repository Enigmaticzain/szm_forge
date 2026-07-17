import os

replacements = {
    "Turbine Assembly": "Structural Assembly",
    "Turbine Unit": "System Unit",
    "Gas Turbine": "Primary System",
    "Steam Turbine": "Secondary System",
    "Turbine Alpha": "System Alpha",
    "Turbine Beta": "System Beta",
    "Turbine": "System",
    "turbine": "system"
}

def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        try:
            content = f.read()
        except UnicodeDecodeError:
            return
            
    original = content
    for old, new in replacements.items():
        content = content.replace(old, new)
        
    if content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Updated {filepath}")

for root_dir in ['new ui/src', 'src']:
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(('.ts', '.tsx', '.cpp', '.hpp', '.js', '.css')):
                process_file(os.path.join(dirpath, filename))

