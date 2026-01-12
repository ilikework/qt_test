import sqlite3
import os

db_path = 'MMFace_.db'
sql_path = 'init.sql'

try:
    # 1. 确保读取时使用 utf-8 编码，防止中文注释导致乱码错误
    with open(sql_path, 'r', encoding='utf-8') as f:
        sql_script = f.read()

    # 2. 连接数据库
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # 3. 使用 executescript 执行多条语句
    cursor.executescript(sql_script)
    
    conn.commit()
    print(f"✅ 成功！数据库 '{db_path}' 已初始化。")

except FileNotFoundError:
    print(f"❌ 错误：找不到文件 '{sql_path}'")
except sqlite3.Error as e:
    print(f"❌ SQLite 发生错误: {e}")
    # 打印出问题的 SQL 片段辅助调试
    print("请检查 SQL 语法，特别是分号 ; 是否遗漏。")
finally:
    if 'conn' in locals():
        conn.close()