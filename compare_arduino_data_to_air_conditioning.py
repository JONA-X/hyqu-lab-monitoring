# -*- coding: utf-8 -*-
"""
Created on Mon Mar 22 17:26:53 2021

@author: Jonathan Knoll <jknoll@phys.ethz.ch>
"""
import numpy as np
import matplotlib.pyplot as plt
import csv
from os import walk
import datetime
import time 




def get_AC_temp_array(filename):
    data_array = []
    AC_temperatures_array = []
    date_time_array = []
    date_time_obj_array = []
    
    with open(filename,'r') as csv_file:
        reader = csv.reader(csv_file)
        for row in reader:
            if(row[0] == "HPVEWIZ01:HPFF_ERR14_B811_M0"):
                data_array.append(row)
                date_time_obj = datetime.datetime.strptime(row[1], '%Y-%m-%d %H:%M:%S.%f')
                timestamp = time.mktime(date_time_obj.timetuple() )
                date_time_array.append(timestamp)
                date_time_obj_array.append(date_time_obj)
                temp = float(row[2])
                temp = np.round(temp, 2)
                AC_temperatures_array.append(temp)

    return date_time_array, date_time_obj_array, AC_temperatures_array
        

def get_Arduino_temp_array(foldername):
    
    _, _, sensor_data_filenames = next(walk(foldername))
    number_of_files = len(sensor_data_filenames)
    print("Import " + str(number_of_files) + " files with sensor data")

    sensors_timestamps_all = []
    sensors_times_obj_all = []
    sensors_temp_all = []
    sensors_labels_all = []

    for index, sensor_filename in enumerate(sorted(sensor_data_filenames)):
        print("Scan sensor data for file: " + sensor_filename)
        raw_data = np.genfromtxt(foldername + "/" + sensor_filename, delimiter=',', dtype='str')
        sensor_timestamps_array = [int(int(timestamp)/1000) for timestamp in raw_data[1:,0]]
        sensor_times_obj_array = [datetime.datetime.fromtimestamp(date_time) for date_time in sensor_timestamps_array]
        sensor_temp_array = np.array(raw_data[1:,1], dtype="float")
        
        # Remove elements from the arrays where the temperature is zero (there must have been some error)
        zero_indices = np.argwhere(sensor_temp_array == 0)
        sensor_timestamps_array = np.delete(sensor_timestamps_array, zero_indices)
        sensor_times_obj_array = np.delete(sensor_times_obj_array, zero_indices)
        sensor_temp_array = np.delete(sensor_temp_array, zero_indices)
        
        sensors_timestamps_all.append(sensor_timestamps_array)
        sensors_times_obj_all.append(sensor_times_obj_array)
        sensors_temp_all.append(sensor_temp_array)
        sensors_labels_all.append(sensor_filename[:-4])
        
        print("DONE! with scanning sensor data for file: " + sensor_filename)
        

    return sensors_timestamps_all, sensors_times_obj_all, sensors_temp_all, sensors_labels_all


filename_AC_data = "data_ac/HPFFERR14B_20210318_111433.csv"
AC_date_time_array, AC_date_time_obj_array, AC_temperatures_array = get_AC_temp_array(filename_AC_data)



sensors_timestamps_all, sensors_times_obj_all, sensors_temp_all, sensors_labels_all = get_Arduino_temp_array("data_influx_fine")



def convert_timestamp_to_time(timestamp_array):
    return_obj = []
    for i, timestamp in enumerate(timestamp_array):
        dt_object = datetime.datetime.fromtimestamp(int(timestamp))
        return_obj.append(dt_object.strftime("%d.%m. %H:%M"))
    return return_obj



def convert_obj_to_time(date_time_obj):
    return_obj = []
    for i, elm in enumerate(date_time_obj):
        return_obj.append(elm.strftime("%d.%m. %H:%M"))
    return return_obj





# Do the plots
fig, ax = plt.subplots(figsize=(16, 6))
fig.canvas.draw()


# Plot the temperature for the individual Arduino sensors
for timestamp, time_obj, temp, label in zip(sensors_timestamps_all, sensors_times_obj_all, sensors_temp_all, sensors_labels_all):
    plt.plot(timestamp, temp, label=label)


# Plot the temperature from the air conditioning
plt.plot(AC_date_time_array, AC_temperatures_array, label="Air conditioning")


# Adjust the x labels: Use date and time instead of timestamp in seconds (not well human-readable)
labels, locations = plt.xticks()
print(locations)
print(labels)
ax.set_xticklabels(convert_timestamp_to_time(labels))


# Add vertical lines to mark specific events or day changes
array_vertical_lines = [
    "2021-03-09 08:30:00",
    "2021-03-10 08:30:00",
    "2021-03-11 08:30:00",
    "2021-03-12 08:30:00",
    "2021-03-13 08:30:00",
    "2021-03-14 08:30:00",
    "2021-03-15 08:30:00",
    "2021-03-16 08:30:00",
    "2021-03-17 08:30:00"
]
for vertical_line_time in array_vertical_lines:
    date_time_obj = datetime.datetime.strptime(vertical_line_time, '%Y-%m-%d %H:%M:%S')
    timestamp = time.mktime(date_time_obj.timetuple() )
    plt.vlines(timestamp, 20.5, 24.5, colors='#888888', linestyles='dashed')


# Labels and plot
plt.xlabel("Date and time")
plt.ylabel("Temperature (°C)")
plt.legend()
plt.savefig("comparison_fine.pdf")
plt.show()