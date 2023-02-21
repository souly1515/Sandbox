#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <unordered_map>

class ShaderBuilder
{
public:
  using PathType = const std::filesystem::path::value_type*;
  using BasePathType = std::filesystem::path::value_type;

private:
  enum class ShaderType
  {
    VS = 0,
    PS = 1,
    CS = 2,
  };
  const wchar_t* ShaderTypeToChar[3] =
  {
    L"VS",
    L"PS",
    L"CS"
  };
  const wchar_t* ShaderTypeToCharLong[3] =
  {
    L"vertex",
    L"fragment",
    L"compute"
  };
  enum class HeaderType
  {
    VS_ENTRY_POINT,
    PS_ENTRY_POINT,
    CS_ENTRY_POINT,
    DEFINES,
    INVALID
  };

  struct ShaderConfigInfo
  {
    std::wstring shaderName;
    std::wstring entryPoint;
    std::vector<std::wstring> defines;
  }; 
  using ShaderConfigInfoMapType = std::unordered_multimap<ShaderType, ShaderConfigInfo>;

  size_t CreateCompilationProcess(
    ShaderConfigInfoMapType::value_type input,
    std::wstring intermediateFolder,
    std::wstring outputFolder,
    std::wstring shaderName);

  void ProcessConfigInfo(std::wstringstream& ss, std::wstring shaderName, ShaderConfigInfoMapType& map);
  HeaderType IsHeader(std::wstring input);

  size_t CompilePermutations(
    ShaderConfigInfoMapType::value_type input, 
    const std::wstring origFile, 
    std::wstring intermediateFolder,
    std::wstring outputFolder,
    std::wstring curPermutation, 
    size_t nextPermutation,
    std::vector<std::wstring> defines);
  size_t CompilePermutations(
    ShaderConfigInfoMapType::value_type input, 
    const std::wstring origFile, 
    std::wstring intermediateFolder,
    std::wstring outputFolder);

  std::wstring GeneratePermutation(
    std::wstring filename, 
    const std::wstring origFile,
    std::wstring permutationName,
    std::wstring intermediateFolder, 
    std::vector<std::wstring> defines);

public:

  size_t Compile(PathType path, std::wstring intermediatePath, std::wstring outputPath);
  size_t CompileAll(PathType path, PathType intermediatePath, PathType outputPath);
};
