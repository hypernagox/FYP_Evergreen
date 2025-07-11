import json
import pyodbc
import jinja2
from pathlib import Path

DB_CONNECTION_STRING = (
    "DRIVER={ODBC Driver 18 for SQL Server};"
    "SERVER=(localdb)\\MSSQLLocalDB;"
    "DATABASE=localtest;"
    "Trusted_Connection=yes;"
)

def load_json_schema(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)

def fetch_current_table_schema(cursor, table_name):
    cursor.execute("""
        SELECT COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, IS_NULLABLE
        FROM INFORMATION_SCHEMA.COLUMNS
        WHERE TABLE_NAME = ?
    """, (table_name,))
    schema = {}
    for row in cursor.fetchall():
        column_type = row.DATA_TYPE.upper()
        if column_type in ("NVARCHAR", "VARCHAR", "CHAR") and row.CHARACTER_MAXIMUM_LENGTH:
            column_type += f"({row.CHARACTER_MAXIMUM_LENGTH})"
        schema[row.COLUMN_NAME] = {
            "type": column_type,
            "nullable": row.IS_NULLABLE == "YES"
        }
    return schema

def generate_alter_statements(table_name, target_columns, current_columns):
    sql_statements = []
    for col in target_columns:
        name = col["name"]
        col_type = col["type"].upper()
        nullable = col.get("nullable", True)
        if name not in current_columns:
            stmt = f"ALTER TABLE [{table_name}] ADD [{name}] {col_type} {'NULL' if nullable else 'NOT NULL'}"
            sql_statements.append(stmt)
        else:
            current = current_columns[name]
            if col_type != current["type"] or nullable != current["nullable"]:
                stmt = f"ALTER TABLE [{table_name}] ALTER COLUMN [{name}] {col_type} {'NULL' if nullable else 'NOT NULL'}"
                sql_statements.append(stmt)
    return sql_statements

def create_table_if_not_exists(cursor, table_name, columns, primary_key=None):
    cursor.execute("""
        SELECT COUNT(*) 
        FROM INFORMATION_SCHEMA.TABLES 
        WHERE TABLE_NAME = ?
    """, (table_name,))
    if cursor.fetchone()[0] > 0:
        return
    column_defs = []
    for col in columns:
        col_type = col["type"].upper()
        nullable = "NULL" if col.get("nullable", True) else "NOT NULL"
        column_defs.append(f"[{col['name']}] {col_type} {nullable}")
    pk_clause = f", PRIMARY KEY ({', '.join([f'[{pk}]' for pk in primary_key])})" if primary_key else ""
    create_sql = f"CREATE TABLE [{table_name}] ({', '.join(column_defs)}{pk_clause})"
    print(f"📌 Creating table: {create_sql}")
    cursor.execute(create_sql)

def apply_schema_updates(tables_schema, cursor):
    for table_block in tables_schema:
        table_name = table_block["table"]
        target_columns = table_block["columns"]
        primary_key = table_block.get("primary_key", None)
        print(f"\n테이블 {table_name} 업데이트 시작")
        create_table_if_not_exists(cursor, table_name, target_columns, primary_key)
        current_schema = fetch_current_table_schema(cursor, table_name)
        sql_updates = generate_alter_statements(table_name, target_columns, current_schema)
        for stmt in sql_updates:
            print(f"Executing: {stmt}")
            cursor.execute(stmt)

def apply_procedures(procedures, cursor):
    for proc in procedures:
        name = proc["name"]
        params = proc["params"]
        body = proc["body"]
        drop_sql = f"IF OBJECT_ID('dbo.sp{name}', 'P') IS NOT NULL DROP PROCEDURE dbo.sp{name};"
        param_sql = ", ".join([f"@{p['name']} {p['type']}" for p in params])
        create_sql = f"CREATE PROCEDURE dbo.sp{name} ({param_sql}) AS BEGIN {body} END"
        print(f"\n🛠️ Updating procedure sp{name}")
        print(f"Executing: {drop_sql}")
        cursor.execute(drop_sql)
        print(f"Executing: {create_sql}")
        cursor.execute(create_sql)

def sql_to_cpp_type(sql_type: str) -> str:
    sql_type = sql_type.lower()
    if "nvarchar" in sql_type or "varchar" in sql_type:
        return "WCHAR"
    elif "bigint" in sql_type:
        return "int64"
    elif "int" in sql_type:
        return "int32"
    elif "datetime" in sql_type:
        return "TIMESTAMP_STRUCT"
    elif "bit" in sql_type:
        return "bool"
    elif "varbinary" in sql_type:
        return "varbinary"
    return sql_type

def generate_header_from_procedures(procedures, template_dir="Templates", output_file = "../DBQueryServer/Procedures.h"):
    env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(template_dir),
        trim_blocks=True,
        lstrip_blocks=True
    )
    env.filters["sql_to_cpp_type"] = sql_to_cpp_type

    template = env.get_template("Procedure.j2")
    result = template.render(procs=procedures)
    with open(output_file, "w", encoding="utf-8") as f:
        f.write(result)

def main():
    schema = load_json_schema("schema.json")
    conn = pyodbc.connect(DB_CONNECTION_STRING)
    cursor = conn.cursor()

    if "tables" in schema:
        apply_schema_updates(schema["tables"], cursor)

    if "procedures" in schema:
        apply_procedures(schema["procedures"], cursor)
        generate_header_from_procedures(schema["procedures"])

    conn.commit()
    conn.close()
    print("\n✅ 스키마 및 프로시저 처리 및 헤더 생성 완료")

if __name__ == "__main__":
    main()