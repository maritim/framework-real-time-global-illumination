#include "Main/GameEngine.h"

#include "Main/Game.h"

#include "Arguments/ArgumentsAnalyzer.h"

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif

int main(int argc, char **argv) 
{
	ArgumentsAnalyzer::Instance ()->ProcessArguments (argc, argv);

	GameEngine::Init ();

	Game::Instance ()->Start ();

	GameEngine::Clear ();
}
