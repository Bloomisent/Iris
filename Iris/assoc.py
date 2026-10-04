"""
register_roman_script.py

Registers the .iris file extension with a "RomanScript" file type in the
Windows registry, equivalent to the .reg file:

    [HKEY_CLASSES_ROOT\\.iris]
    @="Iris"
    [HKEY_CLASSES_ROOT\\Iris]
    @="Iris"
    [HKEY_CLASSES_ROOT\\IrisScript\\shell]
    [HKEY_CLASSES_ROOT\\IrisScript\\shell\\open]
    [HKEY_CLASSES_ROOT\\IrisScript\\shell\\open\\command]
    @="\"C:\\Path\\To\\interpreter.exe\" \"%1\""

Must be run on Windows with administrator privileges (HKEY_CLASSES_ROOT
is writable by non-admins for the current user in some setups, but
admin rights are recommended for a system-wide association).

Usage:
    python register_roman_script.py "C:\\Path\\To\\interpreter.exe"

If no interpreter path is given, it defaults to C:\\Path\\To\\interpreter.exe
(same placeholder as the original .reg file) — edit accordingly.
"""

import sys
import winreg  # only available on Windows


def set_key_default_value(root, path, value):
    """Create (or open) a registry key and set its default (@) value."""
    key = winreg.CreateKey(root, path)
    winreg.SetValueEx(key, None, 0, winreg.REG_SZ, value)
    winreg.CloseKey(key)


def register_iris_script(interpreter_path: str):
    command = f'"{interpreter_path}" "%1"'

    # HKEY_CLASSES_ROOT\.iris -> "Iris"
    set_key_default_value(winreg.HKEY_CLASSES_ROOT, r".iris", "Iris")

    # HKEY_CLASSES_ROOT\Iris -> "Iris"
    set_key_default_value(winreg.HKEY_CLASSES_ROOT, r"Iris", "Iris")

    # HKEY_CLASSES_ROOT\Iris\shell (no default value needed)
    winreg.CloseKey(winreg.CreateKey(winreg.HKEY_CLASSES_ROOT, r"Iris\shell"))

    # HKEY_CLASSES_ROOT\Iris\shell\open (no default value needed)
    winreg.CloseKey(winreg.CreateKey(winreg.HKEY_CLASSES_ROOT, r"Iris\shell\open"))

    # HKEY_CLASSES_ROOT\Iris\shell\open\command -> "<interpreter>" "%1"
    set_key_default_value(
        winreg.HKEY_CLASSES_ROOT, r"Iris\Iris.exe", command
    )

    print("Registry entries created successfully:")


if __name__ == "__main__":
    if sys.platform != "win32":
        print("This script only works on Windows (it uses the winreg module).")
        sys.exit(1)

    interpreter = sys.argv[1] if len(sys.argv) > 1 else r"C:\Path\To\interpreter.exe"

    try:
        register_iris_script(interpreter)
    except PermissionError:
        print(
            "Permission denied. Try running this script as Administrator "
            "(right-click -> Run as administrator)."
        )
        sys.exit(1)
