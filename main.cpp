////////////////////////////////////////////////////////////////////////////////
//
// FILE:        main.cpp
// DESCRIPTION: play chess against AI agent
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "board.h"

int main()
{
    // init game
    Board game;
    
    // load game
    cout << "Load game? (y/n): ";
    char ans; cin >> ans;

    if (ans == 'y') game.load_game();

    //cout << game.player_in_check(Color::Black) << endl;
    game.play(Color::Black, 2);

    return 0;
}
