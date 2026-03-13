# Container Documentation Template Generator
# Run: python generate_docs.py "Container Name" "Category"

import sys
import os
import re

def extract_container_info(container_name):
    """Extract all relevant information for a container"""
    
    # Convert to various case formats
    lower_name = container_name.lower().replace(' ', '')
    camel_name = ''.join(word.capitalize() for word in container_name.split())
    
    info = {
        'name': container_name,
        'lower_name': lower_name,
        'camel_name': camel_name,
        'header': None,
        'implementation': None,
        'ports': [],
        'parameters': [],
        'defaults': {},
        'category': None,
        'description': None
    }
    
    # Find header file
    header_files = [f for f in os.listdir('.') if f.endswith('.h') and lower_name in f.lower()]
    if header_files:
        info['header'] = header_files[0]
        with open(header_files[0], 'r') as f:
            header_content = f.read()
            info['description'] = extract_description(header_content)
            info['parameters'] = extract_parameters(header_content)
    
    # Find implementation file  
    impl_files = [f for f in os.listdir('.') if f.endswith('.cpp') and lower_name in f.lower()]
    if impl_files:
        info['implementation'] = impl_files[0]
        
    # Extract ports from canvas.cpp
    with open('canvas.cpp', 'r') as f:
        canvas_content = f.read()
        info['ports'] = extract_ports(canvas_content, container_name)
    
    # Extract defaults from sounitbuilder.cpp
    with open('sounitbuilder.cpp', 'r') as f:
        builder_content = f.read()
        info['defaults'] = extract_defaults(builder_content, container_name)
    
    # Extract category from specification
    with open('docs/md format/Container Port Specification v1.2.md', 'r') as f:
        spec_content = f.read()
        info['category'] = extract_category(spec_content, container_name)
    
    return info

def extract_description(content):
    """Extract description from header file comment"""
    # Look for class comment block
    class_match = re.search(r'/\*\*\s*\n(.*?)\s*\n\s*\*/\s*\nclass', content, re.DOTALL)
    if class_match:
        return class_match.group(1).strip()
    return "No description found"

def extract_parameters(content):
    """Extract parameter methods from header"""
    params = []
    # Look for getter/setter methods
    setter_match = re.findall(r'set(\w+)\s*\([^)]*\)', content)
    for param in setter_match:
        params.append(param.lower())
    return params

def extract_ports(content, container_name):
    """Extract port definitions from canvas.cpp"""
    ports = []
    # Look for container port definitions
    pattern = f'if \\(type == "{container_name}"\\) \\{{\\s*inputs = \\{{([^}}]*)\\}}'
    match = re.search(pattern, content, re.DOTALL)
    if match:
        inputs = [p.strip().strip('"') for p in match.group(1).split(',')]
        ports.extend([('input', p) for p in inputs])
    
    pattern = f'outputs = \\{{([^}}]*)\\}}'
    match = re.search(pattern, content, re.DOTALL)
    if match:
        outputs = [p.strip().strip('"') for p in match.group(1).split(',')]
        ports.extend([('output', p) for p in outputs])
    
    return ports

def extract_defaults(content, container_name):
    """Extract default parameter values from sounitbuilder.cpp"""
    defaults = {}
    pattern = f'if \\(name == "{container_name}"\\) \\{{(.*?)\\}}'
    match = re.search(pattern, content, re.DOTALL)
    if match:
        setparam_matches = re.findall(r'setParameter\\("([^"]+)",\\s*([^)]+)\\)', match.group(1))
        for param, value in setparam_matches:
            defaults[param] = value.rstrip(';')
    return defaults

def extract_category(content, container_name):
    """Extract category from specification"""
    # Look for container in spec and find nearest category
    lines = content.split('\n')
    current_category = None
    for i, line in enumerate(lines):
        if 'Category' in line:
            current_category = line.strip()
        elif container_name in line and current_category:
            return current_category
    return "Unknown"

def generate_markdown(container_info, category=None):
    """Generate complete markdown documentation"""
    
    category = category or container_info.get('category', 'Unknown')
    category_color = {
        'Essential': 'Blue',
        'Shaping': 'Orange', 
        'Modifiers': 'Green'
    }.get(category, 'Unknown')
    
    md = f"""# {container_info['name']} Documentation

## Overview

{container_info.get('description', 'No description available.')}

## Category

**{category}** ({category_color}) - {'Required for any sound generation' if category == 'Essential' else 'Transforms sound character' if category == 'Shaping' else 'Affects how parameters change over time'}

## Ports

### Input Ports

"""
    
    # Add input ports
    input_ports = [p for p, t in container_info['ports'] if p == 'input']
    if input_ports:
        md += "| Port Name | Data Type | Default | Description |\n"
        md += "|-----------|-----------|---------|-------------|\n"
        for port in input_ports:
            param_info = get_parameter_info(port, container_info)
            md += f"| `{port}` | Control | {param_info.get('default', '0.0')} | {param_info.get('description', 'Input control parameter')} |\n"
    
    # Add output ports
    output_ports = [p for p, t in container_info['ports'] if p == 'output']
    if output_ports:
        md += "\n### Output Ports\n\n"
        md += "| Port Name | Data Type | Description |\n"
        md += "|-----------|-----------|-------------|\n"
        for port in output_ports:
            md += f"| `{port}` | Spectrum | Output signal/spectrum |\n"
    
    md += f"""
## Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
"""
    
    for param in container_info['parameters']:
        default_val = container_info['defaults'].get(param, '0.0')
        param_info = get_parameter_info(param, container_info)
        md += f"| `{param}` | Float | {default_val} | {param_info.get('range', 'N/A')} | {param_info.get('description', 'Parameter control')} |\n"
    
    md += f"""
## Processing Algorithm

[Algorithm details to be filled based on source code analysis]

## Configuration (Preferences)

[Configuration options to be filled based on ContainerSettings]

## Usage Examples

### Basic Setup
```
Input → {container_info['name']} → Output
```

### With Modulation
```
         ┌─────────────────────┐
         │                   │
Input → │ {container_info['name']} ├──→ Output
         │                   │
Mod → parameter            │
         │                   │
         └─────────────────────┘
```

## Troubleshooting

[Troubleshooting section to be filled based on common issues]

## Best Practices

[Best practices to be filled based on usage patterns]

## Technical Notes

[Technical details to be filled based on implementation]

## Version History

[Version history to be filled based on code changes]

---

*This documentation applies to Kala v1.4 and later. For most current implementation, refer to source code in {container_info['header']} and {container_info['implementation']}.*
"""
    
    return md

def get_parameter_info(param_name, container_info):
    """Get parameter information from settings or defaults"""
    # This would need to be extended based on ContainerSettings analysis
    return {
        'description': f'Parameter {param_name}',
        'range': 'N/A',
        'default': container_info['defaults'].get(param_name, '0.0')
    }

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python generate_docs.py \"Container Name\" [Category]")
        sys.exit(1)
    
    container_name = sys.argv[1]
    category = sys.argv[2] if len(sys.argv) > 2 else None
    
    container_info = extract_container_info(container_name)
    markdown = generate_markdown(container_info, category)
    
    filename = f"docs/{container_name.lower().replace(' ', '_')} documentation.md"
    with open(filename, 'w') as f:
        f.write(markdown)
    
    print(f"Documentation generated: {filename}")