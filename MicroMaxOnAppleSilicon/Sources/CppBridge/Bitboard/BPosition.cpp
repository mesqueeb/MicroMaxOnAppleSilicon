#include <map>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

//#include "Types.hpp"
#include "BRkiss.h"
#include "BPsqtab.h"
#include "BMoveGen.h"
#include "BPosition.h"
#include "BBitcount.h"

namespace BBoard
{

using std::string;
using std::cout;
using std::endl;

Key Position::zobrist[2][8][64];
Key Position::zobEp[64];
Key Position::zobCastle[16];
Key Position::zobSideToMove;
Key Position::zobExclusion;

BScore Position::PieceSquareTable[16][64];

// Material values arrays, indexed by Piece
const Value Position::PieceValueMidgame[17] = {
  VALUE_ZERO,
  PawnValueMidgame, KnightValueMidgame, BishopValueMidgame,
  RookValueMidgame, QueenValueMidgame, VALUE_ZERO,
  VALUE_ZERO, VALUE_ZERO,
  PawnValueMidgame, KnightValueMidgame, BishopValueMidgame,
  RookValueMidgame, QueenValueMidgame
};

const Value Position::PieceValueEndgame[17] = {
  VALUE_ZERO,
  PawnValueEndgame, KnightValueEndgame, BishopValueEndgame,
  RookValueEndgame, QueenValueEndgame, VALUE_ZERO,
  VALUE_ZERO, VALUE_ZERO,
  PawnValueEndgame, KnightValueEndgame, BishopValueEndgame,
  RookValueEndgame, QueenValueEndgame
};

// Material values array used by SEE, indexed by PieceType
const Value Position::seeValues[] = {
    VALUE_ZERO,
    PawnValueMidgame, KnightValueMidgame, BishopValueMidgame,
    RookValueMidgame, QueenValueMidgame, QueenValueMidgame*10
};

namespace {

  // Bonus for having the side to move (modified by Joona Kiiski)
  const BScore TempoValue = make_score(48, 22);

  struct PieceLetters : public std::map<char, Piece> {

    PieceLetters() {

      operator[]('K') = WK; operator[]('k') = BK;
      operator[]('Q') = WQ; operator[]('q') = BQ;
      operator[]('R') = WR; operator[]('r') = BR;
      operator[]('B') = WB; operator[]('b') = BB;
      operator[]('N') = WN; operator[]('n') = BN;
      operator[]('P') = WP; operator[]('p') = BP;
      operator[](' ') = PIECE_NONE;
      operator[]('.') = PIECE_NONE_DARK_SQ;
    }

    char from_piece(Piece p) const {

      std::map<char, Piece>::const_iterator it;
      for (it = begin(); it != end(); ++it)
          if (it->second == p)
              return it->first;

      assert(false);
      return 0;
    }
  };

  PieceLetters pieceLetters;
}


/// CheckInfo c'tor

CheckInfo::CheckInfo(const Position& pos) {

  Color us = pos.side_to_move();
  Color them = opposite_color(us);

  ksq = pos.king_square(them);
  dcCandidates = pos.discovered_check_candidates(us);

  checkSq[PAWN] = pos.attacks_from<PAWN>(ksq, them);
  checkSq[KNIGHT] = pos.attacks_from<KNIGHT>(ksq);
  checkSq[BISHOP] = pos.attacks_from<BISHOP>(ksq);
  checkSq[ROOK] = pos.attacks_from<ROOK>(ksq);
  checkSq[QUEEN] = checkSq[BISHOP] | checkSq[ROOK];
  checkSq[KING] = EmptyBoardBB;
}


/// Position c'tors. Here we always create a copy of the original position
/// or the FEN string, we want the new born Position object do not depend
/// on any external data so we detach state pointer from the source one.

Position::Position(const string& fen)
{
  _stateInfo = new StateInfo();
  from_fen(fen);
}

Bitboard Position::attackers_to(Square s, Color c) const
{
    return attackers_to(s) & pieces_of_color(c);
}

ValidateStatus Position::checkKingInCheck(Bitboard kingAttacked)
{
	if (kingAttacked)
    {
		char attackers[15];
        
        for (unsigned char i = 0; i < 15; i++)
        {
            attackers[i] = 0;
        }

		// Count all the attackers for the king
		for (BBoard::Square sq = SQ_A1; sq < SQ_H8; sq++)
		{
			if (bit_is_set(kingAttacked, sq))
			{
				attackers[piece_on(sq)]++;
			}
		}

        if (attackers[WK] >= 1 || attackers[BK] >= 1)
        {
            return BothKingInCheck;
        }
		else if (attackers[WP] >= 2 || attackers[BP] >= 2)
		{
			return TooManyPawnChecked;
		}
		else if (attackers[WN] >= 2 || attackers[BN] >= 2)
		{
			return TooManyKnightChecked;
		}
		else if (attackers[WB] >= 2 || attackers[BB] >= 2)
		{
			return TooManyBishopChecked;
		}
		else if (attackers[WR] >= 2 || attackers[BR] >= 2)
		{
			return TooManyRookChecked;
		}
        else if ((attackers[WQ] && attackers[WB]) || (attackers[BQ] && attackers[BB]))
        {
            return QueenAndBishopCheck;
        }
	}

	return VALIDATE_FEN_OK;
}

/// Position::from_fen() initializes the position object with the given FEN
/// string. This function is not very robust - make sure that input FENs are
/// correct (this is assumed to be the responsibility of the GUI).

#define DEFINE_INVALID_REASON(x, y) if (x) *x = y

ValidateStatus Position::from_fen(const std::string& fen, Validation *validation)
{
    char token;
    int hmc, fmn;
    std::istringstream ss(fen);
    Square sq = SQ_A8;

    clear();

    unsigned char totalWK = 0;
    unsigned char totalBK = 0;
    unsigned char totalWQ = 0;
    unsigned char totalBQ = 0;
    unsigned char totalWR = 0;
    unsigned char totalBR = 0;
    unsigned char totalWB = 0;
    unsigned char totalBB = 0;
    unsigned char totalWN = 0;
    unsigned char totalBN = 0;
    unsigned char totalWP = 0;
    unsigned char totalBP = 0;
        
    // 1. Piece placement field
    while (ss.get(token) && token != ' ')
    {
        if (pieceLetters.find(token) != pieceLetters.end())
        {
            const Piece p = pieceLetters[token];
          
            switch (p)
            {
                case WK: { totalWK++; break; }
                case BK: { totalBK++; break; }
                case WQ: { totalWQ++; break; }
                case BQ: { totalBQ++; break; }
                case WR: { totalWR++; break; }
                case BR: { totalBR++; break; }
                case WB: { totalWB++; break; }
                case BB: { totalBB++; break; }
                case WN: { totalWN++; break; }
                case BN: { totalBN++; break; }
                case WP: { totalWP++; break; }
                case BP: { totalBP++; break; }
                default: { break; }
            }
            
            if ((p == WP || p == BP) && square_rank(sq) == 0)
            {
                return PawnFirstRank;
            }
            else if ((p == WP || p == BP) && square_rank(sq) == 7)
            {
                return PawnLastRank;
            }

            put_piece(pieceLetters[token], sq);
            sq++;
        }
        else if (isdigit(token))
        {
            sq += Square(token - '0'); // Skip the given number of files
        }
        else if (token == '/')
        {
            sq -= SQ_A3; // Jump back of 2 rows
        }
        else
        {
            return InvalidFEN;
        }
    }

    const unsigned char totalWM = totalWN + totalWB + totalWR + totalWQ;
    const unsigned char totalBM = totalBN + totalBB + totalBR + totalBQ;

         if (totalWM + totalWP > 15) { return WhiteTooManyMajor; }
    else if (totalBM + totalBP > 15) { return BlackTooManyMajor; }
    else if (totalWK == 0)           { return WhiteNoKing; }
    else if (totalBK == 0)           { return BlackNoKing; }
    else if (totalWK > 1)            { return WhiteTooManyKing; }
    else if (totalBK > 1)            { return BlackTooManyKing; }
    else if (totalWP > 8)            { return WhiteTooManyPawn; }
    else if (totalBP > 8)            { return BlackTooManyPawn; }
    else if ((totalWN + totalWP > 10) || (totalWB + totalWP > 10) ||
             (totalWR + totalWP > 10) || (totalWQ + totalWP > 10))
    {
        return WhiteTooManyMajor;
    }    
    else if ((totalBN + totalBP > 10) || (totalBB + totalBP > 10) ||
             (totalBR + totalBP > 10) || (totalBQ + totalBP > 10))
    {
        return BlackTooManyMajor;
    }

    /* -------------------- Validation -------------------- */

    Bitboard whiteKingAttacked = attackers_to(king_square(WHITE), BLACK);
    Bitboard blackKingAttacked = attackers_to(king_square(BLACK), WHITE);

	// Check the validity of white's king
    ValidateStatus whiteKingStatus = checkKingInCheck(whiteKingAttacked);

	// Check the validity of black's king
    ValidateStatus blackKingStatus = checkKingInCheck(blackKingAttacked);

    if (whiteKingStatus != VALIDATE_FEN_OK)
    {
        return whiteKingStatus;
    }
    else if (blackKingStatus != VALIDATE_FEN_OK)
    {
        return blackKingStatus;
    }
	else if (whiteKingAttacked && blackKingAttacked)
	{
		return BothKingInCheck;
	}

    if (validation)
    {
        // Check if white's king is under checked
        if (whiteKingAttacked)
        {
            validation->canBlackMove = false;
        }

        // Check if black's king is under checked
        if (blackKingAttacked)
        {
            validation->canWhiteMove = false;
        }
        
        // Check whether white's short castle is possible
        if (piece_on(SQ_E1) != WK || piece_on(SQ_H1) != WR)
        {
            validation->canWKCastle = false;
        }

        // Check whether black's short castle is possible
        if (piece_on(SQ_E8) != BK || piece_on(SQ_H8) != BR)
        {
            validation->canBKCastle = false;
        }

        // Check whether white's long castle is possible
        if (piece_on(SQ_E1) != WK || piece_on(SQ_A1) != WR)
        {
            validation->canWQCastle = false;
        }

        // Check whether black's long castle is possible
        if (piece_on(SQ_E8) != BK || piece_on(SQ_A8) != BR)
        {
            validation->canBQCastle = false;
        }

        return VALIDATE_FEN_OK;
    }
    
    // 2. Active color
    if (!ss.get(token) || (token != 'w' && token != 'b'))
    {
        return InvalidFEN;
    }

	sideToMove = (token == 'w' ? WHITE : BLACK);

	if ((sideToMove == WHITE && blackKingAttacked) || (sideToMove == BLACK && whiteKingAttacked))
	{
		return InvalidFEN;
	}

	if (!ss.get(token) || token != ' ')
	{
		return InvalidFEN;
	}

	// 3. Castling availability
	while (ss.get(token) && token != ' ')
	{
		if (!set_castling_rights(token))
		{
			return InvalidFEN;
		}
	}

	// 4. En passant square
	char col, row;
	if (   (ss.get(col) && (col >= 'a' && col <= 'h'))
		&& (ss.get(row) && (row == '3' || row == '6')))
	{
		_stateInfo->epSquare = make_square(file_from_char(col), rank_from_char(row));

		// Ignore if no capture is possible
		Color them = opposite_color(sideToMove);
		if (!(attacks_from<PAWN>(_stateInfo->epSquare, them) & pieces(PAWN, sideToMove)))
			_stateInfo->epSquare = SQ_NONE;
	}

	// 5. Halfmove clock
	if (ss >> hmc)
		_stateInfo->rule50 = hmc;

	// 6. Fullmove number
	if (ss >> fmn)
	{
		//startPosPlyCounter = (fmn - 1) * 2 + int(sideToMove == BLACK);
	}

	// Various initialisations
	castleRightsMask[make_square(_initialKFile,  RANK_1)] ^= WHITE_OO | WHITE_OOO;
	castleRightsMask[make_square(_initialKFile,  RANK_8)] ^= BLACK_OO | BLACK_OOO;
	castleRightsMask[make_square(_initialKRFile, RANK_1)] ^= WHITE_OO;
	castleRightsMask[make_square(_initialKRFile, RANK_8)] ^= BLACK_OO;
	castleRightsMask[make_square(_initialQRFile, RANK_1)] ^= WHITE_OOO;
	castleRightsMask[make_square(_initialQRFile, RANK_8)] ^= BLACK_OOO;

	find_checkers();

	_stateInfo->key = compute_key();
	_stateInfo->pawnKey = compute_pawn_key();
	_stateInfo->materialKey = compute_material_key();
	_stateInfo->value = compute_value();
	_stateInfo->npMaterial[WHITE] = compute_non_pawn_material(WHITE);
	_stateInfo->npMaterial[BLACK] = compute_non_pawn_material(BLACK);

	return VALIDATE_FEN_OK;
}


/// Position::set_castling_rights() sets castling parameters castling avaiability.
/// This function is compatible with 3 standards: Normal FEN standard, Shredder-FEN
/// that uses the letters of the columns on which the rooks began the game instead
/// of KQkq and also X-FEN standard that, in case of Chess960, if an inner Rook is
/// associated with the castling right, the traditional castling tag will be replaced
/// by the file letter of the involved rook as for the Shredder-FEN.

bool Position::set_castling_rights(char token) {

    Color c = token >= 'a' ? BLACK : WHITE;
    Square sqA = (c == WHITE ? SQ_A1 : SQ_A8);
    Square sqH = (c == WHITE ? SQ_H1 : SQ_H8);
    Piece rook = (c == WHITE ? WR : BR);

    _initialKFile = square_file(king_square(c));
    token = char(toupper(token));

    if (token == 'K')
    {
        for (Square sq = sqH; sq >= sqA; sq--)
            if (piece_on(sq) == rook)
            {
                do_allow_oo(c);
                _initialKRFile = square_file(sq);
                break;
            }
    }
    else if (token == 'Q')
    {
        for (Square sq = sqA; sq <= sqH; sq++)
            if (piece_on(sq) == rook)
            {
                do_allow_ooo(c);
                _initialQRFile = square_file(sq);
                break;
            }
    }
    else if (token >= 'A' && token <= 'H')
    {
        File rookFile = File(token - 'A') + FILE_A;
        if (rookFile < _initialKFile)
        {
            do_allow_ooo(c);
            _initialQRFile = rookFile;
        }
        else
        {
            do_allow_oo(c);
            _initialKRFile = rookFile;
        }
    }
    else
        return token == '-';

  return true;
}


/// Position::to_fen() returns a FEN representation of the position. In case
/// of Chess960 the Shredder-FEN notation is used. Mainly a debugging function.

const string Position::to_fen() const
{
  string fen;
  Square sq;
  char emptyCnt = '0';

  for (Rank rank = RANK_8; rank >= RANK_1; rank--)
  {
      for (File file = FILE_A; file <= FILE_H; file++)
      {
          sq = make_square(file, rank);

          if (square_is_occupied(sq))
          {
              if (emptyCnt != '0')
              {
                  fen += emptyCnt;
                  emptyCnt = '0';
              }
              
              fen += pieceLetters.from_piece(piece_on(sq));
          } else
              emptyCnt++;
      }

      if (emptyCnt != '0')
      {
          fen += emptyCnt;
          emptyCnt = '0';
      }
      
      if (rank != RANK_1)
      {
          fen += '/';
      }
  }

  fen += (sideToMove == WHITE ? " w " : " b ");

  if (_stateInfo->castleRights != CASTLES_NONE)
  {
      if (can_castle_kingside(WHITE))
          fen += 'K';

      if (can_castle_queenside(WHITE))
          fen += 'Q';

      if (can_castle_kingside(BLACK))
          fen += 'k';

      if (can_castle_queenside(BLACK))
          fen += 'q';
  } else
      fen += '-';

  fen += (ep_square() == SQ_NONE ? " -" : " " + square_to_string(ep_square()));
  return fen;
}


/// Position:hidden_checkers<>() returns a bitboard of all pinned (against the
/// king) pieces for the given color and for the given pinner type. Or, when
/// template parameter FindPinned is false, the pieces of the given color
/// candidate for a discovery check against the enemy king.
/// Bitboard checkersBB must be already updated when looking for pinners.

template<bool FindPinned>
Bitboard Position::hidden_checkers(Color c) const {

  Bitboard result = EmptyBoardBB;
  Bitboard pinners = pieces_of_color(FindPinned ? opposite_color(c) : c);

  // Pinned pieces protect our king, dicovery checks attack
  // the enemy king.
  Square ksq = king_square(FindPinned ? c : opposite_color(c));

  // Pinners are sliders, not checkers, that give check when candidate pinned is removed
  pinners &= (pieces(ROOK, QUEEN) & RookPseudoAttacks[ksq]) | (pieces(BISHOP, QUEEN) & BishopPseudoAttacks[ksq]);

  if (FindPinned && pinners)
      pinners &= ~_stateInfo->checkersBB;

  while (pinners)
  {
      Square s = pop_1st_bit(&pinners);
      Bitboard b = squares_between(s, ksq) & occupied_squares();

      assert(b);

      if (  !(b & (b - 1)) // Only one bit set?
          && (b & pieces_of_color(c))) // Is an our piece?
          result |= b;
  }
  return result;
}


/// Position:pinned_pieces() returns a bitboard of all pinned (against the
/// king) pieces for the given color. Note that checkersBB bitboard must
/// be already updated.

Bitboard Position::pinned_pieces(Color c) const {

  return hidden_checkers<true>(c);
}


/// Position:discovered_check_candidates() returns a bitboard containing all
/// pieces for the given side which are candidates for giving a discovered
/// check. Contrary to pinned_pieces() here there is no need of checkersBB
/// to be already updated.

Bitboard Position::discovered_check_candidates(Color c) const {

  return hidden_checkers<false>(c);
}

/// Position::attackers_to() computes a bitboard containing all pieces which
/// attacks a given square.

Bitboard Position::attackers_to(Square s) const {

  return  (attacks_from<PAWN>(s, BLACK) & pieces(PAWN, WHITE))
        | (attacks_from<PAWN>(s, WHITE) & pieces(PAWN, BLACK))
        | (attacks_from<KNIGHT>(s)      & pieces(KNIGHT))
        | (attacks_from<ROOK>(s)        & pieces(ROOK, QUEEN))
        | (attacks_from<BISHOP>(s)      & pieces(BISHOP, QUEEN))
        | (attacks_from<KING>(s)        & pieces(KING));
}

/// Position::attacks_from() computes a bitboard of all attacks
/// of a given piece put in a given square.

Bitboard Position::attacks_from(Piece p, Square s) const {

  assert(square_is_ok(s));

  switch (p)
  {
  case WB: case BB: return attacks_from<BISHOP>(s);
  case WR: case BR: return attacks_from<ROOK>(s);
  case WQ: case BQ: return attacks_from<QUEEN>(s);
  default: return StepAttacksBB[p][s];
  }
}

Bitboard Position::attacks_from(Piece p, Square s, Bitboard occ) {

  assert(square_is_ok(s));

  switch (p)
  {
  case WB: case BB: return bishop_attacks_bb(s, occ);
  case WR: case BR: return rook_attacks_bb(s, occ);
  case WQ: case BQ: return bishop_attacks_bb(s, occ) | rook_attacks_bb(s, occ);
  default: return StepAttacksBB[p][s];
  }
}


/// Position::move_attacks_square() tests whether a move from the current
/// position attacks a given square.

bool Position::move_attacks_square(Move m, Square s) const {

  assert(move_is_ok(m));
  assert(square_is_ok(s));

  Bitboard occ, xray;
  Square f = move_from(m), t = move_to(m);

  assert(square_is_occupied(f));

  if (bit_is_set(attacks_from(piece_on(f), t), s))
      return true;

  // Move the piece and scan for X-ray attacks behind it
  occ = occupied_squares();
  do_move_bb(&occ, make_move_bb(f, t));
  xray = ( (rook_attacks_bb(s, occ)   & pieces(ROOK, QUEEN))
          |(bishop_attacks_bb(s, occ) & pieces(BISHOP, QUEEN)))
         & pieces_of_color(color_of_piece_on(f));

  // If we have attacks we need to verify that are caused by our move
  // and are not already existent ones.
  return xray && (xray ^ (xray & attacks_from<QUEEN>(s)));
}


/// Position::find_checkers() computes the checkersBB bitboard, which
/// contains a nonzero bit for each checking piece (0, 1 or 2). It
/// currently works by calling Position::attackers_to, which is probably
/// inefficient. Consider rewriting this function to use the last move
/// played, like in non-bitboard versions of Glaurung.

void Position::find_checkers() {

  Color us = side_to_move();
  _stateInfo->checkersBB = attackers_to(king_square(us)) & pieces_of_color(opposite_color(us));
}


/// Position::pl_move_is_legal() tests whether a pseudo-legal move is legal

bool Position::pl_move_is_legal(Move m, Bitboard pinned) const {

  assert(is_ok());
  assert(move_is_ok(m));
  assert(pinned == pinned_pieces(side_to_move()));

  // Castling moves are checked for legality during move generation.
  if (move_is_castle(m))
      return true;

  // En passant captures are a tricky special case. Because they are
  // rather uncommon, we do it simply by testing whether the king is attacked
  // after the move is made
  if (move_is_ep(m))
  {
      Color us = side_to_move();
      Color them = opposite_color(us);
      Square from = move_from(m);
      Square to = move_to(m);
      Square capsq = make_square(square_file(to), square_rank(from));
      Square ksq = king_square(us);
      Bitboard b = occupied_squares();

      assert(to == ep_square());
      assert(piece_on(from) == make_piece(us, PAWN));
      assert(piece_on(capsq) == make_piece(them, PAWN));
      assert(piece_on(to) == PIECE_NONE);

      clear_bit(&b, from);
      clear_bit(&b, capsq);
      set_bit(&b, to);

      return   !(rook_attacks_bb(ksq, b) & pieces(ROOK, QUEEN, them))
            && !(bishop_attacks_bb(ksq, b) & pieces(BISHOP, QUEEN, them));
  }

  Color us = side_to_move();
  Square from = move_from(m);

  assert(color_of_piece_on(from) == us);
  assert(piece_on(king_square(us)) == make_piece(us, KING));

  // If the moving piece is a king, check whether the destination
  // square is attacked by the opponent.
  if (type_of_piece_on(from) == KING)
      return !(attackers_to(move_to(m)) & pieces_of_color(opposite_color(us)));

  // A non-king move is legal if and only if it is not pinned or it
  // is moving along the ray towards or away from the king.
  return   !pinned
        || !bit_is_set(pinned, from)
        ||  squares_aligned(from, move_to(m), king_square(us));
}


/// Position::pl_move_is_evasion() tests whether a pseudo-legal move is a legal evasion

bool Position::pl_move_is_evasion(Move m, Bitboard pinned) const
{
  assert(in_check());

  Color us = side_to_move();
  Square from = move_from(m);
  Square to = move_to(m);

  // King moves and en-passant captures are verified in pl_move_is_legal()
  if (type_of_piece_on(from) == KING || move_is_ep(m))
      return pl_move_is_legal(m, pinned);

  Bitboard target = checkers();
  Square checksq = pop_1st_bit(&target);

  if (target) // double check ?
      return false;

  // Our move must be a blocking evasion or a capture of the checking piece
  target = squares_between(checksq, king_square(us)) | checkers();
  return bit_is_set(target, to) && pl_move_is_legal(m, pinned);
}

/// Position::move_is_legal() takes a position and a (not necessarily pseudo-legal)
/// move and tests whether the move is legal. This version is not very fast and
/// should be used only in non time-critical paths.

bool Position::move_is_legal(const Move m) const {

  MoveStack mlist[MAX_MOVES];
  MoveStack *cur, *last = generate<MV_PSEUDO_LEGAL>(*this, mlist);

   for (cur = mlist; cur != last; cur++)
      if (cur->move == m)
          return pl_move_is_legal(m, pinned_pieces(sideToMove));

  return false;
}


/// Fast version of Position::move_is_legal() that takes a position a move and
/// a bitboard of pinned pieces as input, and tests whether the move is legal.

bool Position::move_is_legal(const Move m, Bitboard pinned) const {

  assert(is_ok());
  assert(pinned == pinned_pieces(sideToMove));

  Color us = sideToMove;
  Color them = opposite_color(sideToMove);
  Square from = move_from(m);
  Square to = move_to(m);
  Piece pc = piece_on(from);

  // Use a slower but simpler function for uncommon cases
  if (move_is_special(m))
      return move_is_legal(m);

  // If the from square is not occupied by a piece belonging to the side to
  // move, the move is obviously not legal.
  if (color_of_piece(pc) != us)
      return false;

  // The destination square cannot be occupied by a friendly piece
  if (color_of_piece_on(to) == us)
      return false;

  // Handle the special case of a pawn move
  if (type_of_piece(pc) == PAWN)
  {
      // Move direction must be compatible with pawn color
      int direction = to - from;
      if ((us == WHITE) != (direction > 0))
          return false;

      // We have already handled promotion moves, so destination
      // cannot be on the 8/1th rank.
      if (square_rank(to) == RANK_8 || square_rank(to) == RANK_1)
          return false;

      // Proceed according to the square delta between the origin and
      // destination squares.
      switch (direction)
      {
      case DELTA_NW:
      case DELTA_NE:
      case DELTA_SW:
      case DELTA_SE:
      // Capture. The destination square must be occupied by an enemy
      // piece (en passant captures was handled earlier).
          if (color_of_piece_on(to) != them)
              return false;
          break;

      case DELTA_N:
      case DELTA_S:
      // Pawn push. The destination square must be empty.
          if (!square_is_empty(to))
              return false;
          break;

      case DELTA_NN:
      // Double white pawn push. The destination square must be on the fourth
      // rank, and both the destination square and the square between the
      // source and destination squares must be empty.
      if (   square_rank(to) != RANK_4
          || !square_is_empty(to)
          || !square_is_empty(from + DELTA_N))
          return false;
          break;

      case DELTA_SS:
      // Double black pawn push. The destination square must be on the fifth
      // rank, and both the destination square and the square between the
      // source and destination squares must be empty.
          if (   square_rank(to) != RANK_5
              || !square_is_empty(to)
              || !square_is_empty(from + DELTA_S))
              return false;
          break;

      default:
          return false;
      }
  }
  else if (!bit_is_set(attacks_from(pc, from), to))
      return false;

  // The move is pseudo-legal, check if it is also legal
  return in_check() ? pl_move_is_evasion(m, pinned) : pl_move_is_legal(m, pinned);
}


/// Position::move_gives_check() tests whether a pseudo-legal move is a check

bool Position::move_gives_check(Move m) const {

  return move_gives_check(m, CheckInfo(*this));
}

bool Position::move_gives_check(Move m, const CheckInfo& ci) const {

  assert(is_ok());
  assert(move_is_ok(m));
  assert(ci.dcCandidates == discovered_check_candidates(side_to_move()));

  //assert(color_of_piece_on(move_from(m)) == side_to_move());

  assert(piece_on(ci.ksq) == make_piece(opposite_color(side_to_move()), KING));

  Square from = move_from(m);
  Square to = move_to(m);
  PieceType pt = type_of_piece_on(from);

  // Direct check ?
  if (bit_is_set(ci.checkSq[pt], to))
      return true;

  // Discovery check ?
  if (ci.dcCandidates && bit_is_set(ci.dcCandidates, from))
  {
      // For pawn and king moves we need to verify also direction
      if (  (pt != PAWN && pt != KING)
          || !squares_aligned(from, to, ci.ksq))
          return true;
  }

  // Can we skip the ugly special cases ?
  if (!move_is_special(m))
      return false;

  Color us = side_to_move();
  Bitboard b = occupied_squares();

  // Promotion with check ?
  if (move_is_promotion(m))
  {
      clear_bit(&b, from);

      switch (move_promotion_piece(m))
      {
      case KNIGHT:
          return bit_is_set(attacks_from<KNIGHT>(to), ci.ksq);
      case BISHOP:
          return bit_is_set(bishop_attacks_bb(to, b), ci.ksq);
      case ROOK:
          return bit_is_set(rook_attacks_bb(to, b), ci.ksq);
      case QUEEN:
          return bit_is_set(queen_attacks_bb(to, b), ci.ksq);
      default:
          assert(false);
      }
  }

  // En passant capture with check ? We have already handled the case
  // of direct checks and ordinary discovered check, the only case we
  // need to handle is the unusual case of a discovered check through
  // the captured pawn.
  if (move_is_ep(m))
  {
      Square capsq = make_square(square_file(to), square_rank(from));
      clear_bit(&b, from);
      clear_bit(&b, capsq);
      set_bit(&b, to);
      return  (rook_attacks_bb(ci.ksq, b) & pieces(ROOK, QUEEN, us))
            ||(bishop_attacks_bb(ci.ksq, b) & pieces(BISHOP, QUEEN, us));
  }

  // Castling with check ?
  if (move_is_castle(m))
  {
      Square kfrom, kto, rfrom, rto;
      kfrom = from;
      rfrom = to;

      if (rfrom > kfrom)
      {
          kto = relative_square(us, SQ_G1);
          rto = relative_square(us, SQ_F1);
      } else {
          kto = relative_square(us, SQ_C1);
          rto = relative_square(us, SQ_D1);
      }
      clear_bit(&b, kfrom);
      clear_bit(&b, rfrom);
      set_bit(&b, rto);
      set_bit(&b, kto);
      return bit_is_set(rook_attacks_bb(rto, b), ci.ksq);
  }

  return false;
}

/// Position::do_move() makes a move, and saves all information necessary
/// to a StateInfo object. The move is assumed to be legal. Pseudo-legal
/// moves should be filtered out before this function is called.

void Position::do_move(Move m) {

  CheckInfo ci(*this);
  do_move(m, ci, move_gives_check(m, ci));
}

void Position::do_move(Move m, const CheckInfo& ci, bool moveIsCheck) {
  assert(is_ok());
  assert(move_is_ok(m));
  //assert(&newSt != st);

  Key key = _stateInfo->key;

  // Copy some fields of old state to our new StateInfo object except the
  // ones which are recalculated from scratch anyway, then switch our state
  // pointer to point to the new, ready to be updated, state.
  struct ReducedStateInfo {
    Key pawnKey, materialKey;
    int castleRights, rule50, gamePly, pliesFromNull;
    Square epSquare;
    BScore value;
    Value npMaterial[2];
  };

//  memcpy(&newSt, st, sizeof(ReducedStateInfo));
  //newSt.previous = st;
//  st = &newSt;

  // Save the current key to the history[] array, in order to be able to
  // detect repetition draws.
  // TODO: Fix this! _recentHistory[(_historyIndex++) % HISTORY_HORIZON] = key;

  // Update side to move
  key ^= zobSideToMove;

  // Increment the 50 moves rule draw counter. Resetting it to zero in the
  // case of non-reversible moves is taken care of later.
  _stateInfo->rule50++;
  _stateInfo->pliesFromNull++;

  if (move_is_castle(m))
  {
      _stateInfo->key = key;
      do_castle_move(m);
      return;
  }

  Color us = side_to_move();
  Color them = opposite_color(us);
  Square from = move_from(m);
  Square to = move_to(m);
  bool ep = move_is_ep(m);
  bool pm = move_is_promotion(m);

  Piece piece = piece_on(from);
  PieceType pt = type_of_piece(piece);
  PieceType capture = ep ? PAWN : type_of_piece_on(to);

  assert(color_of_piece_on(from) == us);


// TODO: FIX THIS!!!!!!!!!!  assert(color_of_piece_on(to) == them || square_is_empty(to));


  assert(!(ep || pm) || piece == make_piece(us, PAWN));
  assert(!pm || relative_rank(us, to) == RANK_8);

  if (capture)
      do_capture_move(key, capture, them, to, ep);

  // Update hash key
  key ^= zobrist[us][pt][from] ^ zobrist[us][pt][to];

  // Reset en passant square
  if (_stateInfo->epSquare != SQ_NONE)
  {
      key ^= zobEp[_stateInfo->epSquare];
      _stateInfo->epSquare = SQ_NONE;
  }

  // Update castle rights, try to shortcut a common case
  int cm = castleRightsMask[from] & castleRightsMask[to];
  if (cm != ALL_CASTLES && ((cm & _stateInfo->castleRights) != _stateInfo->castleRights))
  {
      key ^= zobCastle[_stateInfo->castleRights];
      _stateInfo->castleRights &= castleRightsMask[from];
      _stateInfo->castleRights &= castleRightsMask[to];
      key ^= zobCastle[_stateInfo->castleRights];
  }

  // Prefetch TT access as soon as we know key is updated
  // TOOD: prefetch((char*)TT.first_entry(key));

  // Move the piece
  Bitboard move_bb = make_move_bb(from, to);
  do_move_bb(&(byColorBB[us]), move_bb);
  do_move_bb(&(byTypeBB[pt]), move_bb);
  do_move_bb(&(byTypeBB[0]), move_bb); // HACK: byTypeBB[0] == occupied squares

  board[to] = board[from];
  board[from] = PIECE_NONE;

  // Update piece lists, note that index[from] is not updated and
  // becomes stale. This works as long as index[] is accessed just
  // by known occupied squares.
  index[to] = index[from];
  pieceList[us][pt][index[to]] = to;

  // If the moving piece was a pawn do some special extra work
  if (pt == PAWN)
  {
      // Reset rule 50 draw counter
      _stateInfo->rule50 = 0;

      // Update pawn hash key and prefetch in L1/L2 cache
      _stateInfo->pawnKey ^= zobrist[us][PAWN][from] ^ zobrist[us][PAWN][to];

      // Set en passant square, only if moved pawn can be captured
      if ((to ^ from) == 16)
      {
          if (attacks_from<PAWN>(from + (us == WHITE ? DELTA_N : DELTA_S), us) & pieces(PAWN, them))
          {
              _stateInfo->epSquare = Square((int(from) + int(to)) / 2);
              key ^= zobEp[_stateInfo->epSquare];
          }
      }

      if (pm) // promotion ?
      {
          PieceType promotion = move_promotion_piece(m);

          assert(promotion >= KNIGHT && promotion <= QUEEN);

          // Insert promoted piece instead of pawn
          clear_bit(&(byTypeBB[PAWN]), to);
          set_bit(&(byTypeBB[promotion]), to);
          board[to] = make_piece(us, promotion);

          // Update piece counts
          pieceCount[us][promotion]++;
          pieceCount[us][PAWN]--;

          // Update material key
          _stateInfo->materialKey ^= zobrist[us][PAWN][pieceCount[us][PAWN]];
          _stateInfo->materialKey ^= zobrist[us][promotion][pieceCount[us][promotion]-1];

          // Update piece lists, move the last pawn at index[to] position
          // and shrink the list. Add a new promotion piece to the list.
          Square lastPawnSquare = pieceList[us][PAWN][pieceCount[us][PAWN]];
          index[lastPawnSquare] = index[to];
          pieceList[us][PAWN][index[lastPawnSquare]] = lastPawnSquare;
          pieceList[us][PAWN][pieceCount[us][PAWN]] = SQ_NONE;
          index[to] = pieceCount[us][promotion] - 1;
          pieceList[us][promotion][index[to]] = to;

          // Partially revert hash keys update
          key ^= zobrist[us][PAWN][to] ^ zobrist[us][promotion][to];
          _stateInfo->pawnKey ^= zobrist[us][PAWN][to];

          // Partially revert and update incremental scores
          _stateInfo->value -= pst(us, PAWN, to);
          _stateInfo->value += pst(us, promotion, to);

          // Update material
          _stateInfo->npMaterial[us] += PieceValueMidgame[promotion];
      }
  }

  // Prefetch pawn and material hash tables
  //Threads[threadID].pawnTable.prefetch(_stateInfo->pawnKey);
  //Threads[threadID].materialTable.prefetch(_stateInfo->materialKey);

  // Update incremental scores
  _stateInfo->value += pst_delta(piece, from, to);

  // Set capture piece
  _stateInfo->capturedType = capture;

  // Update the key with the final value
  _stateInfo->key = key;

  // Update checkers bitboard, piece must be already moved
  _stateInfo->checkersBB = EmptyBoardBB;

  if (moveIsCheck)
  {
      if (ep | pm)
          _stateInfo->checkersBB = attackers_to(king_square(them)) & pieces_of_color(us);
      else
      {
          // Direct checks
          if (bit_is_set(ci.checkSq[pt], to))
              _stateInfo->checkersBB = SetMaskBB[to];

          // Discovery checks
          if (ci.dcCandidates && bit_is_set(ci.dcCandidates, from))
          {
              if (pt != ROOK)
                  _stateInfo->checkersBB |= (attacks_from<ROOK>(ci.ksq) & pieces(ROOK, QUEEN, us));

              if (pt != BISHOP)
                  _stateInfo->checkersBB |= (attacks_from<BISHOP>(ci.ksq) & pieces(BISHOP, QUEEN, us));
          }
      }
  }

  // Finish
  sideToMove = opposite_color(sideToMove);
  _stateInfo->value += (sideToMove == WHITE ?  TempoValue : -TempoValue);

  assert(is_ok());
}


/// Position::do_capture_move() is a private method used to update captured
/// piece info. It is called from the main Position::do_move function.

void Position::do_capture_move(Key& key, PieceType capture, Color them, Square to, bool ep) {

    assert(capture != KING);

    Square capsq = to;

    // If the captured piece was a pawn, update pawn hash key,
    // otherwise update non-pawn material.
    if (capture == PAWN)
    {
        if (ep) // en passant ?
        {
            capsq = (them == BLACK)? (to - DELTA_N) : (to - DELTA_S);

            assert(to == _stateInfo->epSquare);
            assert(relative_rank(opposite_color(them), to) == RANK_6);
            assert(piece_on(to) == PIECE_NONE);
            assert(piece_on(capsq) == make_piece(them, PAWN));

            board[capsq] = PIECE_NONE;
        }
        _stateInfo->pawnKey ^= zobrist[them][PAWN][capsq];
    }
    else
        _stateInfo->npMaterial[them] -= PieceValueMidgame[capture];

    // Remove captured piece
    clear_bit(&(byColorBB[them]), capsq);
    clear_bit(&(byTypeBB[capture]), capsq);
    clear_bit(&(byTypeBB[0]), capsq);

    // Update hash key
    key ^= zobrist[them][capture][capsq];

    // Update incremental scores
    _stateInfo->value -= pst(them, capture, capsq);

    // Update piece count
    pieceCount[them][capture]--;

    // Update material hash key
    _stateInfo->materialKey ^= zobrist[them][capture][pieceCount[them][capture]];

    // Update piece list, move the last piece at index[capsq] position
    //
    // WARNING: This is a not perfectly revresible operation. When we
    // will reinsert the captured piece in undo_move() we will put it
    // at the end of the list and not in its original place, it means
    // index[] and pieceList[] are not guaranteed to be invariant to a
    // do_move() + undo_move() sequence.
    Square lastPieceSquare = pieceList[them][capture][pieceCount[them][capture]];
    index[lastPieceSquare] = index[capsq];
    pieceList[them][capture][index[lastPieceSquare]] = lastPieceSquare;
    pieceList[them][capture][pieceCount[them][capture]] = SQ_NONE;

    // Reset rule 50 counter
    _stateInfo->rule50 = 0;
}


/// Position::do_castle_move() is a private method used to make a castling
/// move. It is called from the main Position::do_move function. Note that
/// castling moves are encoded as "king captures friendly rook" moves, for
/// instance white short castling in a non-Chess960 game is encoded as e1h1.

void Position::do_castle_move(Move m) {

  assert(move_is_ok(m));
  assert(move_is_castle(m));

  Color us = side_to_move();
  Color them = opposite_color(us);

  // Reset capture field
  _stateInfo->capturedType = PIECE_TYPE_NONE;

  // Find source squares for king and rook
  Square kfrom = move_from(m);
  Square rfrom = move_to(m);  // HACK: See comment at beginning of function
  Square kto, rto;

  assert(piece_on(kfrom) == make_piece(us, KING));
  assert(piece_on(rfrom) == make_piece(us, ROOK));

  // Find destination squares for king and rook
  if (rfrom > kfrom) // O-O
  {
      kto = relative_square(us, SQ_G1);
      rto = relative_square(us, SQ_F1);
  } else { // O-O-O
      kto = relative_square(us, SQ_C1);
      rto = relative_square(us, SQ_D1);
  }

  // Remove pieces from source squares:
  clear_bit(&(byColorBB[us]), kfrom);
  clear_bit(&(byTypeBB[KING]), kfrom);
  clear_bit(&(byTypeBB[0]), kfrom); // HACK: byTypeBB[0] == occupied squares
  clear_bit(&(byColorBB[us]), rfrom);
  clear_bit(&(byTypeBB[ROOK]), rfrom);
  clear_bit(&(byTypeBB[0]), rfrom); // HACK: byTypeBB[0] == occupied squares

  // Put pieces on destination squares:
  set_bit(&(byColorBB[us]), kto);
  set_bit(&(byTypeBB[KING]), kto);
  set_bit(&(byTypeBB[0]), kto); // HACK: byTypeBB[0] == occupied squares
  set_bit(&(byColorBB[us]), rto);
  set_bit(&(byTypeBB[ROOK]), rto);
  set_bit(&(byTypeBB[0]), rto); // HACK: byTypeBB[0] == occupied squares

  // Update board array
  Piece king = make_piece(us, KING);
  Piece rook = make_piece(us, ROOK);
  board[kfrom] = board[rfrom] = PIECE_NONE;
  board[kto] = king;
  board[rto] = rook;

  // Update piece lists
  pieceList[us][KING][index[kfrom]] = kto;
  pieceList[us][ROOK][index[rfrom]] = rto;
  int tmp = index[rfrom]; // In Chess960 could be rto == kfrom
  index[kto] = index[kfrom];
  index[rto] = tmp;

  // Update incremental scores
  _stateInfo->value += pst_delta(king, kfrom, kto);
  _stateInfo->value += pst_delta(rook, rfrom, rto);

  // Update hash key
  _stateInfo->key ^= zobrist[us][KING][kfrom] ^ zobrist[us][KING][kto];
  _stateInfo->key ^= zobrist[us][ROOK][rfrom] ^ zobrist[us][ROOK][rto];

  // Clear en passant square
  if (_stateInfo->epSquare != SQ_NONE)
  {
      _stateInfo->key ^= zobEp[_stateInfo->epSquare];
      _stateInfo->epSquare = SQ_NONE;
  }

  // Update castling rights
  _stateInfo->key ^= zobCastle[_stateInfo->castleRights];
  _stateInfo->castleRights &= castleRightsMask[kfrom];
  _stateInfo->key ^= zobCastle[_stateInfo->castleRights];

  // Reset rule 50 counter
  _stateInfo->rule50 = 0;

  // Update checkers BB
  _stateInfo->checkersBB = attackers_to(king_square(them)) & pieces_of_color(us);

  // Finish
  sideToMove = opposite_color(sideToMove);
  _stateInfo->value += (sideToMove == WHITE ?  TempoValue : -TempoValue);

  assert(is_ok());
}

/// Position::see() is a static exchange evaluator: It tries to estimate the
/// material gain or loss resulting from a move. There are three versions of
/// this function: One which takes a destination square as input, one takes a
/// move, and one which takes a 'from' and a 'to' square. The function does
/// not yet understand promotions captures.

int Position::see(Move m) const {

  assert(move_is_ok(m));
  return see(move_from(m), move_to(m));
}

int Position::see_sign(Move m) const {

  assert(move_is_ok(m));

  Square from = move_from(m);
  Square to = move_to(m);

  // Early return if SEE cannot be negative because captured piece value
  // is not less then capturing one. Note that king moves always return
  // here because king midgame value is set to 0.
  if (midgame_value_of_piece_on(to) >= midgame_value_of_piece_on(from))
      return 1;

  return see(from, to);
}

int Position::see(Square from, Square to) const {

  Bitboard occupied, attackers, stmAttackers, b;
  int swapList[32], slIndex = 1;
  PieceType capturedType, pt;
  Color stm;

  assert(square_is_ok(from));
  assert(square_is_ok(to));

  capturedType = type_of_piece_on(to);

  // King cannot be recaptured
  if (capturedType == KING)
      return seeValues[capturedType];

  occupied = occupied_squares();

  // Handle en-passant moves
  if (_stateInfo->epSquare == to && type_of_piece_on(from) == PAWN)
  {
      Square capQq = (side_to_move() == WHITE ? to - DELTA_N : to - DELTA_S);

      assert(capturedType == PIECE_TYPE_NONE);
      assert(type_of_piece_on(capQq) == PAWN);

      // Remove the captured pawn
      clear_bit(&occupied, capQq);
      capturedType = PAWN;
  }

  // Find all attackers to the destination square, with the moving piece
  // removed, but possibly an X-ray attacker added behind it.
  clear_bit(&occupied, from);
  attackers =  (rook_attacks_bb(to, occupied)  & pieces(ROOK, QUEEN))
             | (bishop_attacks_bb(to, occupied)& pieces(BISHOP, QUEEN))
             | (attacks_from<KNIGHT>(to)       & pieces(KNIGHT))
             | (attacks_from<KING>(to)         & pieces(KING))
             | (attacks_from<PAWN>(to, WHITE)  & pieces(PAWN, BLACK))
             | (attacks_from<PAWN>(to, BLACK)  & pieces(PAWN, WHITE));

  // If the opponent has no attackers we are finished
  stm = opposite_color(color_of_piece_on(from));
  stmAttackers = attackers & pieces_of_color(stm);
  if (!stmAttackers)
      return seeValues[capturedType];

  // The destination square is defended, which makes things rather more
  // difficult to compute. We proceed by building up a "swap list" containing
  // the material gain or loss at each stop in a sequence of captures to the
  // destination square, where the sides alternately capture, and always
  // capture with the least valuable piece. After each capture, we look for
  // new X-ray attacks from behind the capturing piece.
  swapList[0] = seeValues[capturedType];
  capturedType = type_of_piece_on(from);

  do {
      // Locate the least valuable attacker for the side to move. The loop
      // below looks like it is potentially infinite, but it isn't. We know
      // that the side to move still has at least one attacker left.
      for (pt = PAWN; !(stmAttackers & pieces(pt)); pt++)
          assert(pt < KING);

      // Remove the attacker we just found from the 'occupied' bitboard,
      // and scan for new X-ray attacks behind the attacker.
      b = stmAttackers & pieces(pt);
      occupied ^= (b & (~b + 1));
      attackers |=  (rook_attacks_bb(to, occupied)   & pieces(ROOK, QUEEN))
                  | (bishop_attacks_bb(to, occupied) & pieces(BISHOP, QUEEN));

      attackers &= occupied; // Cut out pieces we've already done

      // Add the new entry to the swap list
      assert(slIndex < 32);
      swapList[slIndex] = -swapList[slIndex - 1] + seeValues[capturedType];
      slIndex++;

      // Remember the value of the capturing piece, and change the side to
      // move before beginning the next iteration.
      capturedType = pt;
      stm = opposite_color(stm);
      stmAttackers = attackers & pieces_of_color(stm);

      // Stop before processing a king capture
      if (capturedType == KING && stmAttackers)
      {
          assert(slIndex < 32);
          swapList[slIndex++] = QueenValueMidgame*10;
          break;
      }
  } while (stmAttackers);

  // Having built the swap list, we negamax through it to find the best
  // achievable score from the point of view of the side to move.
  while (--slIndex)
      swapList[slIndex-1] = Min(-swapList[slIndex], swapList[slIndex-1]);

  return swapList[0];
}


/// Position::clear() erases the position object to a pristine state, with an
/// empty board, white to move, and no castling rights.

void Position::clear() {

  //st = &startState;
  memset(_stateInfo, 0, sizeof(StateInfo));
  _stateInfo->epSquare = SQ_NONE;

  memset(byColorBB,  0, sizeof(Bitboard) * 2);
  memset(byTypeBB,   0, sizeof(Bitboard) * 8);
  memset(pieceCount, 0, sizeof(int) * 2 * 8);
  memset(index,      0, sizeof(int) * 64);

  for (int i = 0; i < 64; i++)
      board[i] = PIECE_NONE;

  for (int i = 0; i < 8; i++)
      for (int j = 0; j < 16; j++)
          pieceList[0][i][j] = pieceList[1][i][j] = SQ_NONE;

  for (Square sq = SQ_A1; sq <= SQ_H8; sq++)
      castleRightsMask[sq] = ALL_CASTLES;

  sideToMove = WHITE;
  _initialKFile = FILE_E;
  _initialKRFile = FILE_H;
  _initialQRFile = FILE_A;
}


/// Position::put_piece() puts a piece on the given square of the board,
/// updating the board array, pieces list, bitboards, and piece counts.

void Position::put_piece(Piece p, Square s) {

  Color c = color_of_piece(p);
  PieceType pt = type_of_piece(p);

  board[s] = p;
  index[s] = pieceCount[c][pt]++;
  pieceList[c][pt][index[s]] = s;

  set_bit(&(byTypeBB[pt]), s);
  set_bit(&(byColorBB[c]), s);
  set_bit(&(byTypeBB[0]), s); // HACK: byTypeBB[0] contains all occupied squares.
}


/// Position::compute_key() computes the hash key of the position. The hash
/// key is usually updated incrementally as moves are made and unmade, the
/// compute_key() function is only used when a new position is set up, and
/// to verify the correctness of the hash key when running in debug mode.

Key Position::compute_key() const {

  Key result = zobCastle[_stateInfo->castleRights];

  for (Square s = SQ_A1; s <= SQ_H8; s++)
      if (square_is_occupied(s))
          result ^= zobrist[color_of_piece_on(s)][type_of_piece_on(s)][s];

  if (ep_square() != SQ_NONE)
      result ^= zobEp[ep_square()];

  if (side_to_move() == BLACK)
      result ^= zobSideToMove;

  return result;
}


/// Position::compute_pawn_key() computes the hash key of the position. The
/// hash key is usually updated incrementally as moves are made and unmade,
/// the compute_pawn_key() function is only used when a new position is set
/// up, and to verify the correctness of the pawn hash key when running in
/// debug mode.

Key Position::compute_pawn_key() const {

  Bitboard b;
  Key result = 0;

  for (Color c = WHITE; c <= BLACK; c++)
  {
      b = pieces(PAWN, c);
      while (b)
          result ^= zobrist[c][PAWN][pop_1st_bit(&b)];
  }
  return result;
}


/// Position::compute_material_key() computes the hash key of the position.
/// The hash key is usually updated incrementally as moves are made and unmade,
/// the compute_material_key() function is only used when a new position is set
/// up, and to verify the correctness of the material hash key when running in
/// debug mode.

Key Position::compute_material_key() const {

  int count;
  Key result = 0;

  for (Color c = WHITE; c <= BLACK; c++)
      for (PieceType pt = PAWN; pt <= QUEEN; pt++)
      {
          count = piece_count(c, pt);
          for (int i = 0; i < count; i++)
              result ^= zobrist[c][pt][i];
      }
  return result;
}


/// Position::compute_value() compute the incremental scores for the middle
/// game and the endgame. These functions are used to initialize the incremental
/// scores when a new position is set up, and to verify that the scores are correctly
/// updated by do_move and undo_move when the program is running in debug mode.
BScore Position::compute_value() const {

  Bitboard b;
  BScore result = SCORE_ZERO;

  for (Color c = WHITE; c <= BLACK; c++)
      for (PieceType pt = PAWN; pt <= KING; pt++)
      {
          b = pieces(pt, c);
          while (b)
              result += pst(c, pt, pop_1st_bit(&b));
      }

  result += (side_to_move() == WHITE ? TempoValue / 2 : -TempoValue / 2);
  return result;
}


/// Position::compute_non_pawn_material() computes the total non-pawn middle
/// game material value for the given side. Material values are updated
/// incrementally during the search, this function is only used while
/// initializing a new Position object.

Value Position::compute_non_pawn_material(Color c) const {

  Value result = VALUE_ZERO;

  for (PieceType pt = KNIGHT; pt <= QUEEN; pt++)
      result += piece_count(c, pt) * PieceValueMidgame[pt];

  return result;
}


/// Position::is_draw() tests whether the position is drawn by material,
/// repetition, or the 50 moves rule. It does not detect stalemates, this
/// must be done by the search.

int Position::is_draw() const {

  // Draw by material?
  if (   !pieces(PAWN)
      && (non_pawn_material(WHITE) + non_pawn_material(BLACK) <= BishopValueMidgame))
      return 1;

  // Draw by the 50 moves rule?
  if (_stateInfo->rule50 > 99 && !is_mate())
      return 2;

  // Draw by repetition?
  /*

  TODO: Fix this!

  for (int i = 4, e = Min(Min(_stateInfo->gamePly, _stateInfo->rule50), _stateInfo->pliesFromNull); i <= e; i += 2)
  {
      if (_recentHistory[_stateInfo->gamePly - i] == _stateInfo->key)
      {
          return 3;
      }
  }
  */

  return 0;
}


/// Position::is_mate() returns true or false depending on whether the
/// side to move is checkmated.

bool Position::is_mate() const {

  MoveStack moves[MAX_MOVES];
  return in_check() && generate<MV_LEGAL>(*this, moves) == moves;
}


/// Position::init_zobrist() is a static member function which initializes at
/// startup the various arrays used to compute hash keys.

void Position::init_zobrist() {

  int i,j, k;
  RKISS rk;

  for (i = 0; i < 2; i++) for (j = 0; j < 8; j++) for (k = 0; k < 64; k++)
      zobrist[i][j][k] = rk.rand<Key>();

  for (i = 0; i < 64; i++)
      zobEp[i] = rk.rand<Key>();

  for (i = 0; i < 16; i++)
      zobCastle[i] = rk.rand<Key>();

  zobSideToMove = rk.rand<Key>();
  zobExclusion  = rk.rand<Key>();
}


/// Position::init_piece_square_tables() initializes the piece square tables.
/// This is a two-step operation: First, the white halves of the tables are
/// copied from the MgPST[][] and EgPST[][] arrays. Second, the black halves
/// of the tables are initialized by mirroring and changing the sign of the
/// corresponding white scores.

void Position::init_piece_square_tables() {

  for (Square s = SQ_A1; s <= SQ_H8; s++)
      for (Piece p = WP; p <= WK; p++)
          PieceSquareTable[p][s] = make_score(MgPST[p][s], EgPST[p][s]);

  for (Square s = SQ_A1; s <= SQ_H8; s++)
      for (Piece p = BP; p <= BK; p++)
          PieceSquareTable[p][s] = -PieceSquareTable[p-8][flip_square(s)];
}


/// Position::is_ok() performs some consitency checks for the position object.
/// This is meant to be helpful when debugging.

bool Position::is_ok(int* failedStep) const {

  // What features of the position should be verified?
  const bool debugAll = false;

  const bool debugBitboards       = debugAll || false;
  const bool debugKingCount       = debugAll || false;
  const bool debugKingCapture     = debugAll || false;
  const bool debugCheckerCount    = debugAll || false;
  const bool debugKey             = debugAll || false;
  const bool debugMaterialKey     = debugAll || false;
  const bool debugPawnKey         = debugAll || false;
  const bool debugIncrementalEval = debugAll || false;
  const bool debugNonPawnMaterial = debugAll || false;
  const bool debugPieceCounts     = debugAll || false;
  const bool debugPieceList       = debugAll || false;
  const bool debugCastleSquares   = debugAll || false;

  if (failedStep) *failedStep = 1;

  // Side to move OK?
  if (!color_is_ok(side_to_move()))
      return false;

  // Are the king squares in the position correct?
  if (failedStep) (*failedStep)++;
  if (piece_on(king_square(WHITE)) != WK)
      return false;

  if (failedStep) (*failedStep)++;
  if (piece_on(king_square(BLACK)) != BK)
      return false;

  // Castle files OK?
  if (failedStep) (*failedStep)++;
  if (!file_is_ok(_initialKRFile))
      return false;

  if (!file_is_ok(_initialQRFile))
      return false;

  // Do both sides have exactly one king?
  if (failedStep) (*failedStep)++;
  if (debugKingCount)
  {
      int kingCount[2] = {0, 0};
      for (Square s = SQ_A1; s <= SQ_H8; s++)
          if (type_of_piece_on(s) == KING)
              kingCount[color_of_piece_on(s)]++;

      if (kingCount[0] != 1 || kingCount[1] != 1)
          return false;
  }

  // Can the side to move capture the opponent's king?
  if (failedStep) (*failedStep)++;
  if (debugKingCapture)
  {
      Color us = side_to_move();
      Color them = opposite_color(us);
      Square ksq = king_square(them);
      if (attackers_to(ksq) & pieces_of_color(us))
          return false;
  }

  // Is there more than 2 checkers?
  if (failedStep) (*failedStep)++;
  if (debugCheckerCount && count_1s<CNT32>(_stateInfo->checkersBB) > 2)
      return false;

  // Bitboards OK?
  if (failedStep) (*failedStep)++;
  if (debugBitboards)
  {
      // The intersection of the white and black pieces must be empty
      if ((pieces_of_color(WHITE) & pieces_of_color(BLACK)) != EmptyBoardBB)
          return false;

      // The union of the white and black pieces must be equal to all
      // occupied squares
      if ((pieces_of_color(WHITE) | pieces_of_color(BLACK)) != occupied_squares())
          return false;

      // Separate piece type bitboards must have empty intersections
      for (PieceType p1 = PAWN; p1 <= KING; p1++)
          for (PieceType p2 = PAWN; p2 <= KING; p2++)
              if (p1 != p2 && (pieces(p1) & pieces(p2)))
                  return false;
  }

  // En passant square OK?
  if (failedStep) (*failedStep)++;
  if (ep_square() != SQ_NONE)
  {
      // The en passant square must be on rank 6, from the point of view of the
      // side to move.
      if (relative_rank(side_to_move(), ep_square()) != RANK_6)
          return false;
  }

  // Hash key OK?
  if (failedStep) (*failedStep)++;
  if (debugKey && _stateInfo->key != compute_key())
      return false;

  // Pawn hash key OK?
  if (failedStep) (*failedStep)++;
  if (debugPawnKey && _stateInfo->pawnKey != compute_pawn_key())
      return false;

  // Material hash key OK?
  if (failedStep) (*failedStep)++;
  if (debugMaterialKey && _stateInfo->materialKey != compute_material_key())
      return false;

  // Incremental eval OK?
  if (failedStep) (*failedStep)++;
  if (debugIncrementalEval && _stateInfo->value != compute_value())
      return false;

  // Non-pawn material OK?
  if (failedStep) (*failedStep)++;
  if (debugNonPawnMaterial)
  {
      if (_stateInfo->npMaterial[WHITE] != compute_non_pawn_material(WHITE))
          return false;

      if (_stateInfo->npMaterial[BLACK] != compute_non_pawn_material(BLACK))
          return false;
  }

  // Piece counts OK?
  if (failedStep) (*failedStep)++;
  if (debugPieceCounts)
      for (Color c = WHITE; c <= BLACK; c++)
          for (PieceType pt = PAWN; pt <= KING; pt++)
              if (pieceCount[c][pt] != count_1s<CNT32>(pieces(pt, c)))
                  return false;

  if (failedStep) (*failedStep)++;
  if (debugPieceList)
      for (Color c = WHITE; c <= BLACK; c++)
          for (PieceType pt = PAWN; pt <= KING; pt++)
              for (int i = 0; i < pieceCount[c][pt]; i++)
              {
                  if (piece_on(piece_list(c, pt, i)) != make_piece(c, pt))
                      return false;

                  if (index[piece_list(c, pt, i)] != i)
                      return false;
              }

  if (failedStep) (*failedStep)++;
  if (debugCastleSquares)
  {
      for (Color c = WHITE; c <= BLACK; c++)
      {
          if (can_castle_kingside(c) && piece_on(initial_kr_square(c)) != make_piece(c, ROOK))
              return false;

          if (can_castle_queenside(c) && piece_on(initial_qr_square(c)) != make_piece(c, ROOK))
              return false;
      }
      if (castleRightsMask[initial_kr_square(WHITE)] != (ALL_CASTLES ^ WHITE_OO))
          return false;
      if (castleRightsMask[initial_qr_square(WHITE)] != (ALL_CASTLES ^ WHITE_OOO))
          return false;
      if (castleRightsMask[initial_kr_square(BLACK)] != (ALL_CASTLES ^ BLACK_OO))
          return false;
      if (castleRightsMask[initial_qr_square(BLACK)] != (ALL_CASTLES ^ BLACK_OOO))
          return false;
  }

  if (failedStep) *failedStep = 0;
  return true;
}
}
