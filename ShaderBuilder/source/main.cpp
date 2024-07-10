#include <iostream>
#include <sstream>
#include <filesystem>
#include "ShaderBuilder.h"

void GenerateDirectory(const char* directory)
{
    std::filesystem::path p = directory;
    std::filesystem::create_directory(p);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
        return -1;
    ShaderBuilder sb;
    ShaderBuilder::BasePathType** temp;
    temp = new ShaderBuilder::BasePathType * [argc];
    for (int i = 0; i < argc; ++i)
    {
        size_t len = strlen(argv[i]);
        temp[i] = new ShaderBuilder::BasePathType[len + 1];
        for (int j = 0; j < len + 1; ++j)
        {
            temp[i][j] = argv[i][j];
        }
    }
    if (argv[2])
    {
        GenerateDirectory(argv[2]);
    }
    if (argv[3])
    {
        GenerateDirectory(argv[3]);
    }

    GenerateDirectory("../autogen/");
    size_t compileFailed = sb.CompileAll(temp[1], temp[2], temp[3]);

    if (compileFailed)
    {
        std::cout << compileFailed << " permutations failed\n";
        return 1;
    }
    return 0;
}