#ifndef WB_PROTOCOL_HPP
#define WB_PROTOCOL_HPP

#include <map>
#include <string>
#include "WBProtocolParser.hpp"
#include "AbstractProtocol.hpp"
#include "AdapterReceiver.hpp"
#include "VirtualAtomFactory.hpp"

namespace Framework
{
	class WBProtocol : public AbstractProtocol, public AdapterReceiver
	{
        enum WBProtocolState
        {
            ReadyState, IgnoreUpdateState, WaitingForMoveState,
        };

		public:
			WBProtocol(Adapter *adapter) : AbstractProtocol(adapter), _ignoreUpdate(false) {}
			~WBProtocol() { close(); }

			inline bool isReady() { return true; }
    
			// Inherited from Protocol
			void open();

			// Inherited from Protocol
			void close();

			// Inherited from Protocol
			void start();

			// Inherited from Protocol
            void start(const std::string& state);

			// Inherited from Protocol
			void think(unsigned duration);

			// Inherited from Protocol
            void update(const std::string& state);

			// Inherited from Protocol
			void update(unsigned srcFile, unsigned srcRank, unsigned dstFile, unsigned dstRank, unsigned promote);

			// Inherited from AdapterReceiver
			void process(const std::string& message);

			// Inherited from Protocol
			inline void analyze()
            {
                // Empty Implementation
            }

			// Inherited from Protocol
			inline void stop()
            {
                // Empty Implementation
            }

			// Inherited from Protocol
            inline bool isAnalyzing() const { return false; }

			// Inherited from Protocol
            inline bool isThinking() const { return false; }

		private:
			Context _context;
            VirtualAtom _atom;
            WBProtocolState _state;
			WBProtocolParser _parser;        
            VirtualAtomFactory _atomFactory;

            // This is true if we want to skip the next update(std::string...) because Winboard is a stated protocol
            bool _ignoreUpdate;

            // Implement the Winboard edit command
            void edit(const std::map<std::pair<unsigned, unsigned>, char> &placement, const std::string &state);
	};
}

#endif
