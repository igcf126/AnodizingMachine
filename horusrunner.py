import os
import subprocess

# Set the path to the Horus executable
horus_path = "/mnt/c/Program Files (x86)/Horus/bin/Horus.exe"

# Set the path to the output directory for the PLY file
output_dir = "/mnt/c/Users/samue/Desktop/"

# Run Horus to start scanning
cmd = "{} --output-format ply --output-file {}prueba.ply --save-on-exit --scan --save --exit".format(horus_path, output_dir)
subprocess.run(cmd, shell=True)
