#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <Windows.h>
#include <set>

#include "ShaderIncludes.h"

class ShaderBuilder
{
public:
  using PathType = const std::filesystem::path::value_type*;
  using BasePathType = std::filesystem::path::value_type;

private:
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
    uint32_t shaderNameHash;
    std::vector<std::wstring> defines;
  }; 
  // need its iteration order to be guarenteed so not using unordered
  using ShaderConfigInfoMapType = std::multimap<ShaderType, ShaderConfigInfo>;
  ShaderConfigInfoMapType m_allShaderInfos;
  // process Handle, thread handle
  std::unordered_map<HANDLE, HANDLE> m_processInfo;
  std::hash<std::wstring> hasher = {};

  std::unordered_set<uint32_t> m_createdPermutations;

  size_t CreateCompilationProcess(
    ShaderConfigInfoMapType::value_type& input,
    std::wstring intermediateFolder,
    std::wstring outputFolder,
    std::wstring shaderName);

  void ProcessConfigInfo(std::wstringstream& ss, std::wstring shaderName, ShaderConfigInfoMapType& map);

  HeaderType IsHeader(std::wstring input);

  size_t CompilePermutations(
    ShaderConfigInfoMapType::value_type& input, 
    const std::wstring origFile, 
    std::wstring intermediateFolder,
    std::wstring outputFolder,
    std::wstring curPermutation, 
    size_t nextPermutation,
    std::vector<std::wstring> defines);

  size_t CompilePermutations(
    ShaderConfigInfoMapType::value_type& input, 
    const std::wstring origFile, 
    std::wstring intermediateFolder,
    std::wstring outputFolder);

  std::wstring GeneratePermutation(
    ShaderConfigInfoMapType::value_type& input,
    const std::wstring origFile,
    std::wstring permutationName,
    std::wstring intermediateFolder, 
    std::vector<std::wstring> defines);

  size_t WaitForAllProcessCompletion();

  void GenerateCPPHeaders();

public:

  size_t Compile(PathType path, std::wstring intermediatePath, std::wstring outputPath);
  size_t CompileAll(PathType path, PathType intermediatePath, PathType outputPath);
};
