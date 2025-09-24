# chip8

A compact **CHIP‑8 emulator** written in C++ with SDL3 rendering. The repository also includes a Tetris rom.

<img width="634" height="317" alt="Screenshot 2025-09-22 144607" src="https://github.com/user-attachments/assets/25805bb7-71f1-4c7e-9a60-bde8acc48642" />

---

## Build & run 

**Windows / Visual Studio (recommended)**

1. Install \[vcpkg] and bootstrap it.
2. From the repo root run `vcpkg install` (gets dependencies from `vcpkg.json` ).
3. Open `chip8.sln` in Visual Studio and build the solution.
4. Copy the `roms/` folder and paste it into `x64/Release/` 
   > Note: if you built the solution as an x86 program then look for `x86/`. If you can't find it make sure to check for hidden folders.
5. Run `chip8.exe`.
6. Key mappings for Tetris are displayed in the console.

**Other platforms / alternate workflows**

* The project is configured to use `vcpkg` for dependencies. If you prefer a different build system (CMake, make, etc.), ensure SDL (and any other libs) are available and build the source files in the `chip8/` folder.

---

