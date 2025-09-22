# chip8

A compact **CHIP‑8 emulator** written in C++ with an SDL3 rendering. This repository contains the emulator source and a TETRIS rom.
<img width="628" height="318" alt="Screenshot 2025-09-22 144324" src="https://github.com/user-attachments/assets/6d12aa51-c8e4-4798-bec0-3cedd6d731f5" />

---

## Content 

* CHIP‑8 CPU, memory, display and input implementation in C++.
* SDL-based window/input/graphics layer (project configured via `vcpkg` files).
* Visual Studio solution included (`chip8.sln`) for quick Windows builds.
* Tetris rom for demo

---

## Build & run 

**Windows / Visual Studio (recommended)**

1. Install \[vcpkg] and bootstrap it.
2. From the repo root run `vcpkg install` (it will read `vcpkg.json`).
3. Open `chip8.sln` in Visual Studio and build (Debug/Release).
4. Run the produced executable — load a `.ch8` ROM with the UI or CLI as implemented.
5. Copy the `roms/` folder into the build folder.

**Other platforms / alternate workflows**

* The project is configured to use `vcpkg` for dependencies. If you prefer a different build system (CMake, make, etc.), ensure SDL (and any other libs) are available and build the source files in the `chip8/` folder.

---

## Usage

* Launch the built executable and open a CHIP‑8 ROM (.ch8).
* Keyboard mappings for Tetris are displayed in console.

---


