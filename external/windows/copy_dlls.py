import os
import shutil

# C:\msys64\mingw64\bin
mingw64_bin = os.path.join("C:\\", "msys64", "mingw64", "bin")
print(mingw64_bin)

total_size = 0  # Variable to track the total size of copied DLLs

with open("dlls.txt", "r") as dlls_txt:
    for line in dlls_txt:
        fields = line.split()
        # get the field with mingw64/bin in it
        dll_name = ""
        for field in fields:
            if "/mingw64/bin" in field:
                dll_name = field.split("/")[-1]
                break
        dll_name = dll_name.replace("/mingw64/bin/", "")
        file_name = os.path.join(mingw64_bin, dll_name)
        file_name = os.path.normpath(os.path.join(mingw64_bin, dll_name))
        try:
            shutil.copy(file_name, ".")
            file_size = os.path.getsize(file_name)  # Get the size of the copied file
            total_size += file_size
            print(f"Copied {file_name} to current directory")
        except Exception as e:
            print(f"Error copying {file_name}: {e}")

# Convert total size to MB and print it
total_size_mb = total_size / (1024 * 1024)
print(f"Total size of copied DLLs: {total_size_mb:.2f} MB")
