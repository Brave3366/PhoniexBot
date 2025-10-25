# PhoniexBot
Phoniex is a a new generation of high-speed, autonomous mobile robots (AMRs) critical for optimizing warehouse logistics.
# PhoniexBot RTOS Simulation – Active Object & State Machine Based

## Project Overview
This project simulates a real-time control system using **Active Objects** and **State Machines** in C++. The goal is to demonstrate how tasks, events, and states can be managed in a modular, scalable, and thread-safe way without needing any physical hardware.

The system models multiple “objects” (tasks) that run concurrently and react to events, similar to how an RTOS-based embedded system operates.

---

## Features
- **Active Objects**: Each object has its own thread, event queue, and state machine.  
- **Event-Driven Architecture**: Tasks communicate by posting events to each other.  
- **State Machines**: Each Active Object transitions between states based on incoming events.  
- **Synchronization**: Safe access to shared resources using mutexes and semaphores.  
- **Simulated Real-Time Behavior**: Tasks execute periodically or in response to events.

---

## System Architecture
The system has three main Active Objects:

1. **Sensor Object (AO_Sensor)**  
   - Periodically generates simulated sensor data.  
   - Posts events to the Control Object when new data is ready.

2. **Control Object (AO_Control)**  
   - Waits for sensor events.  
   - Applies control logic based on the sensor data.  
   - Posts status events to the Logger Object.

3. **Logger Object (AO_Logger)**  
   - Receives status events from Control Object.  
   - Logs system activity and state transitions.


---

## Design Decisions
- **Active Object Pattern:**  
  Each task runs independently in its own thread and only interacts via events. This eliminates shared-state issues and makes the system scalable.  

- **State Machines for Each AO:**  
  Clear state definitions make it easy to reason about system behavior, especially for embedded control systems.  

- **Event-Driven Communication:**  
  Decouples tasks from each other, making the system modular and easier to maintain.  

- **Synchronization:**  
  Mutexes protect shared resources (e.g., logs or sensor data). Semaphores signal event availability between objects.

---

## How to Build & Run
### Requirements
- Linux or WSL on Windows  
- GCC or G++ compiler  
- POSIX Threads (`pthread`) library  

### Compile
```bash
gcc main.c ring_buffer.c service.c telemetry.c services.c -o PHONIEXBOT_SERVIIICE.exe -std=c11 -Wall

