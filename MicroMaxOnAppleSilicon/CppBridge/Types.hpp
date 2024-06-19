#ifndef TYPES_HPP
#define TYPES_HPP

typedef enum
{
    WhitePlayer,
    BlackPlayer
} Player;

/*
 * Reference:
 *     enum Piece
 *     {
 *         PIECE_NONE_DARK_SQ = 0, WP = 1, WN = 2, WB = 3, WR = 4, WQ = 5, WK = 6,
 *         BP = 9, BN = 10, BB = 11, BR = 12, BQ = 13, BK = 14, PIECE_NONE = 16
 *     };
 *
 * Important: The orders here are important and integrated into the application.
 */

typedef enum VirtualPiece
{
    NilPiece    = 16,
    BlackPawn   = 9,
    BlackKnight = 10,
    BlackBishop = 11,
    BlackRook   = 12,
    BlackQueen  = 13,
    BlackKing   = 14,
    WhitePawn   = 1,
    WhiteKnight = 2,
    WhiteBishop = 3,
    WhiteRook   = 4,
    WhiteQueen  = 5,
    WhiteKing   = 6,
    
    AnyKing   = 20,
    AnyQueen  = 21,
    AnyRook   = 22,
    AnyBishop = 23,
    AnyKnight = 24,
    AnyPawn   = 25,
    
    CustomBase = 100,
    
    /*
     * Anything >= CustomBase is also a custom piece
     */
    
} VirtualPiece;

#define VALID_MOVE           0
#define INVALID_MOVE        (1 << 0)
#define CAPTURE_MOVE        (1 << 1)
#define PROMOTION_MOVE      (1 << 2)
#define LONG_CASTLE_MOVE    (1 << 3)
#define SHORT_CASTLE_MOVE   (1 << 4)
#define CHECK_MOVE          (1 << 5)
#define CHECKMATE_MOVE      (1 << 6)
#define EN_PASSANT_MOVE     (1 << 7)
#define STALEMATE_MOVE      (1 << 8)
#define PROMOTE_QUEEN_MOVE  (1 << 9)
#define PROMOTE_ROOK_MOVE   (1 << 10)
#define PROMOTE_BISHOP_MOVE (1 << 11)
#define PROMOTE_KNIGHT_MOVE (1 << 12)
#define ADDED_MOVE          (1 << 13)
#define NULL_MOVE           (1 << 14)

#define VirtualMoveType unsigned long

#define HAS_MOVE_TYPE_OF(checked, type) ((checked & type) != 0)
#define ADD_MOVE_TYPE(added, type) (added |= type)

typedef enum
{
    StatusNil,
    StatusCheckmated,
    StatusStalemate,
    //StatusRepetition,
    StatusFiftyMove,
    StatusInsufficient,
} Status;

#endif
