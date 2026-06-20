# Chip8 DirectX
This Chip8 interpreter is implemented in C++ using DirectX API as a graphical interface. 
<img width="625" height="310" alt="image" src="https://github.com/user-attachments/assets/6fe74159-deca-42df-a1f8-2b652738fe45" />
## Features
1. Support Scalable screen
2. ROM loading via Windows Explorer dialog
3. Keyboard layout maped to original 16-key layout
## Controls
This emulator use same key board layout found in here (https://austinmorlan.com/posts/chip8_emulator/)
## Building
Requires Visual Studio 2022 or later, with the "Desktop development with C++" workload.

1. Clone the repository
2. Open `Chip8 Interpreter.sln` in Visual Studio
3. Build (Ctrl+Shift+B) and run (F5)
4. Use the file dialog to select a `.ch8` or `.c8` ROM file

## Known limitations
- [Can load ROM only once - e.g. have to quit program before select new ROM]
- [list anything not yet done — e.g. "8XY6/8XYE use modern shift semantics, not original COSMAC VIP behavior"]
- [e.g. "no configurable instructions-per-second"]

## License
MIT

## Acknowledgments
Built with guidance and pair-programming support from Claude (Anthropic).
