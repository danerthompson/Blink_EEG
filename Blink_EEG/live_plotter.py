from PyQt5.QtWidgets import QApplication
import pyqtgraph as pg
import numpy as np
import serial
import time

# Open serial port
ser = serial.Serial('COM18', 115200)
time.sleep(2) # Wait for serial connection to initialize

# Create a Qt application
app = QApplication([])

# Create a plot window
win = pg.GraphicsLayoutWidget(show=True, title="Arduino Serial Data")
win.resize(1000, 600)

# Create plots for each data type
plots = []
for i in range(7):
    plot = win.addPlot(row=i, col=0, title=f"Data {i+1}")
    plots.append(plot)

# Create arrays to store data for each plot
data = []
for i in range(7):
    data.append(np.zeros(100))

def update():
    # Read one line of data from serial port
    line = ser.readline().decode().strip()
    # Split line into individual values
    values = line.split(',')
    # Convert first value to float for use as x-axis
    x = float(values[0])
    # Update data arrays
    for i in range(7):
        data[i][:-1] = data[i][1:]
        data[i][-1] = float(values[i+1])
    # Update plots with new data
    for i in range(7):
        plots[i].setData(x, int(data[i][-1]))

# Set up a timer to call update function every 1 ms
timer = pg.QtCore.QTimer()
timer.timeout.connect(update)
timer.start(1)

# Start Qt event loop
app.exec_()