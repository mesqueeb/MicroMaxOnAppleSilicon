#if !defined(MOVEGEN_H_INCLUDED)
#define MOVEGEN_H_INCLUDED

#include "BMove.h"
#include "BPosition.h"

namespace BBoard
{

enum MoveType {
  MV_CAPTURE,
  MV_NON_CAPTURE,
  MV_CHECK,
  MV_NON_CAPTURE_CHECK,
  MV_EVASION,
  MV_NON_EVASION,
  MV_LEGAL,
  MV_PSEUDO_LEGAL
};

template<MoveType>
MoveStack* generate(const Position& pos, MoveStack* mlist);

}

#endif // !defined(MOVEGEN_H_INCLUDED)
