#!/usr/bin/env python3
"""
Drone Debug Plotter - Real-time data visualization
Sử dụng HTTP polling để lấy dữ liệu real-time từ ESP32
"""

import requests
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import time
from datetime import datetime

class DroneDebugPlotter:
    def __init__(self, esp32_ip="192.168.4.1", max_points=200):
        self.esp32_ip = esp32_ip
        self.max_points = max_points
        self.data = {
            'timestamp': deque(maxlen=max_points),
            'attitude': {
                'roll': deque(maxlen=max_points),
                'pitch': deque(maxlen=max_points),
                'yaw': deque(maxlen=max_points)
            },
            'pid': {
                'rollPID': deque(maxlen=max_points),
                'pitchPID': deque(maxlen=max_points),
                'yawPID': deque(maxlen=max_points)
            },
            'motors': {
                'motor1': deque(maxlen=max_points),
                'motor2': deque(maxlen=max_points),
                'motor3': deque(maxlen=max_points),
                'motor4': deque(maxlen=max_points)
            },
            'system': {
                'voltage': deque(maxlen=max_points),
                'current': deque(maxlen=max_points)
            }
        }
        
        self.connected = False
        self.fig, self.axes = plt.subplots(2, 2, figsize=(15, 10))
        self.fig.suptitle('Drone Debug Data - Real-time', fontsize=16)
        
    def fetch_data(self):
        """Lấy dữ liệu từ ESP32 qua HTTP"""
        try:
            url = f"http://{self.esp32_ip}/debug"
            response = requests.get(url, timeout=1)
            if response.status_code == 200:
                data = response.json()
                self.update_data(data)
                self.connected = True
                return True
            else:
                print(f"HTTP Error: {response.status_code}")
                self.connected = False
                return False
        except requests.exceptions.RequestException as e:
            print(f"Connection error: {e}")
            self.connected = False
            return False
    
    def update_data(self, data):
        """Cập nhật dữ liệu từ ESP32"""
        timestamp = data.get('timestamp', time.time() * 1000)
        
        self.data['timestamp'].append(timestamp)
        
        # Attitude data
        attitude = data.get('attitude', {})
        self.data['attitude']['roll'].append(attitude.get('roll', 0))
        self.data['attitude']['pitch'].append(attitude.get('pitch', 0))
        self.data['attitude']['yaw'].append(attitude.get('yaw', 0))
        
        # PID data
        pid = data.get('pid', {})
        self.data['pid']['rollPID'].append(pid.get('rollPID', 0))
        self.data['pid']['pitchPID'].append(pid.get('pitchPID', 0))
        self.data['pid']['yawPID'].append(pid.get('yawPID', 0))
        
        # Motor data
        motors = data.get('motors', {})
        self.data['motors']['motor1'].append(motors.get('motor1', 0))
        self.data['motors']['motor2'].append(motors.get('motor2', 0))
        self.data['motors']['motor3'].append(motors.get('motor3', 0))
        self.data['motors']['motor4'].append(motors.get('motor4', 0))
        
        # System data
        system = data.get('system', {})
        self.data['system']['voltage'].append(system.get('voltage', 0))
        self.data['system']['current'].append(system.get('current', 0))
    
    def animate(self, frame):
        """Cập nhật biểu đồ"""
        if not self.connected or len(self.data['timestamp']) == 0:
            return
        
        # Clear all axes
        for ax in self.axes.flat:
            ax.clear()
        
        timestamps = list(self.data['timestamp'])
        
        # Attitude Chart
        ax = self.axes[0, 0]
        ax.plot(timestamps, list(self.data['attitude']['roll']), label='Roll', color='red')
        ax.plot(timestamps, list(self.data['attitude']['pitch']), label='Pitch', color='blue')
        ax.plot(timestamps, list(self.data['attitude']['yaw']), label='Yaw', color='green')
        ax.set_title('Attitude (Roll, Pitch, Yaw)')
        ax.set_ylabel('Degrees')
        ax.legend()
        ax.grid(True)
        
        # PID Chart
        ax = self.axes[0, 1]
        ax.plot(timestamps, list(self.data['pid']['rollPID']), label='Roll PID', color='red')
        ax.plot(timestamps, list(self.data['pid']['pitchPID']), label='Pitch PID', color='blue')
        ax.plot(timestamps, list(self.data['pid']['yawPID']), label='Yaw PID', color='green')
        ax.set_title('PID Output')
        ax.set_ylabel('PID Value')
        ax.legend()
        ax.grid(True)
        
        # Motor Chart
        ax = self.axes[1, 0]
        ax.plot(timestamps, list(self.data['motors']['motor1']), label='Motor 1', color='red')
        ax.plot(timestamps, list(self.data['motors']['motor2']), label='Motor 2', color='blue')
        ax.plot(timestamps, list(self.data['motors']['motor3']), label='Motor 3', color='green')
        ax.plot(timestamps, list(self.data['motors']['motor4']), label='Motor 4', color='orange')
        ax.set_title('Motor Values')
        ax.set_ylabel('Percentage (%)')
        ax.set_ylim(0, 100)
        ax.legend()
        ax.grid(True)
        
        # System Chart
        ax = self.axes[1, 1]
        ax_twin = ax.twinx()
        
        line1 = ax.plot(timestamps, list(self.data['system']['voltage']), 
                       label='Voltage', color='red')
        ax.set_ylabel('Voltage (V)', color='red')
        ax.tick_params(axis='y', labelcolor='red')
        
        line2 = ax_twin.plot(timestamps, list(self.data['system']['current']), 
                            label='Current', color='blue')
        ax_twin.set_ylabel('Current (A)', color='blue')
        ax_twin.tick_params(axis='y', labelcolor='blue')
        
        ax.set_title('System (Voltage, Current)')
        ax.grid(True)
        
        # Combine legends
        lines = line1 + line2
        labels = [l.get_label() for l in lines]
        ax.legend(lines, labels, loc='upper left')
        
        # Update x-axis labels for all charts
        for ax in self.axes.flat:
            if len(timestamps) > 10:
                # Show only some timestamps to avoid crowding
                step = len(timestamps) // 10
                ax.set_xticks(timestamps[::step])
                ax.set_xticklabels([f"{t/1000:.1f}s" for t in timestamps[::step]], rotation=45)
            else:
                ax.set_xticks(timestamps)
                ax.set_xticklabels([f"{t/1000:.1f}s" for t in timestamps], rotation=45)
    
    def start_plotting(self):
        """Bắt đầu vẽ biểu đồ"""
        print(f"Connecting to ESP32 at {self.esp32_ip}...")
        
        # Test connection
        if self.fetch_data():
            print("Connection established!")
        else:
            print("Failed to connect. Make sure ESP32 is running and accessible.")
            return
        
        print("Starting real-time plotting...")
        ani = animation.FuncAnimation(self.fig, self.animate, interval=100, blit=False)
        plt.tight_layout()
        plt.show()

def main():
    # Thay đổi IP của ESP32 nếu cần
    esp32_ip = "192.168.4.1"  # IP mặc định của ESP32 Access Point
    
    plotter = DroneDebugPlotter(esp32_ip)
    
    try:
        plotter.start_plotting()
    except KeyboardInterrupt:
        print("\nStopping plotter...")

if __name__ == "__main__":
    main() 