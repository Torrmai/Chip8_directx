#pragma once
#include <string>
#include <cstdint>

class Chip8
{
public:
    Chip8();
    ~Chip8();

    std::wstring rom_name;

    bool getRom();
    void mainCycle();

    // Debug helpers
    void DebugPrintState();
    void PokeMemory(uint16_t addr, uint8_t val);
    uint8_t GetRegister(uint8_t idx) const;
    uint16_t GetPC() const;
    uint16_t GetI() const;
    const uint32_t* GetFrame() const { return video; }
    void setTimer();
    void SetKey(uint8_t key_index, bool pressed) { key[key_index] = pressed; }

private:
    uint8_t registers[16]{};
    uint8_t memmorys[4096]{};
    uint16_t idx_register{};
    uint16_t PC{};
    uint16_t stack[16]{};
    uint8_t SP{};
    uint8_t delay{};
    uint8_t sound_timer{};
    unsigned char key[16]{};
    uint32_t video[64 * 32]{};
    uint16_t opcode{};

    bool LoadRom(const std::wstring& path);
    std::wstring OpenFileDialog();
    void DisplayResourceNAMessageBox();
};