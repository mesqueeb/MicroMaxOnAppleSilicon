@testable import MicroMaxOnAppleSilicon
import XCTest

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

class MicroMaxOnAppleSiliconTests: XCTestCase {
  var bridge: MicroMaxBridge!

  override func setUpWithError() throws {
    // Put setup code here. This method is called before the invocation of each test method in the class.
    bridge = MicroMaxBridge()
    bridge.connectToEngine()
  }

  override func tearDownWithError() throws {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
  }

  func testCoordinateToFileRank() {
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.B1)), FileRank(file: 1, rank: 0))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.B3)), FileRank(file: 1, rank: 2))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.C3)), FileRank(file: 2, rank: 2))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.D2)), FileRank(file: 3, rank: 1))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.D3)), FileRank(file: 3, rank: 2))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.D8)), FileRank(file: 3, rank: 7))
    XCTAssertEqual(FileRank(bridge.coordinateToFileRank(.H4)), FileRank(file: 7, rank: 3))
  }

  func testFileRankToCoordinate() {
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 1, rank: 0), .B1)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 1, rank: 2), .B3)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 2, rank: 2), .C3)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 3, rank: 1), .D2)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 3, rank: 2), .D3)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 3, rank: 7), .D8)
    XCTAssertEqual(bridge.fileRankToCoordinate(file: 7, rank: 3), .H4)
  }

  func testMoveLegality() {
    // Test 1.Nc3 from the starting position (Expected: True)
    XCTAssertTrue(bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .C3), "1.Nc3 should be legal")

    // Test 1.Nb3 from the starting position (Expected: False)
    XCTAssertFalse(bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .B3), "1.Nb3 should be illegal")

    // Test 1.d4 from the starting position (Expected: True)
    XCTAssertTrue(bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .D2, to: .D3), "1.d4 should be legal")

    // Test 1...Qh4 illegal move (Expected: False)
    XCTAssertFalse(bridge.isMoveLegal("rnbqkb1r/pppp1ppp/4pn2/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4), "1...Qh4 should be illegal in this position")

    // Test 1...Qh4 legal move (Expected: True)
    XCTAssertTrue(bridge.isMoveLegal("rnbqkbnr/pppp1ppp/4p3/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4), "1...Qh4 should be legal in this position")
  }

  func testGameStatus() {
    // Starting position should be normal (Expected: 0)
    XCTAssertEqual(bridge.getGameStatus("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), GameStatus.normal, "Starting position should be normal")

    // Checkmated position (Expected: 1)
    XCTAssertEqual(bridge.getGameStatus("4R1k1/5ppp/8/8/8/8/5PPP/6K1 b - - 0 1"), GameStatus.checkmated, "Position should be checkmate")

    // Stalemate position (Expected: 2)
    XCTAssertEqual(bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 b - - 0 1"), GameStatus.stalemate, "Position should be stalemate")

    // Normal position (Expected: 0)
    XCTAssertEqual(bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 w - - 0 1"), GameStatus.normal, "Position should be normal")
  }
}
