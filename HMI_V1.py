import tkinter as tk
from tkinter import ttk
import customtkinter as ctk
from tkinter.font import Font

# setup
window = ctk.CTk()
window.geometry('1280x800')
window.title('HMI System')

# Make HMI fullscreen
# window.overrideredirect(True)
# window.geometry("{0}x{1}+0+0".format(window.winfo_screenwidth(), window.winfo_screenheight()))

# Tabs para ver info proceso e info pieza
tabview = ctk.CTkTabview(window, height=775, width=1255, fg_color= '#242425')
tabview.pack(padx=10, pady=10)

tabview.add("Tab 1")  # add tab at the end
tabview.add("tab 2")  # add tab at the end
tabview.set("Tab 1")  # set currently visible tab

# Frame Tab 1
top_frame = ctk.CTkFrame(
    tabview.tab("Tab 1"),
    fg_color= '#242425'
    )
lower_frame = ctk.CTkFrame(
    tabview.tab("Tab 1"),
    height= 200,
    )

# Frames en tab1
frame1 = ctk.CTkFrame(top_frame, fg_color= '#242425',width= 250)
frame2 = ctk.CTkFrame(top_frame, corner_radius= 10)
frame3 = ctk.CTkFrame(top_frame, corner_radius= 10)
frame4 = ctk.CTkFrame(top_frame, corner_radius= 10)

# Botones frame1
start = ctk.CTkButton(
    frame1, 
    text="START",
    fg_color='#6ec453',
    corner_radius=150,
    height=100,
    width=150,
    font=('Roboto-Black',50),
    )

stop = ctk.CTkButton(
    frame1, 
    text="STOP",
    fg_color='#e3695e',
    corner_radius=150,
    height=100,
    width=150,
    font=('Roboto-Black',50),
    )

#Info frame2
TiempoAnod = ctk.CTkLabel(
    frame2,
    text="Tiempo Anodizado",
    font=('Roboto-Black',25),
    )
TiempoAnodVal = ctk.CTkLabel(
    frame2,
    text="24",
    font=('Roboto-Black',100),
    )
Temp = ctk.CTkLabel(
    frame2,
    text="Temperatura Agua °C",
    font=('Roboto-Black',25),
    )
TempVal = ctk.CTkLabel(
    frame2,
    text="76",
    font=('Roboto-Black',100),
    )

start.pack(expand = True, fill = 'both', pady=30, padx=10)
stop.pack(expand = True, fill = 'both', pady=30, padx=10)

frame1.pack(side = 'left', fill = 'both', padx = 2,)
frame2.pack(side = 'left', expand = True, fill = 'both', padx = 2,)
frame3.pack(side = 'left', expand = True, fill = 'both', padx = 2,)
frame4.pack(side = 'left', expand = True, fill = 'both', padx = 2,)

TiempoAnod.pack(pady=10)
TiempoAnodVal.pack(pady=10)
Temp.pack(pady=10)
TempVal.pack(pady=10)

top_frame.pack( expand = True, fill = 'both', pady = 5)
lower_frame.pack( expand = True, fill = 'both')

# run
window.mainloop()