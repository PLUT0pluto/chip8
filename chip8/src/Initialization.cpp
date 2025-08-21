#include <iostream>
#include <cstdint>
#include <array>
#include <algorithm>
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>
#include <fstream>
#include <cstdlib>
#include <map>
#include <memory>
#include <random>
#include <vector>
#include <string>


class Graphics {
public:

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	int windowW = 640;
	int windowH = 320;

	void initGraphics() {

		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
			return;
		}

		// Create a window
		window = SDL_CreateWindow(
			"CHIP-8 Emulator",           // Window title
			windowW,                         // Width (scaled 10x)
			windowH,                         // Height (scaled 10x)
			SDL_WINDOW_HIGH_PIXEL_DENSITY // Flags
		);

		if (window == nullptr) {
			std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
			SDL_Quit();
			return;
		}

		// Create a renderer
		renderer = SDL_CreateRenderer(window, nullptr);
		if (renderer == nullptr) {
			std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
			SDL_DestroyWindow(window);
			SDL_Quit();
			return;
		}
	}

	void render_display(SDL_Renderer* renderer, std::array< std::array<bool, 64>, 32 > display) {
		// Clear the screen (set to black)
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
		SDL_RenderClear(renderer);

		// Set draw color to white
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White

		// Loop through each pixel in the display array
		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 64; x++) {
				if (display[y][x] == 1) {
					// Draw a rectangle for each pixel (scaled 10x)
					SDL_FRect pixel = {
						static_cast<float>(x * 10), // X position
						static_cast<float>(y * 10), // Y position
						10.0f,                      // Width
						10.0f                       // Height
					};
					SDL_RenderFillRect(renderer, &pixel);
				}
			}
		}

		// Update the screen
		SDL_RenderPresent(renderer);
	}
};



class Chip8 {
public:

	bool shiftThingy = false;  //8xy6 and 8xyE different operations for later versions of chip8
	uint16_t opcode; //current opcode

	std::array<uint8_t, 4096> memory;  //4kb of memory unsigned char -- 0x00 0xFF (255)
	std::array<uint8_t, 16> V; //16 8bit  general registers -- go from V0 to VF
	uint16_t I; //stores memory addresses or stmn
	uint8_t delay_timer;
	uint8_t sound_timer;

	uint16_t progC; //program counter stores currently executing address
	uint8_t SP; //stack pointer points to topmost level of stack
	std::array<uint16_t, 16> stack; //16 16bit values

	std::array< std::array<bool, 64>, 32 > display; //monochrome display-- 2048 pixels   64accross 32 down

	// CHIP-8 font set (each character is 5 bytes)
	std::array<uint8_t, 80> font_set = { {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	} };


	std::map<SDL_Scancode, uint8_t> codeNative = {  //map for pressedkey:key on chip8  pairs
		{SDL_SCANCODE_1, 0x1},
		{SDL_SCANCODE_2, 0x2},
		{SDL_SCANCODE_3, 0x3},
		{SDL_SCANCODE_4, 0xC},
		{SDL_SCANCODE_Q, 0x4},
		{SDL_SCANCODE_W, 0x5},
		{SDL_SCANCODE_E, 0x6},
		{SDL_SCANCODE_R, 0xD},
		{SDL_SCANCODE_A, 0x7},
		{SDL_SCANCODE_S, 0x8},
		{SDL_SCANCODE_D, 0x9},
		{SDL_SCANCODE_F, 0xE},
		{SDL_SCANCODE_Z, 0xA},
		{SDL_SCANCODE_X, 0x0},
		{SDL_SCANCODE_C, 0xB},
		{SDL_SCANCODE_V, 0xF}
	};
	std::map<uint8_t, bool> nativeBool = {   //map for saving which keys are pressed, using chip8 keys
		{0x1, false},
		{0x2, false},
		{0x3, false},
		{0xC, false},
		{0x4, false},
		{0x5, false},
		{0x6, false},
		{0xD, false},
		{0x7, false},
		{0x8, false},
		{0x9, false},
		{0xE, false},
		{0xA, false},
		{0x0, false},
		{0xB, false},
		{0xF, false}
	};

	void clrScreen() {
		for (int y = 0; y < 32; y++) {
			for (int x = 0; x < 64; x++) {
				display[y][x] = 0;
			}
		}
	}

	void initialize() {
		progC = 0x200; //program counter starts at 0x200
		opcode = 0; //reset current opcode
		I = 0; //reset index register
		SP = 0; //reset stack pointer

		//set up display aka just clear it
		clrScreen();

		for (int i = 0; i < 16; i++) { //stack
			stack[i] = 0;
		}

		for (int i = 0; i < 16; i++) { //general register
			V[i] = 0;
		}

		for (int i = 0; i < 4096; i++) { //memory
			memory[i] = 0;
		}

		std::copy(font_set.begin(), font_set.end(), memory.begin()); //load fontset into memory

		//timers idk
		delay_timer = 0;
		sound_timer = 0;
	}

	bool load_rom(const char* filename) {
		// Open the ROM file in binary mode
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			std::cerr << "Failed to open ROM file: " << filename << "\n";
			return false;
		}

		// Get the file size
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		// Check if the ROM fits in memory
		if (size > 0xFFF - 0x200) {
			std::cerr << "ROM is too large to fit in memory!\n";
			file.close();
			return false;
		}

		// Read the ROM into memory starting at 0x200
		file.read(reinterpret_cast<char*>(&memory[0x200]), size);
		file.close();

		std::cout << "ROM loaded successfully!\n";
		return true;
	}

	uint16_t getOpcode(uint16_t pc) {

		uint8_t firstByte = memory[pc];
		uint8_t secondByte = memory[pc + 1];

		uint16_t opcodeFull = (firstByte << 8) | secondByte;
		return opcodeFull;
	}

	void mainLoop(uint16_t opc) {

		uint16_t fstnibble = opc & 0xF000;
		uint8_t x = (opc & 0x0F00) >> 8;
		uint8_t y = (opc & 0x00F0) >> 4;
		uint8_t kk = opc & 0x00FF;
		uint16_t nnn = opc & 0x0FFF;
		uint16_t n = opc & 0x000F;  //has to be 16 cuz comparing to memory address which are 16bits


		switch (fstnibble) {
		case 0x0000:


			switch (n) {
			case 0x0:  //CLR   --   clear screen
				clrScreen();
				break;
			case  0xE:  //RET   --   get back from subroutine aka set progC as what SP points to
				progC = stack[--SP];
				break;
			}

			break;
		case 0x1000:  //JUMP   -- set progC to nnn address
			progC = nnn;
			break;
		case 0x2000:  // CALL  -- call subroutine  aka   save current progC to stack and then set progC to nnn
			stack[SP++] = progC;
			progC = nnn;
			break;
		case 0x3000: //SE   --  if Vx == kk then progC+=2   (skip next instruction)
			if (V[x] == kk) progC += 2;
			break;
		case 0x4000:   //SNE   --  if Vx != kk then progC+=2   (skip next instruction)
			if (V[x] != kk) progC += 2;
			break;
		case 0x5000:   //SE    -- skip next instr if Vx == Vy
			if (V[x] == V[y]) progC += 2;
			break;
		case 0x6000: //LD Vx kk    -- set Vx to kk
			V[x] = kk;
			break;

		case 0x7000: //ADD Vx kk    -- add kk to Vx
			V[x] += kk;
			break;

		case 0x8000:
			switch (n) {
			case 0x0:
				V[x] = V[y];
				break;
			case 0x1:
				V[x] = (V[x] | V[y]);
				break;
			case 0x2:
				V[x] = (V[x] & V[y]);
				break;
			case 0x3:
				V[x] = (V[x] ^ V[y]);
				break;
			case 0x4: {
				int temp = V[x] + V[y];
				V[x] += V[y];
				if (temp > 255) V[0xF] = 1;
				else V[0xF] = 0;
				break;
			}
			case 0x5: {
				uint8_t saveX = V[x];
				V[x] -= V[y];
				if (saveX >= V[y]) V[0xF] = 1;
				else V[0xF] = 0;
				break;
			}
			case 0x6: {
				if (shiftThingy == false) V[x] = V[y];
				uint8_t saveX = V[x];
				V[x] = saveX >> 1;
				V[0xF] = saveX & 0x1;
				break;
			}
			case 0x7: {
				uint8_t saveX = V[x];
				V[x] = V[y] - V[x];
				if (saveX <= V[y]) V[0xF] = 1;
				else V[0xF] = 0;
				break;
			}
			case 0xE: {
				if (shiftThingy == false) V[x] = V[y];
				uint8_t saveX = V[x];
				V[x] = saveX << 1;
				V[0xF] = (saveX >> 7) & 0x1;
			}
					break;
			}
			break;

		case 0x9000:   //SE    -- skip next instr if Vx != Vy
			if (V[x] != V[y]) progC += 2;
			break;
		case 0xA000: //LD I nnn    -- set I to nnn
			I = nnn;
			break;
		case 0xB000:
			progC = nnn + V[0];
			break;
		case 0xC000: {
			std::random_device rd;  // Obtain a random seed from the OS
			std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
			std::uniform_int_distribution<short> distrib(0, 255); // Range 0 to 255 (inclusive)
			V[x] = distrib(gen) & kk;
			break;
		}
		case 0xD000: { //DRW Vx Vy n    -- draw sprite at Vx Vy with height n

			uint8_t dx = V[x];
			uint8_t dy = V[y];
			V[0xF] = 0;

			for (uint16_t row = 0x00; row < n; row++) {

				uint8_t sprRow = memory[I + row];  //aka sprite byte

				for (int t = 0; t < 8; t++) {
					if ((sprRow & (0x80 >> t)) != 0) {

						int actualX = (dx + t) % 64;
						int actualY = (dy + row) % 32;

						if (display[actualY][actualX] == 1) V[0xF] = 1;
						display[actualY][actualX] ^= 1;
					}
				}
			}

			break;
		}
		case 0xE000:
			switch (kk) {
			case 0x9E:
				if (nativeBool[V[x]] == true) progC += 2;
				break;
			case 0xA1:
				if (nativeBool[V[x]] == false) progC += 2;
				break;
				break;
			}
		case 0xF000:
			switch (kk) {
			case 0x07:
				V[x] = delay_timer;
				break;
			case 0x0A: {
				bool waitKey = true;
				SDL_Event event;
				while (waitKey) {
					while (SDL_PollEvent(&event)) {
						if (event.type == SDL_EVENT_KEY_DOWN) {
							SDL_Scancode pressedKey = event.key.scancode;
							if (codeNative.find(pressedKey) != codeNative.end()) {
								nativeBool[codeNative[pressedKey]] = true;
								waitKey = false;
								V[x] = codeNative[pressedKey];
								break;
							}
						}
						else if (event.type == SDL_EVENT_KEY_UP) {
							SDL_Scancode pressedKey = event.key.scancode;

							if (codeNative.find(pressedKey) != codeNative.end()) {
								nativeBool[codeNative[pressedKey]] = false;
							}
						}
					}
					delay_timer -= 1;
					sound_timer -= 1;
				}
				/*
				uint8_t pressedKey = 0x0;
				for (auto t = nativeBool.begin(); t != nativeBool.end(); ++t) {
					if (t->second == true) {
						waitKey = false;
						pressedKey = t->first;
						break;
					}
				}
				if (waitKey == true) progC -= 2;
				else V[x] = pressedKey;
				*/
				break;
			}
			case 0x15:
				delay_timer = V[x];
				break;
			case 0x18:
				sound_timer = V[x];
				break;

			case 0x1E:
				I += V[x];
				break;
			case 0x29:
				std::array<uint8_t, 16> fontNumbers;
				for (uint8_t t = 0; t <= 0xF; ++t) fontNumbers[t] = t * 5;
				I = fontNumbers[V[x]];
				break;
			case 0x33: {
				int idk = V[x];
				int mod = static_cast<int>(div(idk, 100).quot);
				memory[I] = mod;
				idk -= (mod * 100);

				mod = static_cast<int>(div(idk, 10).quot);
				memory[I + 1] = mod;
				idk -= (mod * 10);

				memory[I + 2] = idk;
				break;
			}
			case 0x55: {
				for (int idk = 0; idk <= x; idk += 1) {
					memory[I + idk] = V[idk];
				}
				break;
			}
			case 0x65:

				for (int idk = 0; idk <= x; idk += 1) {
					V[idk] = memory[I + idk];
				}
				break;
			}
			break;

		}
	}
};

int main() {
	Graphics mainWindow;
	Chip8 chip8;
	mainWindow.initGraphics();
	chip8.initialize();

	chip8.load_rom("roms/TETRIS");
	std::cout << "Keybinds\n"
		"W move left\n"
		"E move right\n"
		"Q rotate\n"
		"A down" << std::endl;

	bool running = true;
	SDL_Event event;

	const int cyclesPerSecond = 600; // Target cycles per second
	const int framesPerSecond = 60;  // Target frames per second
	const int cyclesPerFrame = cyclesPerSecond / framesPerSecond; // Cycles per frame

	uint32_t startTime = SDL_GetTicks();
	int cycleCount = 0;

	while (running) {
		// Handle events (e.g., close window)
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			else if (event.type == SDL_EVENT_KEY_DOWN) {
				SDL_Scancode pressedKey = event.key.scancode;

				if (chip8.codeNative.find(pressedKey) != chip8.codeNative.end()) {
					chip8.nativeBool[chip8.codeNative[pressedKey]] = true;
				}
			}
			else if (event.type == SDL_EVENT_KEY_UP) {
				SDL_Scancode pressedKey = event.key.scancode;

				if (chip8.codeNative.find(pressedKey) != chip8.codeNative.end()) {
					chip8.nativeBool[chip8.codeNative[pressedKey]] = false;
				}
			}
		}

		// Execute cycles
		for (int i = 0; i < cyclesPerFrame; ++i) {
			chip8.opcode = chip8.getOpcode(chip8.progC);
			chip8.mainLoop(chip8.opcode);

			uint16_t countCheck = chip8.opcode & 0xF000;
			if (countCheck != 0x1000 && countCheck != 0x2000 && countCheck != 0xB000) {
				chip8.progC += 2; // Move to the next opcode
			}
		}
		if (chip8.delay_timer > 0) chip8.delay_timer--;
		if (chip8.sound_timer > 0) chip8.sound_timer--;
		// Render the display
		mainWindow.render_display(mainWindow.renderer, chip8.display);

		// Frame rate control
		uint32_t frameTime = SDL_GetTicks() - startTime;
		if (frameTime < 1000 / framesPerSecond) {
			SDL_Delay((1000 / framesPerSecond) - frameTime);
		}
		startTime = SDL_GetTicks();
	}

	// Clean up
	SDL_DestroyRenderer(mainWindow.renderer);
	SDL_DestroyWindow(mainWindow.window);
	SDL_Quit();
}