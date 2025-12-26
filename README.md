# Desk Light

## Overview

This project implements a smart desk light system using the ESP32 microcontroller. It provides customizable LED lighting with various modes and effects, perfect for creating an optimal workspace environment.

## Features

- **RGB LED Control**: Full spectrum color control for personalized lighting
- **Multiple Lighting Modes**: Pre-configured lighting scenes for different tasks
- **ESP32 Based**: Leverages the powerful ESP32 platform with WiFi and Bluetooth capabilities
- **Low Power Design**: Efficient power management for extended operation
- **Easy Configuration**: Simple setup and customization options

## Hardware Requirements

| Component | Specification |
|-----------|--------------|
| **Microcontroller** | ESP32 (ESP32-C2/C3/C5/C6/H2/S2/S3 or standard ESP32) |
| **LED Strip** | WS2812B or compatible RGB LED strip |
| **Power Supply** | 5V DC power adapter (current rating depends on LED count) |
| **Additional Components** | Connecting wires, resistors (as needed) |

## Software Requirements

- **ESP-IDF**: v4.4 or later
- **CMake**: v3.16 or later
- **Python**: v3.6 or later (for ESP-IDF tools)
- **Git**: For cloning the repository

### Supported ESP32 Targets

 ESP32  
 ESP32-C2  
 ESP32-C3  
 ESP32-C5  
 ESP32-C6  
 ESP32-H2  
 ESP32-P4  
 ESP32-S2  
 ESP32-S3

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/Mojinnn/Desk_Light_using_ESP32C3.git
cd Desk_Light_using_ESP32C3

```

### 2. Set up ESP-IDF Environment

Make sure you have ESP-IDF installed. If not, follow the [official ESP-IDF installation guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

```bash
# Set up the ESP-IDF environment
. $HOME/esp/esp-idf/export.sh
```

### 3. Configure the Project

```bash
idf.py set-target <your-esp32-target>  # e.g., esp32, esp32c3, esp32s3
idf.py menuconfig
```

### 4. Build the Project

```bash
idf.py build
```

### 5. Flash to Device

```bash
idf.py -p <PORT> flash monitor
```

Replace `<PORT>` with your serial port (e.g., `/dev/ttyUSB0` on Linux, `COM3` on Windows).

##  Project Structure

```
MCU_finalterm_project/
├── .devcontainer/          # Development container configuration
├── .vscode/                # VS Code settings
├── build/                  # Build output directory
├── main/                   # Main application source code
│   ├── CMakeLists.txt
│   └── main.c             # Main application entry point
├── managed_components/     # Managed component dependencies
│   └── espressif__led_strip/  # LED strip driver component
├── Final_Submission_Z2E/   # Final submission materials
├── CMakeLists.txt          # Root CMake configuration
├── sdkconfig              # ESP-IDF project configuration
├── dependencies.lock       # Component dependency lock file
└── README.md              # This file
```

## Configuration

### LED Strip Configuration

Edit the configuration in `idf.py menuconfig` or modify `sdkconfig` directly:

- **LED Strip Type**: Configure your LED strip model (WS2812B, etc.)
- **GPIO Pin**: Set the GPIO pin connected to LED data line
- **LED Count**: Number of LEDs in your strip
- **Color Order**: RGB, GRB, or other color ordering

### WiFi Configuration (Optional)

If implementing WiFi features:

```c
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-password"
```

## Development

### Setting Up Development Environment

This project includes VS Code and DevContainer configurations for easy development setup.

#### Using VS Code with DevContainer

1. Install Docker and VS Code with Remote-Containers extension
2. Open the project folder in VS Code
3. Click "Reopen in Container" when prompted
4. The development environment will be automatically configured

#### Manual Setup

Install the required tools:

```bash
# Install ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh

# Set up environment
. ./export.sh
```

### Building and Flashing

```bash
# Full clean build
idf.py fullclean
idf.py build

# Flash and monitor
idf.py -p <PORT> flash monitor

# Only monitor
idf.py -p <PORT> monitor
```

### Debugging

Enable debug logging in `menuconfig`:

```
Component config → Log output → Default log verbosity → Debug
```

## 🔍 Troubleshooting

### Common Issues

**Issue**: ESP32 not detected
- **Solution**: Check USB cable and drivers. Install CP210x or CH340 drivers if needed.

**Issue**: Flash fails with "A fatal error occurred"
- **Solution**: Hold the BOOT button while flashing, or check if another program is using the serial port.

**Issue**: LEDs not lighting up
- **Solution**: 
  - Verify GPIO pin configuration
  - Check power supply voltage and current
  - Ensure LED strip is properly connected
  - Check LED strip type in configuration

**Issue**: Build errors
- **Solution**: 
  - Run `idf.py fullclean` and rebuild
  - Ensure ESP-IDF environment is properly set up
  - Check ESP-IDF version compatibility

### Getting Help

- Check [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/)
- Visit [ESP32 forum](https://esp32.com/)
- Open an issue on this repository

5. Open a Pull Request

### Code Style

- Follow ESP-IDF coding conventions
- Use meaningful variable and function names
- Add comments for complex logic
- Test thoroughly before submitting

## Demo

You can see the demo of our product on YouTube: [Link](https://www.youtube.com/shorts/2UX9MsR5fGI)

## Authors
Tran Viet Thang
Tran Anh Toan
Trinh Minh Viet
Tran Le Long Vu

