#include <iostream>
#include <unordered_map>

constexpr unsigned short JUMP_GREATER       = 0b0000000000000001;
constexpr unsigned short JUMP_EQUAL         = 0b0000000000000010;
constexpr unsigned short JUMP_LESS          = 0b0000000000000100;
constexpr unsigned short STORE_MEMORY       = 0b0000000000001000;
constexpr unsigned short STORE_DATA         = 0b0000000000010000;
constexpr unsigned short STORE_ADDRESS      = 0b0000000000100000;
constexpr unsigned short NEGATE_OUTPUT      = 0b0000000001000000;
constexpr unsigned short FUNCTION           = 0b0000000010000000;
constexpr unsigned short NEGATE_Y           = 0b0000000100000000;
constexpr unsigned short ZERO_Y             = 0b0000001000000000;
constexpr unsigned short NEGATE_X           = 0b0000010000000000;
constexpr unsigned short ZERO_X             = 0b0000100000000000;
constexpr unsigned short SWAP_Y             = 0b0001000000000000;
constexpr unsigned short C_INSTRUCTION      = 0b1110000000000000;
constexpr unsigned short A_INSTRUCTION_MASK = 0b0111111111111111;

void addPredefinedSymbols(std::unordered_map<std::string, unsigned short> &symbol_table)
{
    for (int i = 0; i < 16; i++)
    {
        symbol_table["R" + std::to_string(i)] = i;
    }
    symbol_table["SCREEN"] = 16364;
    symbol_table["KBD"] = 24576;
    symbol_table["SP"] = 0;
    symbol_table["LCL"] = 1;
    symbol_table["ARG"] = 2;
    symbol_table["THIS"] = 3;
    symbol_table["THAT"] = 4;
}

int main(const int argc, const char *argv[])
{
    if (argc <= 1)
    {
        std::cout << "You must pass an assembly file." << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Compiling " << argv[1] << " now..." << std::endl;
    std::unordered_map<std::string, unsigned short> symbol_table;
    addPredefinedSymbols(symbol_table);

    // Symbol pass


    // Computation pass


    std::cout << "Compiled!" << std::endl;
    return EXIT_SUCCESS;
}