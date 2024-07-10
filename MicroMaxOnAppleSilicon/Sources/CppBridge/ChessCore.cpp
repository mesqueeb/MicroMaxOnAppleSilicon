#include <vector>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "VirtualNode.hpp"
#include "VirtualMove.hpp"
#include "./include/ChessCore.hpp"
#include "BoardConverter.hpp"
#include "ChessPosition.hpp"
#include "ChessNotation.hpp"
#include "Bitboard/BMoveGen.h"
#include "Bitboard/BPosition.h"

using namespace BBoard;
using namespace Framework;

typedef int Token;

#define SQ_FROM_BB_SQUARE(x) VirtualSquare(static_cast<VirtualFile>(square_file(x)), static_cast<VirtualRank>(square_rank(x)))
#define BB_SQUARE_FROM_VIRTUAL_SQ(x) make_square(static_cast<File>(x.file), static_cast<Rank>(x.rank))
#define BB_COLOR_TO_VIRTUAL_PIECE(x) x == WHITE ? WhitePlayer : BlackPlayer

struct ChessCoreImpl
{
    std::string mainFEN;
    
    // Reference to ChessCore
    ChessCore *core;

    // Play a move given it's token
    bool playToken(int, const VirtualPiece &, VirtualMove *);

    // Update the board to a specified position
    bool updatePosition(const std::string &fen);
        
    void attackTo(const VirtualSquare &,
                  const Player &,
                  std::vector<VirtualSquare>& );

    void attackTo(const VirtualSquare &,
                  const Player &,
                  const VirtualPiece  &,
                  std::vector<VirtualSquare>& );

    VirtualPiece getPiece(const VirtualSquare &) const;

    void analyzeAttacks(Color color,
                        Bitboard &attackKingBB,
                        Bitboard &attackQueenBB,
                        Bitboard &attackRookBB,
                        Bitboard &attackBishopBB,
                        Bitboard &attackKnightBB,
                        Bitboard &attackPawnBB);

    template<PieceType Piece>
    Bitboard pieceAttacks(Color color, const Square *squares)
    {
        Bitboard bitboard = 0;

        while (squares != NULL && (*squares) != SQ_NONE)
        {
            bitboard |= mainBoard.attacks_from_special<Piece>(*(squares++), color); 
        }

        return bitboard;
    }

    Bitboard pieceAttacksPawn(Color color, const Square *squares)
    {
        Bitboard bitboard = 0;
        Color tempColor = color == mainBoard.sideToMove ? color : mainBoard.sideToMove ;

        while (squares != NULL && (*squares) != SQ_NONE)
        {
            bitboard |= mainBoard.attacks_from<PAWN>(*(squares++), color) & mainBoard.pieces_of_color(tempColor); 
        }

        return bitboard;
    }

    StateInfo varState;
    StateInfo mainState;

    // Bitboard for the main game
    Position mainBoard;

    // Bitboard for the variations
    Position varBoard;    
};

static int generateTokenImpl(const Position &board, const VirtualSquare& src, const VirtualSquare& dst, const VirtualPiece &promote, VirtualMoveType &type)
{
    type = VALID_MOVE;
    Move move = make_move(BB_SQUARE_FROM_VIRTUAL_SQ(src), BB_SQUARE_FROM_VIRTUAL_SQ(dst));    

    if (!board.move_is_legal(move))
    {
        /*
         * The move could be illegal because it's not a regular move. Let's check it.
         */
        
        const auto srcPiece = static_cast<VirtualPiece>(board.piece_on(BB_SQUARE_FROM_VIRTUAL_SQ(src)));
        
        // Check for castling
        if (srcPiece == WhiteKing || srcPiece == BlackKing)
        {    
            // Check for king-side castling
            if ((src.file == 4 && src.rank == 0 && dst.file == 6 && dst.rank == 0) ||
                (src.file == 4 && src.rank == 7 && dst.file == 6 && dst.rank == 7))
            {
                type = SHORT_CASTLE_MOVE;
                move = make_castle_move(BB_SQUARE_FROM_VIRTUAL_SQ(src), make_square(static_cast<File>(7), static_cast<Rank>(dst.rank)));
            }
            
            // Check for queen-side castling
            else if ((src.file == 4 && src.rank == 0 && dst.file == 2 && dst.rank == 0) ||
                     (src.file == 4 && src.rank == 7 && dst.file == 2 && dst.rank == 7))
            {
                type = LONG_CASTLE_MOVE;
                move = make_castle_move(BB_SQUARE_FROM_VIRTUAL_SQ(src), make_square(static_cast<File>(0), static_cast<Rank>(dst.rank)));
            }
        }
        
        // Check for promotion
        else if ((src.rank == 6 && dst.rank == 7 && srcPiece == WhitePawn) || (src.rank == 1 && dst.rank == 0 && srcPiece == BlackPawn))
        {
            type = PROMOTION_MOVE;
            
            // Since promote is a const...
            VirtualPiece tempPromote = promote;
            
            /* ---------- Transform generic promotion to specified promotion ---------- */
            
            if ((tempPromote < WhiteKnight || tempPromote > WhiteQueen) && (tempPromote < BlackKnight || tempPromote > BlackQueen))
            {
                Color turn = board.side_to_move();
                
                if (tempPromote == AnyKnight)
                {
                    tempPromote = (turn == WHITE) ? WhiteKnight : BlackKnight;
                }
                else if (tempPromote == AnyRook)
                {
                    tempPromote = (turn == WHITE) ? WhiteRook : BlackRook;
                }
                else if (tempPromote == AnyBishop)
                {
                    tempPromote = (turn == WHITE) ? WhiteBishop : BlackBishop;
                }
                else
                {
                    tempPromote = (turn == WHITE) ? WhiteQueen : BlackQueen;                    
                }
            }
            
            if (tempPromote == WhiteQueen || tempPromote == BlackQueen)
            {
                ADD_MOVE_TYPE(type, PROMOTE_QUEEN_MOVE);
            }
            else if (tempPromote == WhiteKnight || tempPromote == BlackKnight)
            {
                ADD_MOVE_TYPE(type, PROMOTE_KNIGHT_MOVE);
            }
            else if (tempPromote == WhiteRook || tempPromote == BlackRook)
            {
                ADD_MOVE_TYPE(type, PROMOTE_ROOK_MOVE);
            }
            else if (tempPromote == WhiteBishop || tempPromote == BlackBishop)
            {
                ADD_MOVE_TYPE(type, PROMOTE_BISHOP_MOVE);
            }
            
            // Generate a token for the promotion
            move = make_promotion_move(BB_SQUARE_FROM_VIRTUAL_SQ(src), BB_SQUARE_FROM_VIRTUAL_SQ(dst), type_of_piece(static_cast<Piece>(tempPromote)));
        }
        
        // Check for enpassant
        else if ((src.rank == 4 && dst.rank == 5 && abs(static_cast<char>(dst.file - src.file)) == 1 && srcPiece == WhitePawn) ||
                 (src.rank == 3 && dst.rank == 2 && abs(static_cast<char>(dst.file - src.file)) == 1 && srcPiece == BlackPawn))
        {
            type = EN_PASSANT_MOVE;
            move = make_ep_move(BB_SQUARE_FROM_VIRTUAL_SQ(src), BB_SQUARE_FROM_VIRTUAL_SQ(dst));            
        }

        if (type == VALID_MOVE || !board.move_is_legal(move))
        {
            // We've tried again for the special moves, this move must be illegal
            type = INVALID_MOVE;
            
            return MOVE_NULL;
        }
    }
    
    if ((src.file == 1 && src.rank == 6) || (src.file == 1 && src.rank == 7))
    {
        type = type;
    }
    
    if (board.move_gives_check(move))
    {
        ADD_MOVE_TYPE(type, CHECK_MOVE);        
    }
    
    if (board.move_is_capture(move))  { ADD_MOVE_TYPE(type, CAPTURE_MOVE); }
    
    return move;    
}

Validation ChessCore::validate(const std::string &fen)
{
    BBoard::Position  position;
    Validation validation;
    
    switch (position.from_fen(fen, &validation))
    {
        case WhiteNoKing:          { validation.error = "White has no king";                  break; }
        case BlackNoKing:          { validation.error = "Black has no king";                  break; }
        case WhiteTooManyKing:     { validation.error = "White has too many kings";           break; }
        case BlackTooManyKing:     { validation.error = "Black has too many kings";           break; }        
        case PawnFirstRank:        { validation.error = "Pawn on the first rank";             break; }
        case PawnLastRank:         { validation.error = "Pawn on the last rank";              break; }
        case InvalidFEN:           { validation.error = "Please check and try again";         break; }
        case WhiteTooManyPawn:     { validation.error = "White has too many pawns";           break; }
        case BlackTooManyPawn:     { validation.error = "Black has too many pawns";           break; }
        case WhiteTooManyMajor:    { validation.error = "White has too many pieces";          break; }
        case BlackTooManyMajor:    { validation.error = "Black has too many pieces";          break; }
        case BothKingInCheck:      { validation.error = "Both kings are in check";            break; }
        case TooManyPawnChecked:   { validation.error = "Too many pawns checking the king";   break; }
        case TooManyKnightChecked: { validation.error = "Too many knights checking the king"; break; }
        case TooManyBishopChecked: { validation.error = "Too many bishops checking the king"; break; }
        case TooManyRookChecked:   { validation.error = "Too many rooks checking the king";   break; }
        case QueenAndBishopCheck:  { validation.error = "Queen and bishop both checking";     break; }
        default:                   { break; }
    }

    validation.isValid = validation.error.empty();
    return validation;
}

VirtualPiece ChessCore::getPiece(const VirtualSquare &square) const
{
    return static_cast<VirtualPiece>(_pimpl->mainBoard.piece_on(BB_SQUARE_FROM_VIRTUAL_SQ(square)));
}

bool ChessCoreImpl::updatePosition(const std::string &fen)
{
    const bool succeed = mainBoard.from_fen(fen) == VALIDATE_FEN_OK;
    
    if (succeed)
    {
        mainFEN = fen;
    }

    return succeed;
}

bool ChessCoreImpl::playToken(int token, const VirtualPiece &promote, VirtualMove *move)
{
    // Update internal game board
    mainBoard.do_move(static_cast<Move>(token));
    
    /*
     * This is a hack, but with good reasons.
     *
     * The notation is generated before the function is called. However, before it's called it wouldn't
     * be possible to check for checkmated.
     */

    if (HAS_MOVE_TYPE_OF(move->type, CHECK_MOVE) && mainBoard.is_mate())
    {
        ADD_MOVE_TYPE(move->type, CHECKMATE_MOVE);

        // Replace the check symbol with the checkmate symbol
        move->notation[move->notation.size() - 1] = '#';
    }

    // Update history and the latest position
    //core->getHistory()->addNode(*move, mainFEN = mainBoard.to_fen());

    // Send the updated FEN position to the protocol layer
    if (core->getProtocol())
    {
		core->getProtocol()->update(move->src.file, move->src.rank, move->dst.file, move->dst.rank, BoardConverter::pieceToChar(promote));
    }

    return true;
}

std::string ChessCore::generateNotation(const VirtualMovement &move, const VirtualPiece& srcPiece, const VirtualMoveType &type) const
{
    /*
     * Don't bother checking for other attackers if we know that there wouldn't be any ambigious.
     */
/*
	if (HAS_MOVE_TYPE_OF(type, ADDED_MOVE) || srcPiece == WhitePawn || srcPiece == BlackPawn || srcPiece == WhiteKing || srcPiece == BlackKing)
    {
        return ChessNotation::toNotation(move, srcPiece, type);
    }
    else
    {
        std::vector<VirtualSquare> attackers;

        // Search for attackers of type srcPiece attacking dst
        _pimpl->attackTo(move.dst, _history->getTurn(), srcPiece, attackers);

        // Generate notation taking care of other attackers
        return attackers.size() <= 1 ? ChessNotation::toNotation(move, srcPiece, type) : ChessNotation::toNotation(move, srcPiece, type, attackers);
    }
 */
    
    return "";
}

VirtualSquare ChessCore::find(VirtualPiece piece)
{
    assert(piece == WhiteKing || piece == BlackKing);    
    const Square sq = _pimpl->mainBoard.king_square(piece == WhiteKing ? WHITE : BLACK);    
    return SQ_FROM_BB_SQUARE(sq);
}

std::string ChessCore::convertVarToStr(std::size_t index, std::vector<std::string> moves, bool isWhiteTurn)
{
    std::stringstream ss;
    
    index = (index / 2) - 1;

    for (auto i = 0; i < moves.size(); i++)
    {
        if (i == 0 || isWhiteTurn)
        {
            ss << index++ << "." << moves[i];
        }
        else
        {
            ss << " " << moves[i];
        }        
        
        isWhiteTurn = !isWhiteTurn;
    }
    
    return ss.str();
}

/* -------------------- ChessCore -------------------- */

ChessCore::ChessCore(Protocol *protocol) : VirtualBoard(protocol), _pimpl(new ChessCoreImpl())
{
    init(getProtocol());
}

void ChessCore::init(Protocol *protocol)
{
    _pimpl->core  = this;
}

ChessCore::~ChessCore()
{
    if (_pimpl)
    {
        delete _pimpl;
    }
}

/* -------------------- Core -------------------- */

Status ChessCore::getStatus() const
{
    if (_pimpl->mainBoard.is_mate())
    {
        return StatusCheckmated;
    }

    switch (_pimpl->mainBoard.is_draw())
    {
        case 1: { return StatusInsufficient; }
        case 2: { return StatusFiftyMove;    }

        default:
        {
            /*
            std::vector<VirtualNode *> nodes;

            if (_history->getNodes(nodes) && _history->isEnd())
            {
                const auto &state = _history->getState();

                unsigned j = 0;                
                const auto n = nodes.size();

                for (auto i = n; i && (n - i) <= 12; i--)
                {
                    if (nodes[i - 1]->state == state)
                    {
                        j++;
                    }
                }
            
                if (state == _history->getInitialState())
                {
                    j++;
                }

                if (j >= 3)
                {
                    return StatusRepetition;
                }
            }
            */

            checkStalemate:

            /*
             * Check for stalemate
             */

            std::vector<VirtualMove> moves;
            this->getMoves(moves, false);

            return moves.size() ? StatusNil : StatusStalemate;
        }
    }
}

bool ChessCore::canPlay(const VirtualSquare& src, const VirtualSquare& dst, const VirtualPiece &promote, VirtualMove * move) const
{
    const auto token = generateToken(src, dst, promote, move);
    return (token != MOVE_NULL) && (token != MOVE_NONE);
}

void ChessCore::open()
{
    static bool initalized = false;

    if (!initalized)
    {
		initalized = true;
        init_bitboards();
        Position::init_zobrist();
        Position::init_piece_square_tables();
    }
}

void ChessCore::close()
{
    _protocol = NULL;
}

void ChessCore::start()
{
    start(INITIAL_CHESS_FEN);
}

void ChessCore::start(const std::string& state)
{
    // Update internal representation
    if (_pimpl->mainBoard.from_fen(state, NULL) != VALIDATE_FEN_OK)
    {
#ifdef RESTRY_ON_INVALID_STATE
        /*
         * We want to be resistence, don't crash in production. Simply ignore it with the default positon.
         */
        
        // Try again with the default positon
        start();
        
        return;
#else
        throw std::runtime_error("Invalid position: " + state);
#endif
    }

    if (_protocol)
    {
        _protocol->start(state);
    }

    _pimpl->mainFEN = _pimpl->mainBoard.to_fen();
}

void * ChessCore::getInternal() const
{
    return &_pimpl->mainBoard;
}

void ChessCore::captured(unsigned &wq,
                         unsigned &wr,
                         unsigned &wb,
                         unsigned &wn,
                         unsigned &wp,
                         unsigned &bq,
                         unsigned &br,
                         unsigned &bb,
                         unsigned &bn,
                         unsigned &bp) const
{
    wq = bq = 1;
    wp = bp = 8;
    wr = br = wb = bb = wn = bn = 2;
    
    for (auto i = 0; i < 64; i++)
    {
        switch (_pimpl->mainBoard.board[i])
        {
            case WP: { wp--; break; }
            case WN: { wn--; break; }
            case WB: { wb--; break; }
            case WR: { wr--; break; }
            case WQ: { wq--; break; }
            case BP: { bp--; break; }
            case BN: { bn--; break; }
            case BB: { bb--; break; }
            case BR: { br--; break; }
            case BQ: { bq--; break; }
            default: { break; }
        }
    }
}

void ChessCore::getMoves(std::vector<VirtualMove>& moves, bool extended) const
{
	getMoves(VirtualSquare::getIllegal(), moves, extended);
}

unsigned ChessCore::countPieces() const
{
    unsigned n = 0;

    for (auto i = 0; i < 64; i++)
    {
        if (_pimpl->mainBoard.board[i] != PIECE_NONE_DARK_SQ && _pimpl->mainBoard.board[i] != PIECE_NONE)
        {
            n++;
        }
    }

    return n;
}

// Generate all legal move from a specified source for the current position
void ChessCore::getMoves(const VirtualSquare &from, std::vector<VirtualMove> &moves, bool extended) const
{
    moves.clear();
    MoveStack mlist[MAX_MOVES];
    MoveStack *mlistEnd = generate<MV_LEGAL>(_pimpl->mainBoard, mlist);
    
    auto fen = _pimpl->mainFEN;
    auto turn = std::find(fen.begin(), fen.end(), 'w') != fen.end() ? WhitePlayer : BlackPlayer;
    
    while (mlistEnd-- != mlist)
    {
        const auto move = mlistEnd->move;

        if (move != MOVE_NONE && move != MOVE_NULL)
        {
            VirtualSquare src(static_cast<VirtualFile>(square_file(move_from(move))), static_cast<VirtualRank>(square_rank(move_from(move))));

            if (from != VirtualSquare::getIllegal() && from != src)
            {
                continue;
            }

            VirtualSquare dst(static_cast<VirtualFile>(square_file(move_to(move))), static_cast<VirtualRank>(square_rank(move_to(move))));

            const auto srcPiece = getPiece(src);
            const auto dstPiece = getPiece(dst);
            
            /*
             * Stockfish Bitboard represents O-O as e1 -> (7,0). We need to convert it.
             */
            if ((srcPiece == WhiteKing && src.file == 4 && src.rank == 0 && dst.file == 7 && dst.rank == 0) ||
                (srcPiece == BlackKing && src.file == 4 && src.rank == 7 && dst.file == 7 && dst.rank == 7))
            {
                dst.file = _FileG;
            }
            else if ((srcPiece == WhiteKing && src.file == 4 && src.rank == 0 && dst.file == 0 && dst.rank == 0) ||
                     (srcPiece == BlackKing && src.file == 4 && src.rank == 7 && dst.file == 0 && dst.rank == 7))
            {
                dst.file = _FileC;
            }

            /*
             * Since we're only interested the legal moves, we don't need to generate notation and the type of the move.
             */
            
            VirtualMoveType type = VALID_MOVE;
            
            if (extended)
            {
                generateTokenImpl(_pimpl->mainBoard, src, dst, NilPiece, type);
            }

            moves.push_back(VirtualMove("", src, dst, srcPiece, dstPiece, turn, type));                            
        }
    }
}

int ChessCore::generateToken(const VirtualSquare& src, const VirtualSquare& dst, const VirtualPiece &promote, VirtualMove *move) const
{
    // The piece to which the move is played
    const auto srcPiece = getPiece(src);

    // The piece to which the capture is made (if any)
    const auto dstPiece = getPiece(dst);

    // The current turn of the position
    const auto turn = WhitePlayer;

    VirtualMoveType type;
    
    // Generate token for the move and it's type
    const int token = generateTokenImpl(_pimpl->mainBoard, src, dst, promote, type);

    if (token != MOVE_NONE && token != MOVE_NULL)
    {
        if (move)
        {
            move->notation = this->generateNotation(VirtualMovement(src, dst), srcPiece, type);
            move->src = src;
            move->dst = dst;
            move->turn = turn;
            move->type = type;
            move->srcPiece = srcPiece;
            move->dstPiece = dstPiece;
        }
    }
    else
    {
        if (move)
        {
            move->type = INVALID_MOVE;
        }
    }
    
    return token;
}

void ChessCore::update(const std::string& state)
{
    _pimpl->mainBoard.from_fen(state);

    if (_protocol)
    {
        _protocol->update(state);
    }
}

bool ChessCore::add(const VirtualSquare &square, const VirtualPiece &piece)
{
    /*
	const auto turn = getHistory()->getTurn();

	if ((turn == WhitePlayer && piece >= BlackPawn && BlackPawn <= BlackKing) ||
		(turn == BlackPlayer && piece >= WhitePawn && BlackPawn <= WhiteKing) )
	{
		return false;
	}

	BBoard::Position temp;

	temp.from_fen(_pimpl->mainBoard.to_fen());
	temp.put_piece(static_cast<BBoard::Piece>(piece), BB_SQUARE_FROM_VIRTUAL_SQ(square));

	if (temp.is_ok() && ChessCore::validate(temp.to_fen()).isValid)
	{
		_pimpl->mainBoard.put_piece(static_cast<BBoard::Piece>(piece), BB_SQUARE_FROM_VIRTUAL_SQ(square));

		VirtualMove move;

		move.src = VirtualSquare::getIllegal();
		move.dst = square;
		move.turn = BB_COLOR_TO_VIRTUAL_PIECE(_pimpl->mainBoard.side_to_move());
		move.type = ADDED_MOVE;
		move.srcPiece = piece;
		move.dstPiece = piece;
		move.notation = generateNotation(VirtualMovement(move.src, move.dst), move.srcPiece, ADDED_MOVE);

		getHistory()->addNode(move, _pimpl->mainFEN = _pimpl->mainBoard.to_fen());
		return true;
	}
*/
	return false;
}

bool ChessCore::play(const VirtualSquare& src, const VirtualSquare& dst, const VirtualPiece &promote, VirtualMove *move)
{
    // We need VirtualMove for token generation, we create an internal instance if not provided
    auto internalMove = move ? move : new VirtualMove();

    const auto token = generateToken(src, dst, promote, internalMove);

    if (token == MOVE_NULL || token == MOVE_NONE)
    {
        return false;
    }
    else if (_receiver && !_receiver->notifyValid(*move))
    {
        return true;
    }

	const auto status = _pimpl->playToken(token, promote, internalMove);
    
    if (internalMove != move)
    {
        delete internalMove;
    }
    
    return status;
}
