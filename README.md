# Monitoring Water Level System

This project is a **Water Level Monitoring System** designed to measure and monitor the water level in a reservoir, tank, or any container using an ultrasonic sensor. The system uses **ESP32** to read the water level and send the data to a backend server for monitoring. If the water level exceeds or falls below set thresholds, the system will activate a buzzer and control a pump.

## Features

- **Ultrasonic Sensor**: Used for measuring the water level (distance).
- **ESP32**: Handles Wi-Fi connection, data collection, and communication with the server.
- **Buzzer**: Alerts the user when the water level is dangerously high or low.
- **Pump Control**: Automatically starts or stops the pump based on the water level.
- **Backend Server**: Data is sent to a backend server (Node.js with MongoDB) for storage and monitoring.
  
## Hardware Components

- **ESP32**: Microcontroller to handle Wi-Fi communication and sensor data.
- **Ultrasonic Sensor (HC-SR04)**: Measures the distance (water level) from the sensor.
- **Buzzer**: Provides audible alerts based on water level thresholds.
- **Relay Module**: Controls the water pump.
- **Water Pump**: Starts or stops based on the water level.
  
## Software Requirements

- **Arduino IDE**: Used for coding the ESP32.
- **Arduino Libraries**:
  - `WiFi.h`: For connecting the ESP32 to the Wi-Fi network.
  - `HTTPClient.h`: For sending HTTP requests to the server.
  - `ArduinoJson.h`: For formatting and sending data in JSON format.

- **Backend Server**:
  - Node.js with **Express.js** for API endpoints.
  - **MongoDB** to store the water level data.
