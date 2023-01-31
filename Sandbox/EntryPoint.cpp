#include "Engine/Engine.h"
#include "Graphics/GraphicCore/GraphicEngine.h"

int main()
{
  Engine::CreateInstance();
  Engine::GetInstance().Init();

  return Engine::GetInstance().MainLoop();
}