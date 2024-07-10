#ifndef UNIX_ADAPTER_HPP
#define UNIX_ADAPTER_HPP

#include "UnixListener.hpp"
#include "AbstractAdapter.hpp"
#include "PThreadStaticLinker.hpp"

namespace Framework
{
    class UnixAdapter : public AbstractAdapter
    {
        public:
            UnixAdapter() : _listener(NULL) {}
    
            virtual ~UnixAdapter()
            {
                if (_listener)
                {
                    _listener->close();
                }
            }
    
            virtual Linker *createLinker()
            {
#ifdef SYZYGY
                return nullptr;
#else
                return new PThreadStaticLinker("SmallChess");
#endif
            }
    
            virtual void createListener()
            {
                if (!_listener)
                {
                    _listener = new UnixListener();
                    _listener->open(this);
                }
            }
    
        private:
            UnixListener *_listener;
};
}

#endif
