////////////////////////////////////////////////////////////////////////////////
//
// FILE:        main.cpp
// DESCRIPTION: play chess against AI agent
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "agent.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
	auto y = [&](const int &x) {
		if (x < 0)
			return -1 * pow(EXPANSION_RATE, abs(x)) * STARTING_BUCKET_SIZE;
		else
			return pow(EXPANSION_RATE, x) * STARTING_BUCKET_SIZE;
	};

	for (int i = -1 * BUCKETS / 2; i < BUCKETS / 2; ++i)
		cout << i + BUCKETS / 2 << ": " << y(i) << endl;

	return 0;
	// set up favor network
	Network favorNet(vector<pair<size_t, Activation *>>
					 ({ make_pair(384, new Sigmoid), 
					  make_pair(200, new Sigmoid),
					  make_pair(BUCKETS, new Sigmoid) }),
					 .000001,
					 0.0);
	// set up policy network
	Network policyNet(vector<pair<size_t, Activation *>>
					 ({ make_pair(384, new Sigmoid),
					  make_pair(100, new Sigmoid),
					  make_pair(100, new Sigmoid),
					  make_pair(SIZE * SIZE, new Sigmoid) }),
					 .0000005,
					 0.0);

	Agent agent(favorNet, policyNet, 0.4, "agent2.txt");
	agent.load();

	// iterate through all files in directory containing games
	//for (const auto &file : fs::directory_iterator("data"))
	//{
	//	ifstream in(file.path(), ifstream::in);
	//	while (!in.eof())
	//	{
	//		// go through lines until move string begins
	//		string wholeLine, line;
	//		while (std::getline(in, line) && line[0] != '1')
	//			cout << line << endl;

	//		// move string may be spread over multiple lines
	//		wholeLine += line + ' ';
	//		while (std::getline(in, line) && line[0] != '[')
	//		{
	//			cout << line << endl;
	//			wholeLine += line + ' ';
	//		}

	//		agent.train_from_move_string(wholeLine);
	//	}
	//	in.close();
	//}

	//return 0;

	// cpu vs neural net game
	// init game
	Board board;

	// begin game
	// color of player whos turn it is, begin with white
	int turn = 1;
	Color color = Color(turn);

	// display current board
	board.print();

	// game loop, game continues until a player has been checkmated or a stalemate occurs
	int outcome = 0;
	while (!outcome)
	{
		// calculate favor
		cout << "Favor: " << board.favor() << endl;

		// print out whos turn it is
		cout << endl;
		if (color == Color::White)
			cout << "Whites Turn!" << endl << endl;
		else
			cout << "Blacks Turn!" << endl << endl;

		// take min_max turn
		if (color == Color::White)
		{
			Node node = board.min_max_call(board, color, 2);

			cout << "CPU played: "
				<< char(97 + node.current_.second) << node.current_.first + 1 << " -> "
				<< char(97 + node.desired_.second) << node.desired_.first + 1 << endl << endl;

			// make move and end turn
			board.make_move(node.current_, node.desired_);
		}
		else // neural net turn
		{
			Node node = agent.min_max_call(board, color, 2, 4);

			cout << "Neural Net played: "
				<< char(97 + node.current_.second) << node.current_.first + 1 << " -> "
				<< char(97 + node.desired_.second) << node.desired_.first + 1 << endl << endl;

			// make move and end turn
			board.make_move(node.current_, node.desired_);
		}

		// update moves after move played
		board.update_move_set();

		// display current board
		board.print();

		// next turn
		++turn;

		// flip between white and black turns
		color = Color(turn % 2);

		// check outcome of turn
		outcome = board.end_game(color);

		// save game
		board.save_game();
	}

	if (outcome == 1 && color == Color::Black)
		cout << "WHITE HAS WON!" << endl;
	else if (outcome == 1 && color == Color::White)
		cout << "BLACK HAS WON!" << endl;
	else if (outcome == 2 || turn >= 50)
		cout << "STALEMATE!" << endl;
}