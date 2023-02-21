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

size_t ShaderBuilder::CreateCompilationProcess(ShaderConfigInfoMapType::value_type input, std::wstring intermediateFolder, std::wstring outputFolder, std::wstring shaderName)
{
  // compile with generated file
  std::wstring exe = L"../External/vulkan/glslc.exe";
  std::wstring command = exe +
    L" -std=450 -fshader-stage=" + ShaderTypeToCharLong[int(input.first)] + L" " +
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
    map.insert(mapInput);
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
  ShaderConfigInfoMapType::value_type input,
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
    std::wstring shaderPermutation = curPermutation + L"_" + input.second.defines[nextPermutation];
    defines.push_back(input.second.defines[nextPermutation]);

    std::wstring generatedFilename = GeneratePermutation(input.second.shaderName, origFile, curPermutation, intermediateFolder, defines);

    ret |= CreateCompilationProcess(input, intermediateFolder, outputFolder, generatedFilename);

    ret |= CompilePermutations(input, origFile, intermediateFolder, outputFolder, shaderPermutation, nextPermutation + 1, defines);
  }
  return ret;
}

size_t ShaderBuilder::CompilePermutations(
  ShaderConfigInfoMapType::value_type input,
  const std::wstring origFile,
  std::wstring intermediateFolder,
  std::wstring outputFolder)
{
  std::vector<std::wstring> defines{ ShaderTypeToChar[int(input.first)] };

  std::wstring curPermutation = input.second.shaderName + defines[0];

  std::wstring generatedFilename = GeneratePermutation(input.second.shaderName, origFile, curPermutation, intermediateFolder, defines);

  size_t ret = CreateCompilationProcess(input, intermediateFolder, outputFolder, generatedFilename);

  // generates define permutations if they exist
  if(input.second.defines.size())
    ret +=  CompilePermutations(input, origFile, intermediateFolder, outputFolder, curPermutation, 0, defines);
  return ret;
}

std::wstring ShaderBuilder::GeneratePermutation(std::wstring filename, const std::wstring origFile, std::wstring permutationName, std::wstring intermediateFolder, std::vector<std::wstring> defines)
{
  uint32_t hash = uint32_t(std::hash<std::wstring>{}(permutationName));
  wchar_t file_cstr[128];

  // shaderType_shaderName_shaderHash
  swprintf_s<128>(file_cstr, L"%s_%s_%X", defines[0].c_str(), filename.c_str(), hash);

  std::wstringstream output;

  for (auto& define : defines)
    output << "#define " << define << "\n";

  output << origFile;

  if (!intermediateFolder.empty())
  {
    std::wfstream outputFile;
    outputFile.open(intermediateFolder + file_cstr + L".perm", std::ios_base::out | std::ios_base::trunc);
    assert(outputFile.is_open());
    outputFile << output.str();
  }

  return file_cstr;
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
  return compileFailures;
}
