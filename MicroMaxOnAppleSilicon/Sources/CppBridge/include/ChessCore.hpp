#ifndef CHESS_CORE_HPP
#define CHESS_CORE_HPP

#include "../Types.hpp"
#include "../Validation.hpp"
#include "../VirtualBoard.hpp"
#include "../VirtualState.hpp"
#include "../Protocol.hpp"
#include "../VirtualMovement.hpp"
#include <vector>
#include <string>
//#include "ChessMoveDecoder.hpp"

struct ChessCoreImpl;

#define IS_PROMOTION(piece, srcRank, dstRank) ((piece == WhitePawn && (srcRank == 6 && dstRank == 7)) || (piece == BlackPawn && (srcRank == 1 && dstRank == 0)))

namespace Framework
{
	class ChessCore : public VirtualBoard
	{
		public:
			ChessCore(Protocol *protocol = NULL);
			~ChessCore();

			// Validate the position ignoring castling and side-to-move
			static Validation validate(const std::string &);
    
			std::string convertVarToStr(std::size_t index, std::vector<std::string> moves, bool isWhiteTurn);

			void open() override;

			void start() override;
			void start(const std::string &) override;

			void close() override;

			bool add(const VirtualSquare &, const VirtualPiece &) override;

			bool play(const VirtualSquare &,
                      const VirtualSquare &,
                      const VirtualPiece  & = NilPiece,
                      VirtualMove *move = NULL) override;

			Status getStatus() const override;

			void *getInternal() const override;
    
			bool canPlay(const VirtualSquare &,
                         const VirtualSquare &,
                         const VirtualPiece  &,
                         VirtualMove * move = nullptr) const override;

			VirtualPiece getPiece(const VirtualSquare &square) const override;

			// Generate all legal moves in the current position
			void getMoves(std::vector<VirtualMove> &, bool extended) const override;

			// Generate all legal moves for a square
			void getMoves(const VirtualSquare &, std::vector<VirtualMove> &, bool extended) const override;

			VirtualSquare find(VirtualPiece piece) override;

            unsigned countPieces() const override;

            void captured(unsigned &wq,
                          unsigned &wr,
                          unsigned &wb,
                          unsigned &wn,
                          unsigned &wp,
                          unsigned &bq,
                          unsigned &br,
                          unsigned &bb,
                          unsigned &bn,
                          unsigned &bp) const override;

		protected:
			void update(const std::string &) override;

		private:
            bool _hasCachedEqual;

			// A method to share common code in the constructors
			void init(Protocol *protocol);
    
			int generateToken(const VirtualSquare &,
                              const VirtualSquare &,
                              const VirtualPiece  &,
                              VirtualMove *move) const;

			std::string generateNotation(const VirtualMovement &,
                                         const VirtualPiece    &,
                                         const VirtualMoveType &) const;

            ChessCoreImpl *_pimpl;
	};
}

#endif
