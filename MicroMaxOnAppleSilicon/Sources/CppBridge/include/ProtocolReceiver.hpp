#ifndef PROTOCOL_RECEIVER_HPP
#define PROTOCOL_RECEIVER_HPP

#include "../Types.hpp"

namespace Framework
{
    struct ProtocolReceiver
    {
        virtual void receiveMove(int srcFile,
                                 int srcRank,
                                 int dstFile,
                                 int dstRank,
                                 int promote) = 0;
    };
}

#endif
