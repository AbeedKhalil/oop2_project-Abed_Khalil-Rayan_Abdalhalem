#include "Game.h"
#include "DefaultSpriteManager.h"
#include "DefaultAudioPlayer.h"
#include <iostream>
#include <exception>

int main()
{
    try
    {
        FishGame::DefaultSpriteManager spriteManager;
        FishGame::DefaultAudioPlayer audioPlayer;
        FishGame::Game game(spriteManager, audioPlayer);
        game.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
