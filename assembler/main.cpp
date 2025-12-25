#include <bitset>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

constexpr unsigned short JUMP_GREATER = 0b0000000000000001;
constexpr unsigned short JUMP_EQUAL = 0b0000000000000010;
constexpr unsigned short JUMP_LESS = 0b0000000000000100;
constexpr unsigned short STORE_MEMORY = 0b0000000000001000;
constexpr unsigned short STORE_DATA = 0b0000000000010000;
constexpr unsigned short STORE_ADDRESS = 0b0000000000100000;
constexpr unsigned short NEGATE_OUTPUT = 0b0000000001000000;
constexpr unsigned short FUNCTION = 0b0000000010000000;
constexpr unsigned short NEGATE_Y = 0b0000000100000000;
constexpr unsigned short ZERO_Y = 0b0000001000000000;
constexpr unsigned short NEGATE_X = 0b0000010000000000;
constexpr unsigned short ZERO_X = 0b0000100000000000;
constexpr unsigned short SWAP_Y = 0b0001000000000000;
constexpr unsigned short C_INSTRUCTION = 0b1110000000000000;
constexpr unsigned short A_INSTRUCTION_MASK = 0b0111111111111111;
constexpr std::string C_INSTRUCTION_LEADING_CHARS = "ADM-!10";

void addPredefinedSymbols(std::unordered_map<std::string, unsigned short>& symbolTable)
{
    for (int i = 0; i < 16; i++)
    {
        symbolTable["R" + std::to_string(i)] = i;
    }

    symbolTable["SCREEN"] = 16364;
    symbolTable["KBD"] = 24576;
    symbolTable["SP"] = 0;
    symbolTable["LCL"] = 1;
    symbolTable["ARG"] = 2;
    symbolTable["THIS"] = 3;
    symbolTable["THAT"] = 4;
}

// C_INSTRUCTION is our "error" so to speak
unsigned short cComp(const char operation, const char leftOperand, const char rightOperand)
{
    unsigned short compVal = 0;

    switch (operation)
    {
    case '-':
        compVal |= FUNCTION;
        if (rightOperand != '1') compVal |= NEGATE_OUTPUT;

        if (leftOperand == '0')
        {
            if (rightOperand == 'D') compVal |= NEGATE_Y | ZERO_Y;
            else if (rightOperand == 'A' || rightOperand == 'M') compVal |= NEGATE_X | ZERO_X;
            else if (rightOperand == '1') compVal |= NEGATE_X | ZERO_X | ZERO_Y;
            else return C_INSTRUCTION;
        }

        else if (leftOperand == 'D')
        {
            if (rightOperand == 'A' || rightOperand == 'M') compVal |= NEGATE_X;
            else if (rightOperand == '1') compVal |= NEGATE_Y | ZERO_Y;
            else return C_INSTRUCTION;
        }

        else if (leftOperand == 'A' || leftOperand == 'M')
        {
            if (rightOperand == 'D') compVal |= NEGATE_Y;
            else if (rightOperand == '1') compVal |= NEGATE_X | ZERO_X;
            else return C_INSTRUCTION;
        }

        else return C_INSTRUCTION;

        break;
    case '+':
        compVal |= FUNCTION;

        if (leftOperand == '0')
        {
            if (rightOperand == '0') compVal |= ZERO_X | ZERO_Y;
            else if (rightOperand == '1') compVal |= ZERO_X | ZERO_Y | NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
            else return C_INSTRUCTION;
        }

        else if (leftOperand == 'D')
        {
            if (rightOperand == 'A' || rightOperand == 'M')
            {
            }
            else if (rightOperand == '1') compVal |= NEGATE_X | NEGATE_Y | ZERO_Y | NEGATE_OUTPUT;
            else return C_INSTRUCTION;
        }

        else if (leftOperand == 'A' || leftOperand == 'M')
        {
            if (rightOperand == 'D')
            {
            }
            else if (rightOperand == '1') compVal |= NEGATE_X | NEGATE_Y | ZERO_X | NEGATE_OUTPUT;
            else return C_INSTRUCTION;
        }

        else return C_INSTRUCTION;

        break;
    // The ! operator is just an AND 1 with negated output, so a quick check and a fallthrough here
    case '!':
        if (leftOperand != '0') return C_INSTRUCTION;
        compVal |= NEGATE_OUTPUT;
    case '&':
        // Will return failed value if left is equal to 'D' and right is not equal to 'A' or 'M'
        // "Fallthrough" is a compVal of 0, which is D&A or D&M
        if (leftOperand == 'D' && rightOperand != 'A' && rightOperand != 'M') return C_INSTRUCTION;
        if (leftOperand == '0')
        {
            if (rightOperand == 'D') compVal |= ZERO_Y | NEGATE_Y;
            else if (rightOperand == 'A' || rightOperand == 'M') compVal |= ZERO_X | NEGATE_X;
            else return C_INSTRUCTION;
        }
        break;
    case '|':
        if (leftOperand != 'D' || (rightOperand != 'A' && rightOperand != 'M')) return C_INSTRUCTION;
        compVal |= NEGATE_X | NEGATE_Y | NEGATE_OUTPUT;
        break;
    default:
        return C_INSTRUCTION;
    }

    return compVal;
}

int main(const int argc, const char* argv[])
{
    // Check for argument
    if (argc <= 1)
    {
        std::cerr << "You must pass a file as an argument." << std::endl;
        return EXIT_FAILURE;
    }

    // Check file ending (yes, I know this isn't the most secure, but oh well)
    std::string fileName = argv[1];
    if (fileName.substr(fileName.find('.')) != ".asm")
    {
        std::cerr << "\"" << fileName << "\" is not an assembly file." << std::endl;
        return EXIT_FAILURE;
    }

    // Read the file (storing in variable, so I don't keep the file open for too long)
    std::ifstream assemblyFile(fileName, std::ios::in);
    if (!assemblyFile.is_open())
    {
        std::cerr << "\"" << fileName << "\" could not be opened." << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> assemblyLines;

    for (std::string line; std::getline(assemblyFile, line);)
    {
        // Strip all spaces from the line
        std::erase_if(line, [](const unsigned char x) { return std::isspace(x); });
        assemblyLines.push_back(line);
    }

    assemblyFile.close();

    // Symbol pass
    std::unordered_map<std::string, unsigned short> symbolTable;
    addPredefinedSymbols(symbolTable);
    // TODO Make the symbol pass

    // Computation pass
    std::vector<unsigned short> binaryLines;
    for (unsigned int i = 0, lineCount = assemblyLines.size(); i < lineCount; ++i)
    {
        const std::string& line = assemblyLines[i];
        if (!(C_INSTRUCTION_LEADING_CHARS.contains(line[0]) || line[0] == '@'))
        {
            // If it's not a comment, break out of main
            if (line.substr(0, 2) != "//" && !line.empty())
            {
                std::cerr << "Invalid instruction at line " << i + 1 << "." << std::endl;
                return EXIT_FAILURE;
            }
            continue;
        }
        unsigned short instruction;

        // A-Instruction block
        // This will use bitwise AND, as we are ensuring the most significant bit is 0 with the mask
        if (line[0] == '@')
        {
            std::string aString = line.substr(1);
            unsigned short aValue;
            instruction = A_INSTRUCTION_MASK;

            if (symbolTable.contains(aString)) aValue = symbolTable[aString];
            else
            {
                try
                {
                    int val = std::stoi(line.substr(1));
                    // Abusing that this needs to be in a try-catch by throwing an exception prior to assignment
                    if (unsigned short limit = -1; val > limit) throw std::exception();
                    aValue = val;
                }
                // Will not be using that exception for anything
                catch (std::exception&)
                {
                    std::cerr << "Invalid token or value at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }
            }

            instruction &= aValue;
        }

        // C-Instruction block
        // This will use bitwise OR because it's adding to the base C_INSTRUCTION
        else
        {
            instruction = C_INSTRUCTION;
            // The index in the string to read from, gets modified in the .contains('=') check
            unsigned int instructionIndex = 0;

            // Destination
            if (line.contains('='))
            {
                instructionIndex = line.find('=') + 1;
                for (std::string assignment = line.substr(0, instructionIndex - 1); char c : assignment)
                {
                    switch (c)
                    {
                    case 'A':
                        instruction |= STORE_ADDRESS;
                        break;
                    case 'D':
                        instruction |= STORE_DATA;
                        break;
                    case 'M':
                        instruction |= STORE_MEMORY;
                        break;
                    default:
                        std::cerr << "Invalid assignment to \"" << c << "\" line " << i + 1 << "." << std::endl;
                        return EXIT_FAILURE;
                    }
                }
            }

            // Check for SWAP_Y requirement
            unsigned long semicolon = line.contains(';') ? line.find(';') : line.length();
            std::string comp = line.substr(instructionIndex, semicolon);
            if (comp.contains('M'))
            {
                if (comp.contains('A'))
                {
                    std::cerr << "Invalid computation at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }
                instruction |= SWAP_Y;
            }


            // Computation
            unsigned short compLength = comp.length();

            if (compLength > 3)
            {
                std::cerr << "Invalid computation at line " << i + 1 << "." << std::endl;
                return EXIT_FAILURE;
            }

            char op = compLength > 1 ? comp.at(compLength - 2) : comp.at(0) == '0' || comp.at(0) == '1' ? '+' : '&';
            char left = compLength > 2 ? comp.at(0) : '0';
            char right = comp.at(compLength - 1);

            unsigned short compValue = cComp(op, left, right);

            // Using the 3 most significant bits as error checking
            if ((compValue & C_INSTRUCTION) != 0)
            {
                std::cerr << "Invalid computation at line " << i + 1 << "." << std::endl;
                return EXIT_FAILURE;
            }

            instruction |= compValue;

            // Check if a semicolon exists, and read the jump
            if (semicolon != line.length())
            {
                std::string jump = line.substr(semicolon + 1);

                if (jump[0] != 'J')
                {
                    std::cerr << "Invalid jump at line " << i + 1 << "." << std::endl;
                    return EXIT_FAILURE;
                }

                if (jump == "JNE") instruction |= JUMP_LESS | JUMP_GREATER;
                else if (jump == "JMP") instruction |= JUMP_GREATER | JUMP_LESS | JUMP_EQUAL;
                else
                {
                    if (jump.contains('G')) instruction |= JUMP_GREATER;
                    if (jump.contains('L')) instruction |= JUMP_LESS;
                    if (jump.contains('E')) instruction |= JUMP_EQUAL;
                }
            }
        }

        // Add the instruction to the binary lines
        binaryLines.push_back(instruction);
    }


    // Write to .hack file
    fileName = fileName.substr(0, fileName.find('.')) + ".hack";
    std::ofstream binaryFile(fileName, std::ios::out);
    if (!binaryFile.is_open())
    {
        std::cerr << "\"" << fileName << "\" could not be written to." << std::endl;
        return EXIT_FAILURE;
    }

    for (unsigned int i = 0, end = binaryLines.size(); i < end; ++i)
    {
        unsigned short line = binaryLines[i];
        binaryFile << std::bitset<16>(line) << (i < end - 1 ? "\n" : "");
    }

    binaryFile.close();

    std::cout << "Assembled to " << fileName << "!" << std::endl;
    return EXIT_SUCCESS;
}
