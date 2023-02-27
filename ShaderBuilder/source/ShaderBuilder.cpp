#include "ShaderBuilder.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cassert>
#include <memory>
#include <windows.h>
#include <cwctype>

std::string wstringToString(std::wstring str)
{
  std::string ret;
  for (auto& c : str)
  {
    ret.push_back(char(c));
  }
  return ret;
}

size_t ShaderBuilder::CreateCompilationProcess(ShaderConfigInfoMapType::value_type& input, std::wstring intermediateFolder, std::wstring outputFolder, std::wstring shaderName)
{
  // compile with generated file
  std::wstring exe = L"../External/vulkan/glslc.exe";
  std::wstring command = exe +
    L" -std=450 -fshader-stage=" + ShaderTypeToCharLong[int(input.first)] + L" " +
    L"-fentry-point=" + input.second.entryPoint + L" " +
    intermediateFolder + shaderName + L".perm -o " +
    outputFolder + shaderName + L".spv";
  LPWSTR temp = new wchar_t[command.length()];
  lstrcpyW(temp, command.c_str());

  STARTUPINFO si = {};
  PROCESS_INFORMATION pi = {};

  size_t ret = 0;
  // Start the child process. 
  if (!CreateProcess(exe.c_str(),
    temp,        // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    0,          // Set handle inheritance to FALSE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory 
    &si,            // Pointer to STARTUPINFO structure
    &pi)           // Pointer to PROCESS_INFORMATION structure
    )
  {
    printf("CreateProcess failed (%d).\n", GetLastError());
    throw(std::runtime_error("CreateProcess failed.\n"));
  }
  m_processInfo.insert({ pi.hProcess, pi.hThread});
  return ret;
}

void ShaderBuilder::ProcessConfigInfo(std::wstringstream& ss, std::wstring shaderName, ShaderBuilder::ShaderConfigInfoMapType& map)
{
  if (ss.eof())
    return;
  // shader name, entry point
  std::unordered_map<ShaderType, std::wstring> uniqueShaders;
  std::vector<std::wstring> defines;
  std::wstring input;
  auto GetInputLambda = [this](std::wstringstream& ss, std::wstring& input)->HeaderType
  {
    std::wstring temp = input;
    ss >> input;
    if (input.back() == ',')
      input.pop_back();
    if (temp == input)
    {
      throw std::runtime_error("unexpected EOF reached");
    }

    return IsHeader(input);
  };

  auto type = GetInputLambda(ss, input);
  do
  {
    if (type != HeaderType::INVALID)
    {
      if(type == HeaderType::DEFINES)
      {
        type = GetInputLambda(ss, input);
        while (type == HeaderType::INVALID)
        {
          defines.push_back(input);
          if (ss.eof())
            break;
          type = GetInputLambda(ss, input);
        }
        continue;
      }
      else
      {
        std::pair<ShaderType, std::wstring> uniqueShader;
        switch (type)
        {
        case ShaderBuilder::HeaderType::CS_ENTRY_POINT:
          uniqueShader.first = ShaderType::CS;
          break;
        case ShaderBuilder::HeaderType::VS_ENTRY_POINT:
          uniqueShader.first = ShaderType::VS;
          break;
        case ShaderBuilder::HeaderType::PS_ENTRY_POINT:
          uniqueShader.first = ShaderType::PS;
          break;
        }

        type = GetInputLambda(ss, input);
        if (type != HeaderType::INVALID)
        {
          throw std::runtime_error("Using a reserved keyword as shader name");
        }

        uniqueShader.second = input;
        if (uniqueShaders.find(uniqueShader.first) != uniqueShaders.end())
        {
          throw std::runtime_error("Multiple shader entry points defined");
        }

        uniqueShaders.insert(uniqueShader);
      }
      type = GetInputLambda(ss, input);
    }
  } while (!ss.eof());

  for (auto itr : uniqueShaders)
  {
    std::pair< ShaderType, ShaderConfigInfo> mapInput;
    mapInput.first = itr.first;
    mapInput.second.entryPoint = itr.second;
    mapInput.second.defines = defines;
    mapInput.second.shaderName = shaderName;
    mapInput.second.shaderNameHash = hasher(std::wstring(ShaderTypeToChar[int(itr.first)]) + L"_" + shaderName) & ShaderKeyMask;
    map.insert(mapInput);
    m_allShaderInfos.insert(mapInput);
  }
}

ShaderBuilder::HeaderType ShaderBuilder::IsHeader(std::wstring input)
{
  switch (input[0])
  {
  case 'C':
    if (input.compare(L"CS_Entry:") == 0)
    {
      return HeaderType::CS_ENTRY_POINT;
    }
    break;
  case 'P':
    if (input.compare(L"PS_Entry:") == 0)
    {
      return HeaderType::PS_ENTRY_POINT;
    }
    break;
  case 'V':
    if (input.compare(L"VS_Entry:") == 0)
    {
      return HeaderType::VS_ENTRY_POINT;
    }
    break;
  case 'D':
    if (input.compare(L"Defines:") == 0)
    {
      return HeaderType::DEFINES;
    }
    break;
  }
  return HeaderType::INVALID;
}

size_t ShaderBuilder::CompilePermutations(
  ShaderConfigInfoMapType::value_type& input,
  const std::wstring origFile,
  std::wstring intermediateFolder,
  std::wstring outputFolder,
  std::wstring curPermutation,
  size_t nextPermutation,
  std::vector<std::wstring> defines)
{
  size_t ret = 0;
  for (; nextPermutation < input.second.defines.size(); ++nextPermutation)
  {
    defines.push_back(input.second.defines[nextPermutation]);

    std::wstring generatedFilename = GeneratePermutation(input, origFile, curPermutation, intermediateFolder, defines);

    ret |= CreateCompilationProcess(input, intermediateFolder, outputFolder, generatedFilename);

    ret |= CompilePermutations(input, origFile, intermediateFolder, outputFolder, curPermutation, nextPermutation + 1, defines);
    defines.pop_back();
  }
  return ret;
}

size_t ShaderBuilder::CompilePermutations(
  ShaderConfigInfoMapType::value_type& input,
  const std::wstring origFile,
  std::wstring intermediateFolder,
  std::wstring outputFolder)
{
  std::vector<std::wstring> defines{ ShaderTypeToChar[int(input.first)] };

  std::wstring curPermutation = std::wstring(ShaderTypeToChar[int(input.first)]) + L"_" + input.second.shaderName;

  std::wstring generatedFilename = GeneratePermutation(input, origFile, curPermutation, intermediateFolder, defines);

  size_t ret = CreateCompilationProcess(input, intermediateFolder, outputFolder, generatedFilename);

  // generates define permutations if they exist
  if(input.second.defines.size())
    ret +=  CompilePermutations(input, origFile, intermediateFolder, outputFolder, curPermutation, 0, defines);
  return ret;
}

std::wstring ShaderBuilder::GeneratePermutation(ShaderConfigInfoMapType::value_type& input, const std::wstring origFile, std::wstring permutationName, std::wstring intermediateFolder, std::vector<std::wstring> defines)
{
  uint32_t hash = uint32_t(hasher(std::wstring(permutationName))) & ShaderKeyMask;
  wchar_t file_cstr[128];
  
  {
    uint32_t curBit = 1 << ShaderDefineBitshift;
    auto itr = input.second.defines.begin();
    auto itr2 = defines.begin();
    ++itr2;
    if (itr2 != defines.end())
    {
      for (; itr != input.second.defines.end(); ++itr)
      {
        auto& define = *itr2;
        if (define == *itr)
        {
          hash |= curBit;
          ++itr2;
        }
        curBit <<= 1;
        if (itr2 == defines.end())
          break;
      }
    }
  }
#ifdef PRINT_INFO
  std::cout << "generating permutation: " << hash << " with permutations: ";
  for (auto& itr : defines)
  {
    std::wcout << itr;
  }
  std::cout << std::endl;
#endif
  if (m_createdPermutations.find(hash) != m_createdPermutations.end())
  {
    throw(std::runtime_error("permutation already compiled"));
  }
  m_createdPermutations.insert(hash);

  // shaderType_shaderName_shaderHash
  swprintf_s<128>(file_cstr, L"%s_%s_%08X", defines[0].c_str(), input.second.shaderName.c_str(), hash);

  std::wstringstream output;

  for (auto& define : defines)
  {
    output << "#define " << define << "\n";
  }

  output << origFile;

  if (!intermediateFolder.empty())
  {
    std::wfstream outputFile;
    outputFile.open(intermediateFolder + file_cstr + L".perm", std::ios_base::out | std::ios_base::trunc);
    assert(outputFile.is_open());
    outputFile << output.str();
    outputFile.close();
  }

  return file_cstr;
}

size_t ShaderBuilder::WaitForAllProcessCompletion()
{
  size_t ret = 0;
  for (auto itr : m_processInfo)
  {
    DWORD processRet = 0;
    LPDWORD p_processRet = &processRet;
    while (true)
    {
      GetExitCodeProcess(itr.first, p_processRet);
      if (processRet != STILL_ACTIVE)
      {
        if (processRet)
          ++ret;
        CloseHandle(itr.first);
        CloseHandle(itr.second);
        break;
      }
    }
  }
  m_processInfo.clear();
  return ret;
}

void ShaderBuilder::GenerateCPPHeaders()
{
  std::wfstream fs;
  fs.open(L"../autogen/ShaderData.h", std::ios_base::out | std::ios_base::trunc);

  fs << "#pragma once\n\n";
  fs << "#include \"../Include/Shared/ShaderIncludes.h\"\n";
  fs << "#include <cstdint>\n";
  fs << "#include <memory>\n";
  fs << "#include <string>\n\n";

  for (auto& shaderInfo : m_allShaderInfos)
  {
    fs << "struct " << ShaderTypeToChar[uint32_t(shaderInfo.first)] << "_" << shaderInfo.second.shaderName << ": ShaderInfo {\n";
    int i = 0;
    fs << "  operator uint32_t() override {\n";
    fs << "    return " << std::to_wstring(shaderInfo.second.shaderNameHash) << ";\n";
    fs << "  }\n";
    fs << "  operator std::string() override {\n";
    fs << "    return \"" << (std::wstring(ShaderTypeToChar[uint32_t(shaderInfo.first)]) + L"_" + shaderInfo.second.shaderName) << "\";\n";
    fs << "  }\n\n";
    fs << "  uint32_t GetNumDefines() override {\n";
    fs << "    return " << std::to_wstring(shaderInfo.second.defines.size()) << ";\n";
    fs << "  }\n\n";
    fs << "  uint32_t GetShaderStage() override {\n";
    fs << "    return " << std::to_wstring(uint32_t(shaderInfo.first)) << ";\n";
    fs << "  }\n\n";

    fs << "  static constexpr uint32_t Hash = " << std::to_wstring(shaderInfo.second.shaderNameHash) << ";\n\n";
    fs << "  static constexpr uint32_t NumFlags = " << std::to_wstring(shaderInfo.second.defines.size()) << ";\n\n";

    fs << "  struct Key {\n";
    for (auto define : shaderInfo.second.defines)
    {
      define[0] = std::towupper(define[0]);
      fs << "    static const uint32_t " << define << " = 1 << " << std::to_wstring(i) << ";\n";
      ++i;
    }
    fs << "  };\n";
    fs << "};\n\n";
  }

  fs << "const std::shared_ptr<ShaderInfo> g_shaderInfos [] = {\n";
  for (auto& shaderInfo : m_allShaderInfos)
  {
    fs << "  std::make_shared<" << ShaderTypeToChar[uint32_t(shaderInfo.first)] << "_" << shaderInfo.second.shaderName << ">(),\n";
  }
  fs << "};\n";
}

size_t ShaderBuilder::Compile(PathType path, std::wstring intermediatePath, std::wstring outputPath)
{
  std::wfstream fs;
  fs.open(path);
  if (!fs.is_open())
  {
    throw(std::runtime_error("file not found"));
  }

  ShaderConfigInfoMapType shaderInfo;
  std::wstringstream fullFile;;
  while (!fs.eof())
  {
    std::wstring temp;
    std::getline(fs, temp);
    fullFile << temp << "\n";

    std::vector<std::wstring> defines;
     

    std::wstringstream ss{ temp };
    std::wstring temp2;
    std::getline(ss, temp2, L':');

    if (temp[0] != '/' || temp[1] != '/')
      continue;
    if (temp2.find(L"Shader") == temp2.npos)
      continue;

    ss >> temp2;
    if (temp2.back() == ',')
      temp2.pop_back();
    std::wstring shaderName = temp2;
    if (!std::iswupper(shaderName[0]))
    {
      wchar_t errorMsg[256];
      swprintf_s(errorMsg, 256, L"%s: Shader %s does not start with upper case", shaderName.c_str(), path);
      throw std::runtime_error(wstringToString(errorMsg));
    }
    ProcessConfigInfo(ss, shaderName, shaderInfo);
  }

  size_t compileFailures = 0;
  for (auto& shader : shaderInfo)
  {
    compileFailures += CompilePermutations(shader, fullFile.str(), intermediatePath, outputPath);
  }

  return compileFailures;
}

size_t ShaderBuilder::CompileAll(PathType path, PathType intermediatePath, PathType outputPath)
{
  size_t compileFailures = 0;
  for (auto& entry : std::filesystem::directory_iterator(path))
  {
    try
    {
      std::cout << "Compiling " << entry.path() << std::endl;

      std::wstring intermediatePathStr{ intermediatePath };
      std::wstring outputPathStr{ outputPath };

      compileFailures += Compile(entry.path().c_str(), intermediatePathStr, outputPathStr);
    }
    catch (std::runtime_error e)
    {
      std::cout << "Error: " << e.what() << std::endl;
    }
  }
  GenerateCPPHeaders();
  compileFailures = WaitForAllProcessCompletion();
  return compileFailures;
}
