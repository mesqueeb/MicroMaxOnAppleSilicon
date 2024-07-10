#ifndef NODE_HPP
#define NODE_HPP

#include "VirtualMove.hpp"

namespace Framework
{
    class VirtualNode
    {
        public:
            VirtualNode(const VirtualMove& move, const std::string state)
                : move(move), state(state), prevVar(NULL), nextVar(NULL), prevNode(NULL), nextNode(NULL) {}

            VirtualNode(const VirtualNode &node) : state(node.state), move(node.move) {}

            // State of the node after the move played
            const std::string state;

            // The move representing the node
            const VirtualMove move;

            VirtualNode *prevVar;
            VirtualNode *nextVar;
            VirtualNode *nextNode;
            VirtualNode *prevNode;
        };
}

#endif
