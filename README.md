# Embedded Software - FS FEUP 02

This repository contains the embedded software for the FS FEUP 02 prototype, developed when I was at the Formula Student team at the University of Porto. The project is primarily built for Teensy microcontrollers and focuses on vehicle control, sensor integration, and real-time communication.

## System Architecture

### Master Controller (Teensy 4.1)
* Autonomous system state and safety logic
* CAN bus communication
* Real-time sensor acquisition (e.g., brake pressure, wheel speed, State of Charge)
* Brake light activation logic

### Dashboard Controller (Teensy 4.0)
* Driver interface logic (Ready-to-Drive, APPS plausibility, vehicle state)
* Sensor integration (hydraulic brake pressure, APPS, wheel speed)
* Real-time display control via SPI
* Inverter Mode selection
* CAN communication with Powertrain (Motor Controller, Inverter)

### Temperature Monitoring Units (6x Teensy 4.1)
* Distributed accumulator thermal monitoring using NTC sensors (18 per stack)
* CAN communication with Battery Management System (AMS) and between units

### Charging Controller (Teensy 4.0)
* Accumulator charging control logic
* CAN bus communication with charger
* Display control via SPI

### ROS CAN Interface (AS CU)
* ROS 2 node for CAN integration
* Enables autonomous operation and telemetry logging

## Repository Structure

The codebase is organized into the following main directories and files:

* **teensy_dash**: Core implementation for the Dashboard controller and SPI display logic.
* **teensy_cells**: Software for the thermal monitoring units.
* **master**: Logic for the Master Controller.
* **4d_systems**: Support and assets for display systems.
* **prechargeattiny**: Code for the precharge circuit logic.
* **teensy_handcart**: Code for the handcart interface.
* **CAN_IDs.h**: Centralized CAN bus identifier definitions.
* **conf.dbc**: Database file for system-wide CAN communication.

## Infrastructure
* **External Data Server**: Central node for data processing, visualization, and storage.
* **Documentation**: Technical manuals and instructions located in the docs folder.
