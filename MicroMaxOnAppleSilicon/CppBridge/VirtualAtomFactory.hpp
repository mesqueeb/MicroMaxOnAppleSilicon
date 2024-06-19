#ifndef ATOM_FACTORY_HPP
#define ATOM_FACTORY_HPP

#include "VirtualAtom.hpp"

#define INVALID_ATOM 0

namespace Framework
{
    class VirtualAtomFactory
    {
        public:
            VirtualAtom getNext()
			{
				return (_nextID == 0) ? (_nextID = 1) : ++_nextID;
			}

            VirtualAtom getCurrent() const
            {
                return _nextID;
            }

            bool isCurrent(const VirtualAtom &atom) const
            {
                return (atom == _nextID);
            }

        private:
            VirtualAtom _nextID;
    };
}

#endif
