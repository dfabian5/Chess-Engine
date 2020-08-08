#ifndef AGENT_H
#define AGENT_H

////////////////////////////////////////////////////////////////////////////////
//
// FILE:        agent.h
// DESCRIPTION: contains agent class definition
// AUTHOR:      Dan Fabian
// DATE:        7/24/2020

#include "network.h"
#include "board.h"
#include "agent_utility.h"
#include <cctype>
#include <map>

using std::map;

ValD dist(0.0, BUCKETS); // just used for testing

////////////////////////////////////////////////////////////////////////////////
//
// AGENT
class Agent {
public:
	// constructor
	Agent(const Network &favorNet, const Network &policyNet, const double &discount, const string &fileName) : 
		favorNet_(favorNet), policyNet_(policyNet), discount_(discount), fileName_(fileName) {}

	// methods
	void load() {
		favorNet_.load("favor_" + fileName_); 
		policyNet_.load("policy_" + fileName_); 
	} 
	void train_from_move_string               (const string &str); // trains favorNet from a string of moves
	vector<Piece> top_n_likely_pieces_to_move (const Board &board, const Color &color, const int &n);
	Node min_max_call                         (const Board &board, const Color &maximizingColor, const int &depth, const int &n);
	double min_max                            (const Board &board, int depth, double alpha, double beta, Color maximizingColor, const int &n);

private:
	Network favorNet_;
	Network policyNet_;
	double discount_;
	string fileName_;

};

////////////////////////////////////////////////////////////////////////////////
//
// AGENT functions
////////////////////////////////////////
// trains network from a pgn file loaded into a string
void Agent::train_from_move_string(const string &str)
{
	// init board
	Board board;

	// split string to seperate moves
	list<string> subStrings = split_string(str);

	// holds board state and favor of the board at that step
	list<pair<ValD, double>> favorSteps;

	// holds board state and index of board pos of piece that moved this step
	list<pair<ValD, size_t>> policySteps;

	cout << "Reading: " << str << endl;

	// loop through game and create a list of board states and favor
	int i = 0;
	for (auto it = subStrings.begin(); it != subStrings.end(); ++it, ++i)
	{
		// every third string is a move number so skip
		if (i % 3 != 0 
			&& (*it != "1-0" || *it != "0-1" || *it != "1/2-1/2"))
		{
			string moveStr = *it;

			// remove + from end of string
			if (moveStr.back() == '+' || moveStr.back() == '#') moveStr.erase(--moveStr.end());

			// info about move
			Position desired(-10, -10), current(-10, -10);
			char rep = '@';
			Color color = i % 3 == 1 ? Color::White : Color::Black;

			// if first char is lower case and length is 2 then it was a pawn move in that file
			if (islower(moveStr[0]) && moveStr.length() == 2)
			{
				current.second = moveStr[0] - 97;

				desired.first = moveStr[1] - 49;
				desired.second = current.second;

				// check if move was a pawn jump
				int checkMult = color == Color::White ? -1 : 1;
				auto piece = cfind(board.get_pieces(), make_pair(desired.first + 2 * checkMult, current.second));
				if (piece != board.get_pieces().end()
					&& piece->get_rep() == PAWN_REP
					&& cfind(board.get_pieces(), make_pair(desired.first + 1 * checkMult, current.second)) == board.get_pieces().cend())
				{
					current = piece->get_position();
					desired.first = -3;
				}
				
				rep = PAWN_REP;
			}
			// pawn takes move
			else if (islower(moveStr[0]) && moveStr.length() == 4 && moveStr[1] == 'x')
			{
				current.second = moveStr[0] - 97;

				desired.first = moveStr[3] - 49;
				desired.second = moveStr[2] - 97;

				// check if it was an en passant
				if (cfind(board.get_pieces(), desired)->get_rep() == EMPTY_REP)
					desired.first = -4;

				rep = PAWN_REP;
			}
			// king side castle
			else if (moveStr == "O-O")
			{
				desired.first = -1;
				desired.second = -1;

				current = board.get_king_pos(color);
			}
			// queen side castle
			else if (moveStr == "O-O-O")
			{
				desired.first = -2;
				desired.second = -2;

				current = board.get_king_pos(color);
			}
			// normal piece move where no other piece of that type and color can make that move
			else if (isupper(moveStr[0]) && moveStr.length() == 3)
			{
				desired.first = moveStr[2] - 49;
				desired.second = moveStr[1] - 97;

				rep = moveStr[0];
			}
			// normal piece move where no other piece of that type and color can make that move and a piece was taken
			else if (isupper(moveStr[0]) && moveStr.length() == 4 && moveStr[1] == 'x')
			{
				desired.first = moveStr[3] - 49;
				desired.second = moveStr[2] - 97;

				rep = moveStr[0];
			}
			// normal piece move where a piece of the same type and color can make the same move
			else if (isupper(moveStr[0]) && moveStr.length() == 4)
			{
				if (isdigit(moveStr[1])) current.first = moveStr[1] - 49;
				else current.second = moveStr[1] - 97;

				desired.first = moveStr[3] - 49;
				desired.second = moveStr[2] - 97;

				rep = moveStr[0];
			}
			// normal piece move where a piece of the same type and color can make the same move and a piece was taken
			else if (isupper(moveStr[0]) && moveStr.length() == 5 && moveStr[2] == 'x')
			{
				if (isdigit(moveStr[1])) current.first = moveStr[1] - 49;
				else current.second = moveStr[1] - 97;

				desired.first = moveStr[4] - 49;
				desired.second = moveStr[3] - 97;

				rep = moveStr[0];
			}
			// check if string was outcome ex: 1-0, 1/2-1/2 or 0-1
			else
			{
				// if game ended, add a bonus to favor of last move played
				double winningBonus = 0;

				// white won
				if (moveStr[0] == '1' && moveStr[1] == '-')
					winningBonus += 10.0;
				// black won
				else if (moveStr[0] == '0' && moveStr[1] == '-')
					winningBonus -= 10.0;

				favorSteps.push_front(make_pair(create_board_state(board, favorNet_.getInputSize()), board.favor()));
				favorSteps.front().second += winningBonus;
				break;
			}

			// if castling or pawn jump, enough info was found to make move
			bool moveFound = false;
			if (!(desired.first < 0) || desired.first == -4)
				for (const Piece &p : board.get_pieces())
				{
					// check if only the desired pos was found
					if ((current == make_pair(-10, -10)
						 || (current.first == p.get_position().first
							 || current.second == p.get_position().second))
						&& p.get_rep() == rep
						&& p.get_color() == color)
						for (const Position &pos : p.move_list())
							if (pos == desired)
							{
								current = p.get_position();
								moveFound = true;
								break;
							}

					if (moveFound) break;
				}

			// before move is made, add state and current favor to list
			ValD state = create_board_state(board, favorNet_.getInputSize());

			// find favor
			double compoundDiscount = 1.0;
			double favor = board.favor();
			for (auto it = favorSteps.begin(); it != favorSteps.end(); ++it, compoundDiscount *= discount_)
				it->second += favor * compoundDiscount;

			favorSteps.push_front(make_pair(state, favor));
			policySteps.push_back(make_pair(state, get_board_index(current)));

			// make move and update board move set
			board.make_move(current, desired);
			board.update_move_set();
		}
	}

	// train models after game is over
	cout << "Training..." << endl;

	// train favorNet
	double totalLoss = 0, totalFavor = 0;
	for (auto it = favorSteps.rbegin(); it != favorSteps.rend(); ++it)
	{
		ValD alpha = favorNet_.forwardPropagation(it->first);

		ValD ans(0.0, favorNet_.getOutputSize());
		ans[favor_to_index(it->second)] = 1.0;
		dist[favor_to_index(it->second)] += 1.0;

		totalLoss += abs(alpha - ans).sum();

		favorNet_.backPropagation(alpha, ans);
	}
	cout << "Average Favor Loss: " << totalLoss / favorSteps.size() << endl << endl;

	// print favor index distribution for testing
	for (int j = 0; j < dist.size(); ++j)
		cout << j << ": " << dist[j] << endl;
	cout << endl;

	// train policyNet
	totalLoss = 0;
	for (auto it = policySteps.begin(); it != policySteps.end(); ++it)
	{
		ValD alpha = policyNet_.forwardPropagation(it->first);

		ValD ans(0.0, policyNet_.getOutputSize());
		ans[it->second] = 1.0;

		totalLoss += abs(alpha - ans).sum();

		policyNet_.backPropagation(alpha, ans);
	}
	cout << "Average Policy Loss: " << totalLoss / policySteps.size() << endl << endl;

	// save neural nets
	favorNet_.save("favor_" + fileName_);
	policyNet_.save("policy_" + fileName_);
}

////////////////////////////////////////
// finds the top most likely pieces to be moved using the policy network, helps prune search space for min max
vector<Piece> Agent::top_n_likely_pieces_to_move(const Board &board, const Color &color, const int &n)
{
	ValD state = create_board_state(board, policyNet_.getInputSize());
	ValD output = policyNet_.forwardPropagation(state);

	// create map of pieces
	map<double, Position> piecesToMove;
	for (size_t i = 0; i < output.size(); ++i)
		piecesToMove.insert(make_pair(output[i], get_board_position(i)));

	int j = 0;
	vector<Piece> topPieces;
	for (auto it = piecesToMove.rbegin(); it != piecesToMove.rend() && j < n; ++it)
	{
		auto pieceIt = cfind(board.get_pieces(), it->second);
		if (pieceIt != board.get_pieces().end()
			&& pieceIt->get_color() == color
			&& pieceIt->move_list().size() != 0)
		{
			topPieces.push_back(*pieceIt);
			++j;
		}
	}

	return topPieces;
}

////////////////////////////////////////
// min max calling func for cpu moves
Node Agent::min_max_call(const Board &board, const Color &maximizingColor, const int &depth, const int &n)
{
	// init alpha and beta
	double alpha = -1 * std::numeric_limits<double>::max(),
		beta = std::numeric_limits<double>::max();

	// get number of threads available
	int threads = std::thread::hardware_concurrency();
	if (threads == 0) // couldnt find threads available
		threads = 1;

	// create vector for async functions
	//vector<future<double>> minMaxAsync;

	// use policy net to find most probable pieces to move
	vector<Piece> topPieces = top_n_likely_pieces_to_move(board, maximizingColor, n);

	Node value(0, Position(), Position());
	if (maximizingColor == Color::White)
		value.value_ = -1 * std::numeric_limits<double>::max();
	else
		value.value_ = std::numeric_limits<double>::max();

	cout << topPieces.size() << endl;

	// for every move
	vector<Node> tieMoves;
	double prevVal = 0;
	for (const Piece &p : topPieces)
		for (const Position &move : p.move_list())
		{
			// copy board
			Board update(board);

			// move piece
			update.make_move(p.get_position(), move);

			// update move set after move completed
			update.update_move_set();

			if (maximizingColor == Color::White)
				value = max(Node(min_max(update, depth - 1, alpha, beta, Color::Black, n),
							p.get_position(), move), value);
			else
				value = min(Node(min_max(update, depth - 1, alpha, beta, Color::White, n),
							p.get_position(), move), value);

			// move tied with previous
			if (prevVal != value.value_)
				tieMoves.clear();		
			prevVal = value.value_;
			tieMoves.push_back(value);
			
			cout << "tieMoves size: " << tieMoves.size() << endl;
			// set alpha/beta
			if (maximizingColor == Color::White)
				alpha = max(value.value_, alpha);
			else
				beta = min(beta, value.value_);
		}

	// select a random move amongst ties
	std::default_random_engine generator;
	std::uniform_int_distribution<int> dist(0, tieMoves.size());
	int selection = dist(generator);
	return tieMoves[selection];
}

////////////////////////////////////////
// min max branching function
double Agent::min_max(const Board &board, int depth, double alpha, double beta, Color maximizingColor, const int &n)
{
	//cout << depth << endl;
	// check if game is over
	int outcome = board.end_game(Color(maximizingColor));

	cout << valarray_argmax(favorNet_.forwardPropagation(create_board_state(board, favorNet_.getInputSize()))) << endl;
	if (depth == 0 && outcome == 0)
		return valarray_argmax(favorNet_.forwardPropagation(create_board_state(board, favorNet_.getInputSize())));
	else if (outcome == 1 && maximizingColor == Color::White)
		return -1 * std::numeric_limits<double>::max();
	else if (outcome == 1 && maximizingColor == Color::Black)
		return std::numeric_limits<double>::max();
	else if (outcome == 2)
		return 0.0;

	// use policy net to find most probable pieces to move
	vector<Piece> topPieces = top_n_likely_pieces_to_move(board, maximizingColor, n);

	double value;
	if (maximizingColor == Color::White)
		value = -1 * std::numeric_limits<double>::max();
	else 
		value = std::numeric_limits<double>::max();

	// for every move
	for (const Piece &p : board.get_pieces())
		if (p.get_color() == maximizingColor)
			for (const Position &move : p.move_list())
			{
				// copy board
				Board update(board);

				// move piece
				update.make_move(p.get_position(), move);

				// update move set after move completed
				update.update_move_set();

				if (maximizingColor == Color::White)
					value = max(value, min_max(update, depth - 1, alpha, beta, Color::Black, n));
				else
					value = min(value, min_max(update, depth - 1, alpha, beta, Color::White, n));

				// set alpha/beta
				if (maximizingColor == Color::White)
					alpha = max(value, alpha);
				else
					beta = min(beta, value);

				if (alpha >= beta)
					return value;
			}

	return value;
}

#endif // AGENT_H
