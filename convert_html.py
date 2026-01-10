"""
Script to convert HTML file to C++ raw string literal for ESP32
"""

def html_to_cpp_string(html_file, output_file):
    with open(html_file, 'r', encoding='utf-8') as f:
        html_content = f.read()
    
    # Create the C++ raw string literal
    cpp_code = 'const char index_html[] PROGMEM = R"rawliteral(\n'
    cpp_code += html_content
    cpp_code += '\n)rawliteral";\n'
    
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(cpp_code)
    
    print(f"✅ Converted {html_file} to {output_file}")
    print(f"📊 Size: {len(html_content)} bytes")

if __name__ == '__main__':
    html_to_cpp_string(
        'dashboard/user_app.html',
        'src/html_content.h'
    )
