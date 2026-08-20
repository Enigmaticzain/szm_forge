import json
import os
from pathlib import Path

def convert_to_unsloth_format(input_json_path, output_jsonl_path):
    """
    Converts SZM Machine JSON data into ShareGPT/Alpaca JSONL format 
    compatible with Unsloth Studio for fine-tuning.
    """
    print(f"Reading machine data from {input_json_path}...")
    with open(input_json_path, 'r') as f:
        machine_data = json.load(f)
    
    # We will generate a few Q&A pairs for the model to learn from this machine
    training_examples = []
    
    # Example 1: General specification
    training_examples.append({
        "instruction": f"Generate a detailed specification for a {machine_data['type']} machine.",
        "input": "",
        "output": f"Here is the specification for the {machine_data['name']} (Model: {machine_data['model']}). It features capabilities in {', '.join(machine_data['capabilities'])}. The work area is {machine_data['specifications']['work_area']} with a spindle power of {machine_data['specifications']['spindle_power']} and a precision of {machine_data['specifications']['precision']}."
    })
    
    # Example 2: Component breakdown
    components_desc = "\n".join([f"- {c['name']} ({c['type']})" for c in machine_data['components']])
    training_examples.append({
        "instruction": f"What are the main components of the {machine_data['name']}?",
        "input": "",
        "output": f"The main components of the {machine_data['name']} are:\n{components_desc}"
    })
    
    # Write to JSONL
    print(f"Writing {len(training_examples)} training examples to {output_jsonl_path}...")
    with open(output_jsonl_path, 'w') as f:
        for example in training_examples:
            f.write(json.dumps(example) + '\n')
            
    print("\n✅ Dataset successfully prepared for Unsloth Studio!")
    print(f"Please upload '{output_jsonl_path}' in the 'Train' tab of Unsloth Studio.")

if __name__ == "__main__":
    base_dir = Path(__file__).resolve().parent.parent
    input_file = base_dir / "machine_data" / "sample_cnc_mill.json"
    output_file = base_dir / "machine_data" / "szm_unsloth_dataset.jsonl"
    
    convert_to_unsloth_format(input_file, output_file)
