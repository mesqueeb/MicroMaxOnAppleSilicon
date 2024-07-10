@testable import MicroMaxOnAppleSilicon
import Testing

struct FileRank: Equatable {
  let file: Int
  let rank: Int

  init(file: Int, rank: Int) {
    self.file = file
    self.rank = rank
  }

  init(_ payload: (file: Int, rank: Int)?) {
    if let payload {
      self.file = payload.file
      self.rank = payload.rank
    } else {
      self.file = -1
      self.rank = -1
    }
  }
}

final class Tests {
  var bridge: MicroMaxBridge
  
  init() async throws {
    self.bridge = MicroMaxBridge()
    bridge.connectToEngine()
  }
  
  deinit {}
  
  @Test func coordinateToFileRank() throws {
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B1)) == FileRank(file: 1, rank: 0))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B3)) == FileRank(file: 1, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.C3)) == FileRank(file: 2, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D2)) == FileRank(file: 3, rank: 1))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D3)) == FileRank(file: 3, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D8)) == FileRank(file: 3, rank: 7))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.H4)) == FileRank(file: 7, rank: 3))
  }
  
  @Test func fileRankToCoordinate() throws {
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 0) == .B1)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 2) == .B3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 2, rank: 2) == .C3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 1) == .D2)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 2) == .D3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 7) == .D8)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 7, rank: 3) == .H4)
  }
  
  @Test func moveLegality() throws {
    // Test 1.Nc3 from the starting position (Expected: True)
    #expect(bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .C3), "1.Nc3 should be legal")
  
    // Test 1.Nb3 from the starting position (Expected: False)
    #expect(!bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .B3), "1.Nb3 should be illegal")
  
    // Test 1.d4 from the starting position (Expected: True)
    #expect(bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .D2, to: .D3), "1.d4 should be legal")
  
    // Test 1...Qh4 illegal move (Expected: False)
    #expect(!bridge.isMoveLegal("rnbqkb1r/pppp1ppp/4pn2/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4), "1...Qh4 should be illegal in this position")
  
    // Test 1...Qh4 legal move (Expected: True)
    #expect(bridge.isMoveLegal("rnbqkbnr/pppp1ppp/4p3/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4), "1...Qh4 should be legal in this position")
  }
  
  @Test func gameStatus() throws {
    // Starting position should be normal (Expected: 0)
    #expect(bridge.getGameStatus("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == GameStatus.normal, "Starting position should be normal")
    
    // Checkmated position (Expected: 1)
    #expect(bridge.getGameStatus("4R1k1/5ppp/8/8/8/8/5PPP/6K1 b - - 0 1") == GameStatus.checkmated, "Position should be checkmate")
    
    // Stalemate position (Expected: 2)
    #expect(bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 b - - 0 1") == GameStatus.stalemate, "Position should be stalemate")
    
    // Normal position (Expected: 0)
    #expect(bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 w - - 0 1") == GameStatus.normal, "Position should be normal")
  }
}
