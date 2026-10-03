# IEEE Tetris Table

A dual-ESP32 control system for a custom LED Tetris table, featuring a wireless NES controller, ESP-NOW communication, and a browser-based control interface.

## Overview

This project modernizes the control system of an existing LED Tetris table while maintaining compatibility with its original game hardware.

The system uses two ESP32 microcontrollers. One ESP32 interfaces with a physical NES controller and transmits button presses wirelessly using ESP-NOW. A second ESP32 receives these commands and converts them into the timed, active-low signals required by the original Tetris hardware.

After completing the original wireless controller implementation, I returned to the project and expanded the table-side firmware to include a Wi-Fi web server. The table can now be controlled either with the physical NES controller or through a browser-based virtual controller.

## System Architecture

```text
Physical NES Controller
        │
        ▼
Controller ESP32
        │
        │ ESP-NOW
        ▼
┌─────────────────────────┐
│    Table-Side ESP32     │
│                         │
│  ESP-NOW Receiver       │
│  Wi-Fi Web Server       │◄──── Web Browser
│  GPIO Pulse Control     │       (HTTP)
└────────────┬────────────┘
             │
             │ Active-Low GPIO Signals
             ▼
     Legacy Tetris Hardware
```

Both control methods ultimately generate the same output signals, allowing the new ESP32-based system to interface with the existing Tetris hardware without modifying the original game logic.

## Features

- Physical NES controller interface
- Wireless controller communication using ESP-NOW
- Browser-based virtual Tetris controller
- Wi-Fi HTTP server hosted by the table-side ESP32
- mDNS discovery using `IEEE_PLAY_TETRIS.local`
- Timed active-low GPIO signals that emulate the original controller interface
- Non-blocking GPIO pulse scheduling in the updated table-side firmware
- Single-client web control with a 15-second timeout
- Support for A, B, Start, Select, and directional controls

## Firmware

### NES Controller

`controller/NES_Controller.ino`

The controller-side ESP32 interfaces directly with a classic NES controller. It reads the controller state, identifies button presses, and transmits the corresponding commands to the table using ESP-NOW.

This allows the original wired controller interface to be replaced with a wireless connection while retaining the physical NES controller.

### Tetris Table Controller

`table_controller/Tetris_Table_Controller.ino`

The table-side ESP32 serves as the interface between the new control system and the original Tetris hardware.

The current firmware accepts commands from two sources:

1. **ESP-NOW** — receives commands from the wireless NES controller.
2. **HTTP** — receives commands from the browser-based virtual controller.

Commands from either source are translated into timed, active-low GPIO pulses that emulate the signals expected by the original controller interface.

The firmware also hosts the web controller and provides local mDNS access at:

```text
IEEE_PLAY_TETRIS.local
```

### Original ESP-NOW Receiver

`legacy/Original_ESPNow_Receiver.ino`

This file contains the original table-side receiver implementation.

The first version of the project used ESP-NOW exclusively. The receiver decoded commands from the controller ESP32 and generated the required GPIO signals using blocking delays.

After the original implementation was completed, the receiver was redesigned to incorporate the web server and a non-blocking pulse system. The original firmware is retained here to document the development of the project.

## Web Controller

The updated table-side ESP32 hosts a mobile-friendly web interface containing virtual NES controls.

HTTP requests corresponding to each controller button are handled by the ESP32:

```text
/A
/B
/up
/down
/left
/right
/select
/start
```

The web interface and physical NES controller operate through the same output system, so either input method can control the existing Tetris hardware.

To prevent multiple devices from attempting to control the table simultaneously, the web interface uses a single-client IP lock with a 15-second timeout.

## Legacy Hardware Interface

A major part of the project involved interfacing the ESP32 with hardware that was not originally designed for it.

Rather than replacing the existing Tetris electronics, the ESP32 emulates the original controller signals. Each button command is converted into a precisely timed active-low GPIO pulse.

This approach allowed new wireless and web-based controls to be added while preserving the existing Tetris game hardware.

## Technologies

- ESP32
- Embedded C/C++
- ESP-NOW
- Wi-Fi
- HTTP
- mDNS
- GPIO
- NES controller serial interface
- Embedded web interface
- Legacy hardware interfacing

## Project Development

The project was developed in two main stages.

**Original implementation:**  
A physical NES controller was connected to an ESP32, which decoded the controller inputs and transmitted commands wirelessly over ESP-NOW. A second ESP32 received those commands and generated the signals required by the Tetris table.

**Web control expansion:**  
After completing the original system, I returned to the project independently and redesigned the table-side firmware. I integrated a Wi-Fi web server, browser-based controller, mDNS discovery, and non-blocking GPIO pulse scheduling while retaining compatibility with the ESP-NOW controller.

The resulting system supports both a physical wireless NES controller and browser-based control without requiring changes to the original Tetris game hardware.

## Repository Structure

```text
IEEE_TetrisTable/
│
├── README.md
│
├── controller/
│   └── NES_Controller.ino
│
├── table_controller/
│   └── Tetris_Table_Controller.ino
│
└── legacy/
    └── Original_ESPNow_Receiver.ino
```

## Author

**Dylan Bollone**  
Computer Engineering  
Lake Superior State University
