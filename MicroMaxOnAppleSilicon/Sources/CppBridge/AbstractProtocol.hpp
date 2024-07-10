#ifndef ABSTRACT_PROTOCOL_PROVIDER_HPP
#define ABSTRACT_PROTOCOL_PROVIDER_HPP

#include <set>
#include "Adapter.hpp"
#include "Protocol.hpp"
#include "BoardConverter.hpp"
#include "./include/ProtocolReceiver.hpp"

namespace Framework
{
	class AbstractProtocol : public Protocol
	{
		public:
			AbstractProtocol(Adapter *adapter) : _adapter(adapter) {}

			virtual ~AbstractProtocol() { close(); };

			inline void attach(ProtocolReceiver *receiver) { _receivers.insert(receiver); }

            inline void detach() { _receivers.clear(); }
            inline void detach(ProtocolReceiver *receiver) { _receivers.erase(receiver); }

			// Inherited from Protocol
			virtual void close()
			{
				_receivers.clear();
			}
        
			static void convertCoordToStr(char buffer[], unsigned int srcFile, unsigned int srcRank, unsigned int dstFile, unsigned int dstRank, unsigned promote)
			{       
				sprintf(buffer, "%c%d%c%d", CONVERT_FILE_TO_CHAR(srcFile), srcRank + 1, CONVERT_FILE_TO_CHAR(dstFile), dstRank + 1);
				buffer[4] = promote ? promote : '\0';
				buffer[5] = '\0';
			}

			void receiveMove(const std::string& bestmove, const std::string &ponder)
			{
                for (auto& receiver : _receivers)
                {
                    const auto length = bestmove.length();
                    
                    if (length >= 4 && length <= 5)
                    {
                        receiver->receiveMove(BoardConverter::convert(bestmove[0]),
                                              BoardConverter::convert(bestmove[1]),
                                              BoardConverter::convert(bestmove[2]),
                                              BoardConverter::convert(bestmove[3]),
                                               (length == 5) ? bestmove[4] : 0);
                    }
                }
			}

		protected:
			Adapter *_adapter;
            std::set<ProtocolReceiver *> _receivers;
	};
}

#endif
