# Remote Controller Websocket

This project implements a remote-controlled car system using an ESP32 microcontroller. The ESP32 acts as a Wi-Fi Access Point and runs a WebSocket server to receive control commands.

The system is designed to control two motors via an H-bridge (e.g., L298N). It interprets incoming WebSocket messages, typically containing joystick-like 'x' and 'y' coordinates, to manage the car's movement:
- The 'y' coordinate generally controls the forward/backward motion of the drive motor.
- The 'x' coordinate generally controls the left/right steering motor.

## ESP32 Pin Connections

The firmware is configured to use the following ESP32 GPIO pins to interface with an H-bridge motor driver:

### Motor A (Main Drive)
- **GPIO 12:** Connects to H-Bridge IN1
- **GPIO 14:** Connects to H-Bridge IN2
- **GPIO 13:** Connects to H-Bridge ENA (Enable A / PWM Speed Control)

### Motor B (Steering)
- **GPIO 27:** Connects to H-Bridge IN3
- **GPIO 26:** Connects to H-Bridge IN4
- **GPIO 25:** Connects to H-Bridge ENB (Enable B / PWM Speed Control)

## Wi-Fi and WebSocket Configuration

The ESP32 creates a Wi-Fi Access Point (AP) with the following default credentials:
- **SSID:** `ESP32-AP`
- **Password:** `12345678`

Once connected to this Wi-Fi network, the WebSocket server can be reached at:
- **IP Address:** The IP address assigned to the ESP32 (typically printed to the Serial Monitor on startup, often `192.168.4.1`).
- **Port:** `8080`

## Control Logic

The car is controlled by sending JSON messages over WebSocket to the ESP32. Each message should contain 'x' and 'y' values, typically ranging from -1.0 to 1.0.

- **`y` value (Drive Motor A):** Controls forward and backward movement.
    - A negative `y` value (e.g., -1.0) results in forward motion.
    - A positive `y` value (e.g., 1.0) results in backward motion.
    - The magnitude of `y` determines the speed (scaled to `MAX_PWM_SPEED`, which is 255).
- **`x` value (Steering Motor B):** Controls left and right steering.
    - A positive `x` value (e.g., 1.0) might turn the wheels right.
    - A negative `x` value (e.g., -1.0) might turn the wheels left.
    - The magnitude of `x` determines the steering intensity/angle (scaled to `MAX_PWM_SPEED`).

A value of 0 for both `x` and `y` will stop the motors. If the WebSocket connection is lost, the motors will also automatically stop as a safety measure.
