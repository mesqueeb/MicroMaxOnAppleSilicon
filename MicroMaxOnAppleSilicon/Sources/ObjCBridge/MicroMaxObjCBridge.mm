#import "include/MicroMaxObjCBridge.h"
#include "Resources.hpp"
#include "ChessCore.hpp"
#include "ProtocolReceiver.hpp"

MicroMaxObjCBridge *_bridge;

struct MicroMaxBridgeReceiver : Framework::ProtocolReceiver
{
    void receiveMove(int srcFile,
                     int srcRank,
                     int dstFile,
                     int dstRank,
                     int promote)
    {
        _bridge.receiveMove(srcFile, srcRank, dstFile, dstRank, promote);
    }
};

@implementation MicroMaxObjCBridge
{
    Framework::Resources *_resources;
  MicroMaxBridgeReceiver _receiver;
}

-(id) init
{
    if (self = [super init])
    {
        _resources = Framework::Resources::create();
        _bridge = self;
        return self;
    }
    
    return nil;
}

-(bool) isMoveLegal:(NSString *)fen
            srcFile:(int)srcFile
            srcRank:(int)srcRank
            dstFile:(int)dstFile
            dstRank:(int)dstRank
{
    Framework::ChessCore chess;
    chess.open();
    chess.start([fen UTF8String]);
    
    VirtualSquare src((VirtualFile) srcFile, (VirtualRank) srcRank);
    VirtualSquare dst((VirtualFile) dstFile, (VirtualRank) dstRank);
    
    return chess.canPlay(src, dst, AnyQueen);
}

-(int) getGameStatus:(NSString *)fen
{
    Framework::ChessCore chess;
    chess.open();
    chess.start([fen UTF8String]);

    switch (chess.getStatus())
    {
        case StatusNil: { return 0; }
        case StatusCheckmated: { return 1; }
        case StatusStalemate: { return 2; }
        case StatusFiftyMove: { return 3; }
        case StatusInsufficient: { return 4; }
    }
    
    return 0;
}

-(void) connectToEngine
{
    assert(_resources);
    _resources->open();
    _resources->protocol->attach(&_receiver);
}

-(void) request:(NSString *)fen
{
    _resources->protocol->start([fen UTF8String]);
    _resources->protocol->think(2);
}

@end
