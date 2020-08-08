////////////////////////////////////////////////////////////////////////////////
//
// FILE:        board.h
// DESCRIPTION: contains board class implementation
// AUTHOR:      Dan Fabian
// DATE:        7/1/2020

#include "board.h"

////////////////////////////////////////////////////////////////////////////////
//
// BOARD functions
////////////////////////////////////////
// init board
Board::Board() :
	totalGridPoints_(0), turn_(1)
{
	// set up white side
	pieces_.push_front(Piece::rook(Color::White, make_pair(0, 0)));
	pieces_.push_front(Piece::knight(Color::White, make_pair(0, 1)));
	pieces_.push_front(Piece::bishop(Color::White, make_pair(0, 2)));
	pieces_.push_front(Piece::queen(Color::White, make_pair(0, 3)));
	pieces_.push_front(Piece::king(Color::White, make_pair(0, 4)));
	pieces_.push_front(Piece::bishop(Color::White, make_pair(0, 5)));
	pieces_.push_front(Piece::knight(Color::White, make_pair(0, 6)));
	pieces_.push_front(Piece::rook(Color::White, make_pair(0, 7)));

	// set king position
	kingPos_[int(Color::White)] = make_pair(0, 4);

	// place white pawns
	for (int i = 0; i < SIZE; ++i)
		pieces_.push_front(Piece::pawn(Color::White, make_pair(1, i)));

	// set up black side
	pieces_.push_front(Piece::rook(Color::Black, make_pair(SIZE - 1, 0)));
	pieces_.push_front(Piece::knight(Color::Black, make_pair(SIZE - 1, 1)));
	pieces_.push_front(Piece::bishop(Color::Black, make_pair(SIZE - 1, 2)));
	pieces_.push_front(Piece::queen(Color::Black, make_pair(SIZE - 1, 3)));
	pieces_.push_front(Piece::king(Color::Black, make_pair(SIZE - 1, 4)));
	pieces_.push_front(Piece::bishop(Color::Black, make_pair(SIZE - 1, 5)));
	pieces_.push_front(Piece::knight(Color::Black, make_pair(SIZE - 1, 6)));
	pieces_.push_front(Piece::rook(Color::Black, make_pair(SIZE - 1, 7)));

	// set king position
	kingPos_[int(Color::Black)] = make_pair(SIZE - 1, 4);

	// place black pawns
	for (int i = 0; i < SIZE; ++i)
		pieces_.push_front(Piece::pawn(Color::Black, make_pair(6, i)));

	// calc total grid point value
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0; j < SIZE; ++j)
			totalGridPoints_ += get_tile_value(make_pair(i, j));

	update_move_set();
}

////////////////////////////////////////
// board copy constructor, creates a deep copy so current board state isn't modified
Board::Board(const Board &board)
{
	totalGridPoints_ = board.totalGridPoints_;
	pieces_ = board.pieces_;
	kingPos_[0] = board.kingPos_[0];
	kingPos_[1] = board.kingPos_[1];
}

////////////////////////////////////////
// save game to text file to be loaded later
void Board::save_game(const string &game) const
{
	// create file stream
	ofstream out(game);

	out << turn_ << endl;
	for (const Piece &p : pieces_)
		out << p.get_rep() << ' '
			<< p.get_position().first << ' ' << p.get_position().second << ' '
			<< int(p.get_color()) << ' '
			<< p.has_moved() << endl;

	// close stream
	out.close();
}

////////////////////////////////////////
// load game from file
void Board::load_game(const string &game)
{
	// create file stream
	ifstream in(game);

	// get turn number
	string str;
	in >> str;
	turn_ = stoi(str) % 2;

	// get pieces
	pieces_.clear();
	while (!in.eof())
	{
		// get piece rep
		in >> str;
		char rep = str.front();

		// get position
		in >> str;
		int first = stoi(str);
		in >> str;
		int second = stoi(str);

		// get color
		in >> str;
		Color color = Color(stoi(str));

		// get has moved
		in >> str;
		bool hasMoved = stoi(str);

		// add piece to list
		switch (rep)
		{
		case KING_REP:
			pieces_.push_back(Piece::king(color, make_pair(first, second), hasMoved));
			kingPos_[int(color)] = make_pair(first, second);
			break;
		case QUEEN_REP:
			pieces_.push_back(Piece::queen(color, make_pair(first, second), hasMoved));
			break;
		case KNIGHT_REP:
			pieces_.push_back(Piece::knight(color, make_pair(first, second), hasMoved));
			break;
		case BISHOP_REP:
			pieces_.push_back(Piece::bishop(color, make_pair(first, second), hasMoved));
			break;
		case ROOK_REP:
			pieces_.push_back(Piece::rook(color, make_pair(first, second), hasMoved));
			break;
		case PAWN_REP:
			pieces_.push_back(Piece::pawn(color, make_pair(first, second), hasMoved));
			break;
		case EMPTY_REP:
			pieces_.push_back(Piece::empty(color, make_pair(first, second), hasMoved));
			break;
		}
	}

	// update moves
	update_move_set();

	// close stream
	in.close();
}

////////////////////////////////////////
// print board
void Board::print() const
{
	// print pieces and row numbers
	for (int i = SIZE - 1; i >= 0; --i)
	{
		cout << i + 1 << "| ";
		for (int j = 0; j < SIZE; ++j)
		{
			// doesnt matter what piece is searched for, only position matters
			auto it = cfind(pieces_, make_pair(i, j));

			if (it != pieces_.end()) // piece
			{
				if (it->get_color() == Color::Black)
					cout << "\x1B[3;47;30m" << it->get_rep() << "\033[0m ";
				else
					cout << it->get_rep() << ' ';
			}
			else // empty space
				cout << EMPTY_REP << ' ';
		}
		
		cout << endl;
	}

	// print column letters
	cout << "   "; // line up with pieces
	for (int i = 0; i < SIZE; ++i)
		cout << "_ ";
	cout << endl;

	cout << "   "; // line up with pieces
	for (int i = 0; i < SIZE; ++i)
		cout << char(97 + i) << ' ';
	cout << endl;
}

////////////////////////////////////////
// check if a player is in check
bool Board::player_in_check(const Color &color) const
{
	// create blank position to check with
	Position blank(-1, -1);

	// pieces that are closest to the king in each direction
	// direction elements: 0 = up, 1 = down, 2 = left, 3 = right
	//		4 = up left, 5 = up right, 6 = down left, 7 = down right
	vector<Piece> direction(8);
	for (int i = 0; i < 8; ++i)
		direction[i] = Piece::empty(Color::Empty, blank);

	// get king int
	int kingNum = int(color);

	// go through all pieces
	for (const Piece &p : pieces_)
		if (p.get_rep() != EMPTY_REP)
		{
			Position piecePos = p.get_position();

			// check up and down
			if (kingPos_[kingNum].second == piecePos.second)
			{
				// check up
				if ((piecePos.first < direction[0].get_position().first || direction[0].get_position() == blank) &&
					piecePos.first > kingPos_[kingNum].first)
					direction[0] = p;
				// check down
				else if ((piecePos.first > direction[1].get_position().first || direction[1].get_position() == blank) &&
						 piecePos.first < kingPos_[kingNum].first)
					direction[1] = p;
			}
			// check left and right
			else if (kingPos_[kingNum].first == piecePos.first)
			{
				// check left
				if ((piecePos.second > direction[2].get_position().second || direction[2].get_position() == blank) &&
					piecePos.second < kingPos_[kingNum].second)
					direction[2] = p;
				// check right
				else if ((piecePos.second < direction[3].get_position().second || direction[3].get_position() == blank) &&
						 piecePos.second > kingPos_[kingNum].second)
					direction[3] = p;
			}
			// check up left and down right
			else if (double(kingPos_[kingNum].second - piecePos.second) / double(kingPos_[kingNum].first - piecePos.first) == -1) // check slope 
			{
				// check up left
				if ((distance(kingPos_[kingNum], piecePos) < distance(kingPos_[kingNum], direction[4].get_position()) ||
					 direction[4].get_position() == blank) &&
					piecePos.first < kingPos_[kingNum].first)
					direction[4] = p;
				// check down right
				else if ((distance(kingPos_[kingNum], piecePos) < distance(kingPos_[kingNum], direction[7].get_position()) ||
						  direction[7].get_position() == blank) &&
						 piecePos.first > kingPos_[kingNum].first)
					direction[7] = p;
			}
			// check up right and down left
			else if (double(kingPos_[kingNum].second - piecePos.second) / double(kingPos_[kingNum].first - piecePos.first) == 1) // check slope
			{
				// check up right
				if ((distance(kingPos_[kingNum], piecePos) < distance(kingPos_[kingNum], direction[5].get_position()) ||
					 direction[5].get_position() == blank) &&
					piecePos.first > kingPos_[kingNum].first)
					direction[5] = p;
				// check down left
				else if ((distance(kingPos_[kingNum], piecePos) < distance(kingPos_[kingNum], direction[6].get_position()) ||
						  direction[6].get_position() == blank) &&
						 piecePos.first < kingPos_[kingNum].first)
					direction[6] = p;
			}

			// check all knight positions that can attack the king
			if (p.get_rep() == KNIGHT_REP && p.get_color() != color &&
				 // check up right moves
				(make_pair(kingPos_[kingNum].first + 2, kingPos_[kingNum].second + 1) == piecePos ||
				 make_pair(kingPos_[kingNum].first + 1, kingPos_[kingNum].second + 2) == piecePos ||
				 // check up left moves
				 make_pair(kingPos_[kingNum].first + 2, kingPos_[kingNum].second - 1) == piecePos ||
				 make_pair(kingPos_[kingNum].first + 1, kingPos_[kingNum].second - 2) == piecePos ||
				 // check down right moves
				 make_pair(kingPos_[kingNum].first - 2, kingPos_[kingNum].second + 1) == piecePos ||
				 make_pair(kingPos_[kingNum].first - 1, kingPos_[kingNum].second + 2) == piecePos ||
				 // check down left moves
				 make_pair(kingPos_[kingNum].first - 2, kingPos_[kingNum].second - 1) == piecePos ||
				 make_pair(kingPos_[kingNum].first - 1, kingPos_[kingNum].second - 2) == piecePos))
				return true;

			// check white pawn attacks
			if (p.get_rep() == PAWN_REP && p.get_color() != color && color == Color::Black &&
				// up left attack
				piecePos.first + 1 == kingPos_[kingNum].first&&
				(piecePos.second - 1 == kingPos_[kingNum].second || piecePos.second + 1 == kingPos_[kingNum].second))
				return true;
			// check black pawn attacks
			else if (p.get_rep() == PAWN_REP && p.get_color() != color && color == Color::White &&
					 // up left attack
					 piecePos.first - 1 == kingPos_[kingNum].first &&
					 (piecePos.second - 1 == kingPos_[kingNum].second || piecePos.second + 1 == kingPos_[kingNum].second))
				return true;
		}

	// now go through all directions the king can be attacked and verify it is a piece that can attack
	// check rook attacking directions
	for (int i = 0; i < 4; ++i)
		if ((direction[i].get_rep() == ROOK_REP || direction[i].get_rep() == QUEEN_REP) && direction[i].get_color() != color)
			return true;

	// check bishop attacking directions
	for (int i = 4; i < 8; ++i)
		if ((direction[i].get_rep() == BISHOP_REP || direction[i].get_rep() == QUEEN_REP) && direction[i].get_color() != color)
			return true;

	// if nothing can attack the king then its not in check
	return false;
}

////////////////////////////////////////
// moving a piece
void Board::make_move(const Position &currentPos, const Position &desiredPos)
{
	// get piece
	auto p = find(pieces_, currentPos);

	// get piece color
	Color color = p->get_color();

	// reset empty tile from en passant
	for (auto it = pieces_.begin(); it != pieces_.end(); ++it)
		if (it->get_color() == color &&
			it->get_rep() == EMPTY_REP)
		{
			pieces_.erase(it);
			break;
		}

	// get piece, iterator may have gone stale from erase function
	p = find(pieces_, currentPos);

	// update board
	if (desiredPos.first == -1) // check king side castle 
	{
		// get rook
		auto rook = find(pieces_, make_pair(currentPos.first, currentPos.second + 3));

		// change king position
		p->set_position(make_pair(currentPos.first, currentPos.second + 2));
		p->set_has_moved(true);
		kingPos_[int(color)] = p->get_position();

		// change rook position
		rook->set_position(make_pair(currentPos.first, currentPos.second + 1));
		rook->set_has_moved(true);
	}
	else if (desiredPos.first == -2) // check queen side castle 
	{
		// get rook
		auto rook = find(pieces_, make_pair(currentPos.first, currentPos.second - 4));

		// change king position
		p->set_position(make_pair(currentPos.first, currentPos.second - 2));
		p->set_has_moved(true);
		kingPos_[int(color)] = p->get_position();

		// change rook position
		rook->set_position(make_pair(currentPos.first, currentPos.second - 1));
		rook->set_has_moved(true);
	}
	else if (desiredPos.first == -3 && color == Color::Black) // black pawn jump
	{
		// change pawn position
		p->set_position(make_pair(currentPos.first - 2, currentPos.second));

		// set empty position color to black for en passant, will be changed back after opponents turn
		pieces_.push_front(Piece::empty(color, make_pair(currentPos.first - 1, currentPos.second)));

		// set pawn has moved
		p->set_has_moved(true);
	}
	else if (desiredPos.first == -3 && color == Color::White) // white pawn jump
	{
		// change pawn position
		p->set_position(make_pair(currentPos.first + 2, currentPos.second));

		// set empty position color to black for en passant, will be changed back after opponents turn
		pieces_.push_front(Piece::empty(color, make_pair(currentPos.first + 1, currentPos.second)));

		// set pawn has moved
		p->set_has_moved(true);
	}
	else if (desiredPos.first == -4) // en passant
	{
		// delete whatever piece was at desired pos and move piece to desired pos
		PieceList::iterator it;

		// check color
		if (color == Color::Black)
		{
			it = find(pieces_, make_pair(currentPos.first - 1, desiredPos.second));
			p->set_position(make_pair(currentPos.first - 1, desiredPos.second));
		}
		else
		{
			it = find(pieces_, make_pair(currentPos.first + 1, desiredPos.second));
			p->set_position(make_pair(currentPos.first + 1, desiredPos.second));
		}

		pieces_.erase(it);

		// delete pawn that was captured
		it = find(pieces_, make_pair(currentPos.first, desiredPos.second));
		pieces_.erase(it);
	}
	else if ((desiredPos.first == 0 || // check pawn queen promotion
			  desiredPos.first == SIZE - 1) &&
			 p->get_rep() == PAWN_REP)
	{
		// delete pawn and replace with queen
		auto it = find(pieces_, currentPos);
		pieces_.erase(it);

		// find out if there was piece at the desired pos
		it = find(pieces_, desiredPos);

		if (it != pieces_.end()) // there is a piece at that pos
			pieces_.erase(it);

		// create queen
		pieces_.push_front(Piece::queen(color, desiredPos, true));
	}
	else
	{
		// find out if there was piece at the desired pos
		auto it = find(pieces_, desiredPos);

		if (it != pieces_.end()) // there is a piece at that pos
			pieces_.erase(it);

		// change position to desired
		p->set_position(desiredPos);

		// if it was a king, keep track of position
		if (p->get_rep() == KING_REP)
			kingPos_[int(color)] = p->get_position();

		p->set_has_moved(true);
	}
}

////////////////////////////////////////
// check for stalemate or checkmate, 0 = game not over, 1 = checkmate, 2 = stalemate
int Board::end_game(const Color &color) const
{
	bool check = player_in_check(color);

	// find all pieces that have moves
	vector<Piece> colorPiecesWithMoves, piecesWithMoves;
	for (const Piece &p : pieces_)
		// if piece has at least one move, add it to the list
		if (!p.move_list().empty())
		{
			if (p.get_color() == color)
				colorPiecesWithMoves.push_back(p);

			piecesWithMoves.push_back(p);
		}

	// if no piece can move, and player is in check then player has been checkmated
	if (colorPiecesWithMoves.empty() && check)
		return 1;
	// if no pieces can move, and player isnt in check then player has been stalemated
	else if (colorPiecesWithMoves.empty())
		return 2;
	// king vs king stalemate
	else if (piecesWithMoves.size() == 2)
		return 2;
	// check all unsufficient material to checkmate possibilities
	else if (piecesWithMoves.size() == 3 || piecesWithMoves.size() == 4)
	{
		// count how many of which pieces are in left
		int darkBishop = 0,
			lightBishop = 0,
			knight = 0;
		for (const Piece &p : pieces_)
		{
			char rep = p.get_rep();

			if (rep == BISHOP_REP &&
				(p.get_position().first + p.get_position().second) % 2 == 1)
				++lightBishop;
			else if (rep == BISHOP_REP &&
					 (p.get_position().first + p.get_position().second) % 2 == 0)
				++darkBishop;
			else if (rep == KNIGHT_REP)
				++knight;
		}

		// king vs king and bishop
		if ((((darkBishop == 1 && lightBishop == 0) ||
			  (darkBishop == 0 && lightBishop == 1)) &&
			 knight == 0 &&
			 piecesWithMoves.size() == 3) ||
			// king vs king and knight
			(darkBishop == 0 && lightBishop == 0 &&
			 knight == 1 &&
			 piecesWithMoves.size() == 3) ||
			// bishop and king vs bishop and king with same color bishop
			(darkBishop == 2 || lightBishop == 2))
			return 2;
	}

	return 0;
}

////////////////////////////////////////
// updates all pieces move lists
void Board::update_move_set()
{
	for (Piece &p : pieces_)
		p.get_moves(*this);
}

////////////////////////////////////////
// calculate favor of current board position
// note: positive = favor of white, negative = favor of black, 0 = neutral
double Board::favor() const
{
	// get points from pieces for each player
	double favor[2] = { 0, 0 }, positionFavor[2] = { 0, 0 };

	for (const Piece &p : pieces_)
	{
		Color color = p.get_color();
		Position pos = p.get_position();

		// get points from pieces for each player
		if (color == Color::White)
			favor[int(Color::White)] += p.get_points();
		else
			favor[int(Color::Black)] += p.get_points();

		// get points for tile control
		vector<Position> moves;

		// change moves for pawns
		if (p.get_rep() == PAWN_REP)
		{
			// check white pawns
			if (color == Color::White)
			{
				// check up/left attack
				moves.push_back(make_pair(pos.first + 1, pos.second - 1));
				// check up/right attack
				moves.push_back(make_pair(pos.first + 1, pos.second + 1));
			}
			// check black pawns
			else
			{
				// check down/right attack
				moves.push_back(make_pair(pos.first - 1, pos.second + 1));
				// check down/left attack
				moves.push_back(make_pair(pos.first - 1, pos.second - 1));
			}
		}

		// switch moves if not a pawn
		if (moves.empty())
			moves = p.move_list();

		double value = 0;
		for (const Position &move : moves)
			switch (move.first)
			{
			case -1: // king side castle
				value = get_tile_value(make_pair(0, move.second)) * 2;
				break;

			case -2: // queen side castle
				value = get_tile_value(make_pair(0, move.second)) * 2;
				break;

			case -3: // pawn jump
				value = 0;
				break;

			case -4: // en passant
				value = get_tile_value(make_pair(2, move.second)) + PAWN_POINTS;
				break;

			default:
				value = get_tile_value(move);

				// if attacking a piece at this position, add its point value to the tile
				auto p = cfind(pieces_, move);
				if (p != pieces_.end() &&
					p->get_color() != color)
					value += p->get_points() / QUEEN_POINTS;
				break;
			}

		positionFavor[int(color)] += value;
	}

	// calculate final favor
	return (favor[int(Color::White)] + (positionFavor[int(Color::White)] / 10)) -
		(favor[int(Color::Black)] + (positionFavor[int(Color::Black)] / 10));
}

////////////////////////////////////////
// min max calling func for cpu moves
Node Board::min_max_call(const Board &board, const Color &maximizingColor, const int &depth)
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

	Node value(0, Position(), Position());
	if (maximizingColor == Color::White)
	{
		value.value_ = -1 * std::numeric_limits<double>::max();

		// for every move
		for (const Piece &p : board.pieces_)
			if (p.get_color() == maximizingColor)
				for (const Position &move : p.move_list())
				{
					// copy board
					Board update(board);

					// move piece
					update.make_move(p.get_position(), move);

					// update move set after move completed
					update.update_move_set();

					value = max(value,
								Node(min_max(update, depth - 1, alpha, beta, Color::Black),
									 p.get_position(), move));
			
					// set alpha
					alpha = max(value.value_, alpha);
				}
	}
	else
	{
		value.value_ = std::numeric_limits<double>::max();

		// for every move
		for (const Piece &p : board.pieces_)
			if (p.get_color() == maximizingColor)
				for (const Position &move : p.move_list())
				{
					// copy board
					Board update(board);

					// move piece
					update.make_move(p.get_position(), move);

					// update move set after move completed
					update.update_move_set();

					value = min(value,
								Node(min_max(update, depth - 1, alpha, beta, Color::White),
									 p.get_position(), move));

					// set beta
					beta = min(beta, value.value_);
				}
	}

	return value;
}

////////////////////////////////////////
// min max branching function
double Board::min_max(const Board &board, int depth, double alpha, double beta, Color maximizingColor)
{
	//cout << depth << endl;
	// check if game is over
	int outcome = board.end_game(Color(maximizingColor));
	if (depth == 0 && outcome == 0)
		return board.favor();
	else if (outcome == 1 && maximizingColor == Color::White)
		return -1 * std::numeric_limits<double>::max();
	else if (outcome == 1 && maximizingColor == Color::Black)
		return std::numeric_limits<double>::max();
	else if (outcome == 2)
		return 0.0;

	double value;
	if (maximizingColor == Color::White)
	{
		value = -1 * std::numeric_limits<double>::max();

		// for every move
		for (const Piece &p : board.pieces_)
			if (p.get_color() == maximizingColor)
				for (const Position &move : p.move_list())
				{
					// copy board
					Board update(board);

					// move piece
					update.make_move(p.get_position(), move);

					// update move set after move completed
					update.update_move_set();

					value = max(value, min_max(update, depth - 1, alpha, beta, Color::Black));

					// set alpha
					alpha = max(value, alpha);

					if (alpha >= beta)
						return value;
				}
				
	}
	else // maximizeColor == Black
	{
		value = std::numeric_limits<double>::max();

		// for every move
		for (const Piece &p : board.pieces_)
			if (p.get_color() == maximizingColor)
				for (const Position &move : p.move_list())
				{
					// copy board
					Board update(board);
					
					// move piece
					update.make_move(p.get_position(), move);

					// update move set after move completed
					update.update_move_set();

					value = min(value, min_max(update, depth - 1, alpha, beta, Color::White));

					// set beta
					beta = min(beta, value);

					if (alpha >= beta)
						return value;
				}
	}

	return value;
}

////////////////////////////////////////
// play chess against basic cpu
void Board::play(const Color &cpu, const int &depth)
{
	// color of player whos turn it is, begin with white
	Color color = Color(turn_);

	// display current board
	print();

	// game loop, game continues until a player has been checkmated or a stalemate occurs
	int outcome = 0;
	while (!outcome)
	{
		// print out whos turn it is
		cout << endl;
		if (color == Color::White)
			cout << "Whites Turn!" << endl << endl;
		else
			cout << "Blacks Turn!" << endl << endl;

		// take turn

		// calculate favor
		cout << "Favor: " << favor() << endl;

		// if cpu is playing, take turn
		if (cpu == color)
		{
			Node node = min_max_call(*this, color, depth);

			cout << "CPU played: " 
				<< char(97 + node.current_.second) << node.current_.first + 1  << " -> " 
				<< char(97 + node.desired_.second) << node.desired_.first + 1 << endl << endl;

			// make move and end turn
			make_move(node.current_, node.desired_);
		}
		// human turn
		else
		{
			cout << "player in check " << player_in_check(color) << endl;

			// find all white pieces that have moves
			vector<Piece> piecesWithMoves;
			for (const Piece &p : pieces_)
				if (p.get_color() == color &&
					!p.move_list().empty()) // if piece has at least one move, add it to the list
						piecesWithMoves.push_back(p);

			// move selection loop
			bool selectionMade = false;
			int pieceNum, moveNum;
			while (!selectionMade)
			{
				// output selection of moves
				cout << "Pieces to move:" << endl << endl;
				int i = 0;
				for (const Piece &p : piecesWithMoves)
				{
					int row = p.get_position().first, col = p.get_position().second;

					cout << i << ": " << p.get_rep() << ' '
						<< char(97 + col) << row + 1 << endl;

					++i;
				}

				// select a piece to move
				cout << "Selection: ";
				cin >> pieceNum;

				// view possible moves of chosen piece
				cout << "Possible moves:" << endl << endl;

				piecesWithMoves[pieceNum].print_moves();
				cout << piecesWithMoves[pieceNum].move_list().size() << ": select a different piece" << endl;

				// select a position to move
				cout << "Selection: ";
				cin >> moveNum;

				if (moveNum != piecesWithMoves[pieceNum].move_list().size()) 
					selectionMade = true;
			}

			// make move and end turn
			make_move(piecesWithMoves[pieceNum].get_position(), piecesWithMoves[pieceNum].move_list()[moveNum]);

		} // end of humans turn

		cout << endl;

		// update moves after move played
		update_move_set();

		// display current board
		print();

		// check outcome of turn
		outcome = end_game(color);

		// next turn
		++turn_;

		// flip between white and black turns
		color = Color(turn_ % 2);

		// save game
		cout << endl << "Save game? (y/n): ";
		char ans; cin >> ans;

		if (ans == 'y') save_game();
	}

	if (outcome == 1 && color == Color::Black)
		cout << "WHITE HAS WON!" << endl;
	else if (outcome == 1 && color == Color::White)
		cout << "BLACK HAS WON!" << endl;
	else if (outcome == 2)
		cout << "STALEMATE!" << endl;
}