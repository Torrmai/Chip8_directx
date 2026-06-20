#include "Chip8.h"
#include <Windows.h>
#include <commdlg.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#pragma comment(lib, "comdlg32.lib")

const uint8_t fontset[80] =
{
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
};
const unsigned int font_start_pos = 0x50;

Chip8::Chip8()
{
	PC = 0x200;
	opcode = 0x0;
	idx_register = 0x0;
	SP = 0x0;
	for (int i = 0; i < (int)std::size(fontset); i++)
		memmorys[font_start_pos + i] = fontset[i];
}

Chip8::~Chip8()
{
	PC = 0x0;
}

bool Chip8::getRom()
{
	rom_name = OpenFileDialog();
	if (rom_name.empty())
	{
		DisplayResourceNAMessageBox();
		return false;
	}
	std::wcout << L"Load rom name: " << rom_name << std::endl;
	if (!LoadRom(rom_name))
	{
		DisplayResourceNAMessageBox();
		return false;
	}
	return true;
}

void Chip8::DebugPrintState()
{
	std::wcout << L"PC=0x" << std::hex << PC
		<< L" I=0x" << idx_register
		<< L" opcode=0x" << opcode
		<< L" V0=0x" << (int)registers[0]
		<< L" VA=0x" << (int)registers[0xA]
		<< L" VF=0x" << (int)registers[0xF]
		<< std::dec << std::endl;
}

void Chip8::PokeMemory(uint16_t addr, uint8_t val) { memmorys[addr] = val; }
uint8_t Chip8::GetRegister(uint8_t idx) const { return registers[idx]; }
uint16_t Chip8::GetPC() const { return PC; }
uint16_t Chip8::GetI() const { return idx_register; }

void Chip8::mainCycle()
{
	opcode = memmorys[PC] << 8 | memmorys[PC + 1];

	uint8_t  X = (opcode & 0x0F00) >> 8;
	uint8_t  Y = (opcode & 0x00F0) >> 4;
	uint8_t  N = (opcode & 0x000F);
	uint8_t  NN = (opcode & 0x00FF);
	uint16_t NNN = (opcode & 0x0FFF);

	PC += 2;

	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode)
		{
		case 0x00E0:
			memset(video, 0, sizeof(video));
			break;
		case 0x00EE:
			SP--;
			PC = stack[SP];
			break;
		}
		break;

	case 0x1000:
		PC = NNN;
		break;

	case 0x2000:
		stack[SP] = PC;
		SP++;
		PC = NNN;
		break;

	case 0x3000:
		if (registers[X] == NN) PC += 2;
		break;

	case 0x4000:
		if (registers[X] != NN) PC += 2;
		break;

	case 0x5000:
		if (registers[X] == registers[Y]) PC += 2;
		break;

	case 0x6000:
		registers[X] = NN;
		break;

	case 0x7000:
		registers[X] += NN;
		break;

	case 0x8000:
		switch (N)
		{
		case 0x0:
			registers[X] = registers[Y];
			break;
		case 0x1:
			registers[X] |= registers[Y];
			break;
		case 0x2:
			registers[X] &= registers[Y];
			break;
		case 0x3:
			registers[X] ^= registers[Y];
			break;
		case 0x4:
		{
			uint16_t sum = registers[X] + registers[Y];
			registers[0xF] = (sum > 0xFF) ? 1 : 0;
			registers[X] = sum & 0xFF;
			break;
		}
		case 0x5:
			registers[0xF] = (registers[X] > registers[Y]) ? 1 : 0;
			registers[X] = registers[X] - registers[Y];
			break;
		case 0x6:
		{
			uint8_t lsb = registers[X] & 0x1;
			registers[X] >>= 1;
			registers[0xF] = lsb;
			break;
		}
		case 0x7:
			registers[0xF] = (registers[Y] > registers[X]) ? 1 : 0;
			registers[X] = registers[Y] - registers[X];
			break;
		case 0xE:
		{
			uint8_t msb = (registers[X] & 0x80) >> 7;
			registers[X] <<= 1;
			registers[0xF] = msb;
			break;
		}
		}
		break;

	case 0x9000:
		if (registers[X] != registers[Y]) PC += 2;
		break;

	case 0xA000:
		idx_register = NNN;
		break;

	case 0xB000:
		PC = registers[0] + NNN;
		break;

	case 0xC000:
		registers[X] = (rand() % 256) & NN;
		break;

	case 0xD000:
	{
		uint8_t xPos = registers[X] % 64;
		uint8_t yPos = registers[Y] % 32;
		registers[0xF] = 0;

		for (uint8_t row = 0; row < N; row++)
		{
			uint8_t spriteByte = memmorys[idx_register + row];

			for (uint8_t col = 0; col < 8; col++)
			{
				uint8_t spritePixel = spriteByte & (0x80 >> col);
				uint32_t* screenPixel = &video[((yPos + row) % 32) * 64 + ((xPos + col) % 64)];

				if (spritePixel)
				{
					if (*screenPixel == 0xFFFFFFFF)
						registers[0xF] = 1;

					*screenPixel ^= 0xFFFFFFFF;
				}
			}
		}
		break;
	}

	case 0xE000:
		switch (NN)
		{
		case 0x9E:
			if (key[registers[X]]) PC += 2;
			break;
		case 0xA1:
			if (!key[registers[X]]) PC += 2;
			break;
		}
		break;

	case 0xF000:
		switch (NN)
		{
		case 0x07:
			registers[X] = delay;
			break;
		case 0x0A:
		{
			bool keyPressed = false;
			for (uint8_t i = 0; i < 16; i++)
			{
				if (key[i])
				{
					registers[X] = i;
					keyPressed = true;
					break;
				}
			}
			if (!keyPressed) PC -= 2;
			break;
		}
		case 0x15:
			delay = registers[X];
			break;
		case 0x18:
			sound_timer = registers[X];
			break;
		case 0x1E:
			idx_register += registers[X];
			break;
		case 0x29:
			idx_register = font_start_pos + (registers[X] * 5);
			break;
		case 0x33:
			memmorys[idx_register] = registers[X] / 100;
			memmorys[idx_register + 1] = (registers[X] / 10) % 10;
			memmorys[idx_register + 2] = registers[X] % 10;
			break;
		case 0x55:
			for (uint8_t i = 0; i <= X; i++)
				memmorys[idx_register + i] = registers[i];
			break;
		case 0x65:
			for (uint8_t i = 0; i <= X; i++)
				registers[i] = memmorys[idx_register + i];
			break;
		}
		break;
	}
}
void Chip8::setTimer()
{
	if (delay > 0)
	{
		--delay;
	}
	if (sound_timer > 0)
	{
		if(sound_timer == 1)
			std::thread([]() { Beep(300, 100); }).detach();
		--sound_timer;
	}
}

bool Chip8::LoadRom(const std::wstring& path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) return false;

	std::streampos size = file.tellg();
	if (size <= 0 || size > (4096 - 0x200))
	{
		file.close();
		return false;
	}

	char* buffer = new char[size];
	file.seekg(0, std::ios::beg);
	file.read(buffer, size);
	file.close();

	for (long i = 0; i < (long)size; i++)
		memmorys[0x200 + i] = buffer[i];

	delete[] buffer;
	return true;
}

std::wstring Chip8::OpenFileDialog()
{
	wchar_t file_name[MAX_PATH] = L"";
	OPENFILENAMEW ofn{};
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = L"CHIP-8 ROMs (*.ch8)\0*.ch8\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = file_name;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetOpenFileNameW(&ofn))
		return file_name;

	return L"";
}

void Chip8::DisplayResourceNAMessageBox()
{
	MessageBox(
		NULL,
		(LPCWSTR)L"The rom isn't provided\nPlease Provide the rom!!!\n",
		(LPCWSTR)L"Error",
		MB_ICONERROR | MB_OK
	);
}