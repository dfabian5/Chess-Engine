#ifndef AGENT_UTILITY_H
#define AGENT_UTILITY_H

////////////////////////////////////////////////////////////////////////////////
//
// FILE:        agent_utility.h
// DESCRIPTION: contains agent class helper functions
// AUTHOR:      Dan Fabian
// DATE:        7/24/2020

#include <string>
#include <list>
#include <sstream>
#include <cmath>

using std::string;
using std::list;
using std::stringstream;

////////////////////////////////////////////////////////////////////////////////
//
// CONSTANTS
const int BUCKETS = 60; // buckets for favorNet to output to
const double STARTING_BUCKET_SIZE = .001; // starting increment
const double EXPANSION_RATE = 1.1; // rate that bucket sizes expand at

////////////////////////////////////////////////////////////////////////////////
//
// HELPER functions
////////////////////////////////////////
// split string by a delimiter
list<string> split_string(const string &str)
{
	list<string> subStrings;
	stringstream stream(str);
	string sub;
	while (!stream.eof())
	{
		stream >> sub;
		subStrings.push_back(sub);
	}
	
	return subStrings;
}

////////////////////////////////////////
// creates a valarray of the board state be used as input to the network
ValD create_board_state(const Board &board, const int &inputSize)
{
	ValD state(0.0, inputSize);
	for (const Piece &p : board.get_pieces())
	{
		// map each piece to an input neuron
		Position pos = p.get_position();
		int map = (pos.first * SIZE + pos.second) * 6; // * 6 for the six pieces

		// piece map
		switch (p.get_rep())
		{
		case KING_REP:
			map += 1;
			break;
		case QUEEN_REP:
			map += 2;
			break;
		case KNIGHT_REP:
			map += 3;
			break;
		case BISHOP_REP:
			map += 4;
			break;
		case ROOK_REP:
			map += 5;
			break;
		case PAWN_REP:
			break; // leave map the same if pawn
		}

		// activate input neuron
		if (p.get_color() == Color::Black)
			state[map] = -1.0;
		else
			state[map] = 1.0;
	}

	return state;
}

////////////////////////////////////////
// takes in a position and returns single dim board index
size_t get_board_index(const Position &pos)
{
	return pos.first * SIZE + pos.second;
}

////////////////////////////////////////
// takes in a position and returns single dim board index
Position get_board_position(const int &index)
{
	return Position(index / SIZE, index % SIZE);
}

////////////////////////////////////////
// takes in favor and outputs an index
size_t favor_to_index(const double &favor)
{
	auto y = [&](const int &x) {
		if (x < 0)
			return -1 * pow(EXPANSION_RATE, abs(x)) * STARTING_BUCKET_SIZE;
		else
			return pow(EXPANSION_RATE, x) * STARTING_BUCKET_SIZE;
	};

	// check ends
	if (favor < y(-1 * BUCKETS / 2))
		return 0;
	else if (y(BUCKETS / 2) < favor)
		return BUCKETS - 1;

	for (int i = -1 * BUCKETS / 2; i < BUCKETS / 2; ++i)
		if (y(i) <= favor && favor < y(i + 1))
			return i + BUCKETS / 2;

	// bucket isnt found
	return -1;
}

////////////////////////////////////////
// finds argmax in a valarray
size_t valarray_argmax(const ValD &arr)
{
	size_t argMax = 0;
	for (size_t i = 1; i < arr.size(); ++i)
		if (arr[i] >= arr[argMax])
			argMax = i;

	return argMax;
}

#endif // AGENT_UTILITY_H