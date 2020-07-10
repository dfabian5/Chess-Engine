////////////////////////////////////////////////////////////////////////////////
//
// FILE:        main.cpp
// DESCRIPTION: play chess against AI agent
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "board.h"

int main()
{
    Board game;
    game.play(Color::Black, 2);

    return 0;
}
