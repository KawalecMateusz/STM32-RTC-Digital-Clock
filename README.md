# STM32-RTC-Digital-Clock

Academic embedded project developed for the **Microprocessors and Microcontrollers** course.
The project implements a digital clock system based on an **STM32 microcontroller** with RTC timekeeping, multiplexed 7-segment LED display, I²C LCD interface and a button-controlled configuration menu.
The repository contains the original version of the project submitted for evaluation. Some implementation decisions, comments and code structure reflect the requirements and constraints of the original academic assignment.

## Overview

The system works as a digital clock with two display interfaces:

* 6 individual 7-segment LED displays
  * Display current time in HH:MM:SS format.
  * Controlled using multiplexing with individual digit selection.

* 16x2 LCD display
  * Displays current weekday and date.
  * Provides a text interface during configuration.

The clock uses the internal STM32 RTC peripheral for time counting and date management.

## Features

* Real-time clock based on STM32 RTC peripheral
* HH:MM:SS time display using multiplexed 7-segment displays
* Date and weekday display on 16x2 I²C LCD
* Button-controlled configuration menu
* Finite State Machine (FSM) based user interface
* Software weekday calculation based on entered date

## Operating Modes

The application is controlled by a finite state machine with three main operating states:

### Normal Mode
The system displays:
* current time on the 7-segment display,
* weekday and date on the LCD.

### Field Selection Mode
Allows the user to select which parameter should be modified:
* hours
* minutes
* day
* month
* year
* 
The currently selected value remains active on the 7-segment display while other digit pairs are disabled.

### Value Editing Mode
Allows modification of the selected parameter.
During editing:
* the LCD displays the current editing state and temporary values,
* only the currently edited pair of 7-segment digits remains active.

## User Interface
The system uses four physical buttons:

* **Button 1**
  * Enter configuration mode.
  * Exit configuration mode.

* **Button 2 / Button 3**
  * Navigate between fields.
  * Increase or decrease selected values.

* **Button 4**
  * Enter value editing mode.
  * Confirm and save changes.

## Hardware Architecture
The prototype consists of:
* STM32 Nucleo development board based on STM32L476RG
* Six individual common-anode 7-segment LED displays
* 16x2 LCD display with I²C interface
* Four tactile buttons
* Transistor-based digit control circuit

The 7-segment display uses shared segment lines with individual digit enable control, allowing multiplexed operation of all six displays.

## Microcontroller Peripherals
Used STM32 peripherals:

| Peripheral | Purpose                           |
| ---------- | --------------------------------- |
| RTC        | Time and date management          |
| GPIO       | Display control and button inputs |
| I²C        | LCD communication                 |
| HAL        | Hardware abstraction layer        |

## Calendar Handling
The system supports date configuration in the range:
01.01.2000 - 31.12.2099
The weekday is calculated automatically in software based on the selected date.

## Software Structure
The application is organized into several logical sections:
* RTC initialization and time management
* Button input handling
* FSM-based menu control
* LCD communication over I²C
* 7-segment display control
* Display multiplexing

## Demonstration
A short video demonstrating the implemented clock system:
[Watch the project demonstration on YouTube](https://www.youtube.com/watch?v=HxlgqXiBhhA)

The video shows:
- normal clock operation,
- entering configuration mode,
- selecting editable fields,
- modifying time and date values.

## Documentation
Additional project documentation includes:
* STM32CubeMX pin configuration,
* hardware block diagram,
* electrical schematic,
* prototype photos,
* demonstration video.

## Technologies
* C
* STM32 HAL
* STM32CubeIDE
* STM32 RTC
* GPIO
* I²C
* LCD 16x2
* 7-segment LED multiplexing
* Finite State Machine (FSM)

## Project Status
Completed academic project.
The repository contains the original implementation submitted for evaluation.

## Running the Project
The project requires:
- STM32L476RG Nucleo board
- compatible 7-segment display hardware
- I²C LCD module
- configured wiring according to the provided documentation

The firmware can be built and flashed using STM32CubeIDE.

## Development Note
AI-assisted tools were used during the development process as learning and documentation support, mainly for understanding STM32 concepts, analysing documentation and reviewing technical descriptions.
The hardware design, implementation and software logic were developed by the author.

