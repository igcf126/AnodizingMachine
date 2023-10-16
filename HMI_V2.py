import tkinter as tk
from tkinter import ttk
import os
import subprocess
import serial
import os.path
import time



# Create the main window
root = tk.Tk()
root.title("Anodizing Machine Control")

surface_area = 0 
ser = 0

# Define function to handle button clicks

def start_scan():
    # Set up serial communication with the Arduino
    # ser = serial.Serial('/dev/ttyACM0', 9600) ######

    ## PUT HERE ALL CODE NEEDED TO RUN HORUS 
    
    # opc1. you run it via only commands and stuff
        # DOWNHERE WE GOT AN OPTION
    # Run Horus command to start scanning
    #cmd = "fabscan-cli --horus"
    #subprocess.run(cmd, shell=True) 
    
    # opc2. you run an external code inside windows, due to the fact this HMI is made inside WSL-Ubuntu
    # # Set the path to the Python script you want to run
    # script_path = '/mnt/c/Users/samue/Desktop/horusStuff/horusrunner.py'

    # # Run the Python script using the `python` command
    # subprocess.run(['python', script_path])
    # horusrunner.py debería de escanear automáticamente, además de crear un archivo tal cual.

    
    
    
    file_path = "/mnt/c/Users/samue/Desktop/horusStuff/prueba.ply"

    # Code to start the 3D scanning process via Horus
    status_label.config(text="Scanning...")

    while(not os.path.isfile(file_path)):
        print("File existsn't")    
    #     os.system('ls')
    
    os.system('ls')


    # Code to start the 3D scanning process via Horus
    status_label.config(text="Scanned...")
    print("ecaneao")

    time.sleep(0.5) # pause for half a second.
    
    status_label.config(text="Awanta...")
    print("awanta")
    
    # Copy the PLY file from the Windows desktop to the WSL Ubuntu system
    os.system('cp /mnt/c/Users/samue/Desktop/horusStuff/prueba.ply ~/prueba.ply')
    
    time.sleep(0.5) # pause for half a second.
    
    # Code to take measurements via MeshLab
    status_label.config(text="Measuring...")
    print("midiending")
    
    # Run the MeshLab script to compute the surface area of the PLY file
    meshlab_script_path = '/root/surface_area.mlx'
    input_file_path = '/root/prueba.ply'
    output_file_path = '/root/prueba.txt'   
    os.system('cd ~')

    while(not os.path.isfile(input_file_path)):
        status_label.config(text="Hold on...")
        print("tamo en eso")
        os.system('ls')
        time.sleep(0.5)


    subprocess.run(['meshlabserver', '-i', input_file_path, '-o', output_file_path, '-s', meshlab_script_path])
    print("c logro")

    while(not os.path.isfile(output_file_path)):
        status_label.config(text="casi...")
        print("casi")
        os.system('ls')
        time.sleep(0.5)

    # Read the surface area from the output file
    with open(output_file_path, 'r') as f:
        surface_area = float(f.readline())
        data_label.config(text="Surface area: {} mm²".format(surface_area))

    # def send_data():
    # Code to send data to Arduino via serial communication
    status_label.config(text="Sending data...")
    
    # Send the surface area to the Arduino
    ser.write(str(surface_area).encode())

    

# Create buttons with custom styling
style = ttk.Style()
style.theme_create('custom', parent='alt', settings={
    'TButton': {
        'configure': {
            'font': ('Arial', 12),
            'background': '#ECECEC',
            'foreground': '#333333',
            'bordercolor': '#C0C0C0',
            'padding': 10,
            'width': 20,
            'relief': 'raised'
        },
        'map': {
            'background': [('active', '#D0D0D0'), ('disabled', '#F0F0F0')]
        }
    },
    'TLabel': {
        'configure': {
            'font': ('Arial', 14),
            'background': '#ECECEC',
            'foreground': '#333333',
            'padding': 10,
            'anchor': 'center'
        }
    }
})
style.theme_use('custom')

scan_button = ttk.Button(root, text="Start Scan", command=start_scan)
scan_button.pack(fill='x')

# measure_button = ttk.Button(root, text="Create Measurement", command=create_measurement)
# measure_button.pack(fill='x')

# send_button = ttk.Button(root, text="Send Data", command=send_data)
# send_button.pack(fill='x')

# Create status label
status_label = ttk.Label(root, text="Aver")
status_label.pack(fill='both', expand=True)

data_label = ttk.Label(root, text="xD")
data_label.pack(fill='both', expand=True)

stage_label = ttk.Label(root, text="Parte")
stage_label.pack(fill='both', expand=True)



# Set window size and position
root.geometry("400x200+100+100")

# Start the main loop
root.mainloop()
