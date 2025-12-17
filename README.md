# Ghost Hunt Simulation

A multithreaded ghost hunting simulation written in C, inspired by Phasmophobia. Hunters explore a haunted house collecting evidence while a ghost wanders, leaving clues and scaring investigators.

## 🎮 Overview

This project simulates a team of paranormal investigators exploring haunted House, searching for evidence to identify a ghost. Each hunter carries a detection device and must collect three unique pieces of evidence before fear or boredom drives them away. Meanwhile, the ghost roams freely, leaving evidence and haunting the hunters.

## ✨ Features

- **Multithreaded Simulation**: Hunters and ghost run concurrently using POSIX threads
- **Thread Safe Operations**: Semaphores prevent race conditions and deadlocks
- **Dynamic Memory Management**: Growing hunter array and linked-list pathfinding
- **Bitwise Evidence System**: Efficient evidence storage using bit flags
- **Deadlock Prevention**: Consistent lock ordering prevents thread deadlocks
- **Real time Logging**: CSV logs track every action for validation

## 🚀 Getting Started

### Prerequisites

- GCC compiler
- POSIX threads library (pthread)
- Linux/Unix environment

### Compilation

```bash
make
```

### Running

```bash
./ghost_hunt
```

### Cleaning

```bash
make clean
```

## 🎯 How It Works

### Hunter Behavior

1. **Explore**: Move randomly through connected rooms
2. **Detect**: Find matching evidence using their device
3. **Return**: Follow breadcrumb trail back to van
4. **Swap**: Change detection device at van
5. **Exit**: Leave when evidence collected, too scared, or bored

### Ghost Behavior

1. **Idle**: Stay in current room
2. **Haunt**: Drop evidence matching ghost type
3. **Move**: Wander to connected rooms (only when alone)
4. **Exit**: Leave when bored (no hunter interactions)

### Win Conditions

- **Hunters Win**: Collect 3 unique evidence types matching a ghost type
- **Hunters Lose**: All hunters exit due to fear or boredom
- **Ghost Wins**: Hunters fail to identify ghost before leaving
