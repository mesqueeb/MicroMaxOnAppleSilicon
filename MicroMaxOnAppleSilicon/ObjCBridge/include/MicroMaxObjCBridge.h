#import <Foundation/Foundation.h>

// Define a block type that matches the signature of your Swift function
typedef void (^SwiftFunctionBlock)(int, int, int, int, int);

@interface MicroMaxObjCBridge : NSObject

// Property to hold the Swift function block
@property (nonatomic, copy) SwiftFunctionBlock receiveMove;

// Establish a connection to the engine layer
-(void) connectToEngine;

// Send a request to an engine
-(void) request:(NSString *)fen;

// Is the move legal?
-(bool) isMoveLegal:(NSString *)fen
            srcFile:(int)srcFile
            srcRank:(int)srcRank
            dstFile:(int)dstFile
            dstRank:(int)dstRank;

// What's the game status?
-(int) getGameStatus:(NSString *)fen;

@end
