import configparser
import os
import shutil
import glob
import time


Import("env")

def get_library_path(lib_name):
    global_lib_path = os.path.join(env["PROJECT_DIR"], ".pio", "libdeps", env["PIOENV"], lib_name)
    if os.path.exists(global_lib_path):
        print(f"✅ Found global library: {global_lib_path}")
        return global_lib_path

def merge_filesystems():
    print("\n🔧 Merging filesystems...")
    # Пути к папкам с данными
    project_data = env.subst("$PROJECT_DATA_DIR")
    lib_data = os.path.join(get_library_path("avr-fota"), "data")
    temp_data = os.path.join(env["PROJECT_DIR"], "merged_data")

    # Создаем временную папку
    if os.path.exists(temp_data):
        shutil.rmtree(temp_data)
    os.makedirs(temp_data)

    print(f"📂 Project data: {project_data}")
    print(f"📂 Library data: {lib_data}")
    print(f"📂 Temporary FS: {temp_data}")

    # Копируем файлы из проекта
    if os.path.exists(project_data):
        print("\n📥 Copying project data:")
        for item in os.listdir(project_data):
            src = os.path.join(project_data, item)
            dst = os.path.join(temp_data, item)
            if os.path.isfile(src):
                shutil.copy2(src, dst)
                print(f"  → {item}")
            elif os.path.isdir(src):
                shutil.copytree(src, dst)
                print(f"  → {item}/")

    # Копируем выборочные файлы из библиотеки
    if os.path.exists(lib_data):
        print("\n📥 Copying library data:")
        # Маски файлов для копирования
        patterns = [
            "config.json",
            "*.js",
            "*.html",
            "*.gif"
        ]

        for pattern in patterns:
            for src in glob.glob(os.path.join(lib_data, pattern), recursive=True):
                rel_path = os.path.relpath(src, lib_data)
                dst = os.path.join(temp_data, rel_path)

                # Создаем директории при необходимости
                os.makedirs(os.path.dirname(dst), exist_ok=True)

                shutil.copy2(src, dst)
                print(f"  → {rel_path}")

    # Переопределяем путь к данным
    env["PROJECT_DATA_DIR"] = temp_data
    print(f"\n✅ Merged {len(os.listdir(temp_data))} items into temporary FS")

def apply_submodule_config(submodule_config, env_name):
    config = configparser.ConfigParser()
    config.read(submodule_config)

    if env_name in config:
        for key, value in config.items(env_name):
            if key not in ["platform", "framework", "extra_scripts", "lib_deps"]:
                print(key, value)
                if value.isdigit():
                    value = int(value)
                env.Replace(**{key.upper(): value})
                print(f"Applied: {key.upper()} = {value}")
# Помечаем как загруженное
    env.Append(CUSTOM_SUBMODULE_LOADED="1")


apply_submodule_config("avr-fota/platformio.ini", "env:esp32cam")
merge_filesystems()