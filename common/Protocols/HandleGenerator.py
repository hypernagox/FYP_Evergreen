import os
import re
import jinja2

def parse_fbs(file_path):
    with open(file_path, 'r') as fbs_file:
        file_content = fbs_file.read()

    file_content = re.sub(r'//.*', '', file_content)

    tables = []
    table_pattern = re.compile(r'table\s+(\w+)\s*\{([^}]*)\}', re.MULTILINE)
    field_pattern = re.compile(r'\s*(\w+):\s*([\w:[\].]+);', re.MULTILINE)

    scalar_types = {
        "bool": "bool",
        "byte": "int8_t",
        "ubyte": "uint8_t",
        "short": "int16_t",
        "ushort": "uint16_t",
        "int8": "int8_t",
        "uint8": "uint8_t",
        "int16": "int16_t",
        "uint16": "uint16_t",
        "int32": "int32_t",
        "uint32": "uint32_t",
        "int64": "int64_t",
        "uint64": "uint64_t",
        "float": "float",
        "double": "double"
    }

    def resolve_type(field_type):
        if field_type == "string":
            return "std::string_view"
        elif field_type in scalar_types:
            return scalar_types[field_type]
        elif field_type.startswith('[') and field_type.endswith(']'):
            elem_type = field_type[1:-1]
            if elem_type in scalar_types:
                return f"std::vector<{scalar_types[elem_type]}>"
            else:
                return f"std::vector<{elem_type.replace('.', '::')}>"
        else:
            return field_type.replace('.', '::')

    def is_custom_type(field_type):
        return "::" in field_type

    for table_match in table_pattern.finditer(file_content):
        table_name = table_match.group(1)
        table_body = table_match.group(2)
        fields = []

        for field_match in field_pattern.finditer(table_body):
            field_name = field_match.group(1)
            field_type = resolve_type(field_match.group(2))
            fields.append({
                'name': field_name,
                'type': field_type,
                'is_custom_type': is_custom_type(field_type)
            })

        tables.append({'name': table_name, 'fields': fields})

    return tables

def render_cpp(context, template_file, output_file):
    template_loader = jinja2.FileSystemLoader(searchpath="./")
    template_env = jinja2.Environment(loader=template_loader, trim_blocks=True, lstrip_blocks=True)
    template = template_env.get_template(template_file)
    rendered_content = template.render(context)

    with open(output_file, 'w') as output:
        output.write(rendered_content)

def main():
    input_dir = ""
    fbs_files = ["struct.fbs", "enum.fbs", "protocol.fbs"]
    all_tables = []

    for fbs_file in fbs_files:
        file_path = os.path.join(input_dir, fbs_file)
        tables = parse_fbs(file_path)
        all_tables.extend(tables)

    start_id = 1000
    all_packets = []
    for table in all_tables:
        all_packets.append({"name": table['name'], "id": start_id})
        start_id += 1

    c2s_tables = [table for table in all_tables if table['name'].startswith('c2s')]
    s2c_tables = [table for table in all_tables if table['name'].startswith('s2c')]

    context_c2s = {
        'all_packets': all_packets,
        'packets': c2s_tables,
        'handler_class': 'c2s_PacketHandler',
        'namespace_prefix': 'ServerCore::'
    }
    render_cpp(context_c2s, "create_packet_handler.h.j2", "c2s_PacketHandler.h")

    context_s2c = {
        'all_packets': all_packets,
        'packets': s2c_tables,
        'handler_class': 's2c_PacketHandler',
        'namespace_prefix': 'NetHelper::'
    }
    render_cpp(context_s2c, "create_packet_handler.h.j2", "s2c_PacketHandler.h")

if __name__ == "__main__":
    main()