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
  }

  override func tearDownWithError() throws {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
  }

  func testCoordinateToFileRank() {
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B1)) == FileRank(file: 1, rank: 0))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B3)) == FileRank(file: 1, rank: 2))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.C3)) == FileRank(file: 2, rank: 2))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D2)) == FileRank(file: 3, rank: 1))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D3)) == FileRank(file: 3, rank: 2))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D8)) == FileRank(file: 3, rank: 7))
    XCTAssertTrue(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.H4)) == FileRank(file: 7, rank: 3))
  }

  func testFileRankToCoordinate() {
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 0) == .B1)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 2) == .B3)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 2, rank: 2) == .C3)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 1) == .D2)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 2) == .D3)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 7) == .D8)
    XCTAssertTrue(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 7, rank: 3) == .H4)
  }

  func testMoveLegality() async {
    await bridge.connectToEngine()
    // Test 1.Nc3 from the starting position (Expected: True)
    let result1 = await bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .C3)
    XCTAssertTrue(result1, "1.Nc3 should be legal")

    // Test 1.Nb3 from the starting position (Expected: False)
    let result2 = await bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .B1, to: .B3)
    XCTAssertTrue(result2 == false, "1.Nb3 should be illegal")

    // Test 1.d4 from the starting position (Expected: True)
    let result3 = await bridge.isMoveLegal("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", from: .D2, to: .D3)
    XCTAssertTrue(result3, "1.d4 should be legal")

    // Test 1...Qh4 illegal move (Expected: False)
    let result4 = await bridge.isMoveLegal("rnbqkb1r/pppp1ppp/4pn2/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4)
    XCTAssertTrue(result4 == false, "1...Qh4 should be illegal in this position")

    // Test 1...Qh4 legal move (Expected: True)
    let result5 = await bridge.isMoveLegal("rnbqkbnr/pppp1ppp/4p3/8/6P1/8/PPPPPP1P/RNBQKBNR b KQkq - 0 1", from: .D8, to: .H4)
    XCTAssertTrue(result5, "1...Qh4 should be legal in this position")
  }

  func testGameStatus() async {
    await bridge.connectToEngine()
    // Starting position should be normal (Expected: 0)
    if let result1 = await bridge.getGameStatus("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
      XCTAssertTrue(result1 == GameStatus.normal, "Game status should be normal")
    } else {
      XCTFail("Status should not be nil")
    }

    // Checkmated position (Expected: 1)
    if let result2 = await bridge.getGameStatus("4R1k1/5ppp/8/8/8/8/5PPP/6K1 b - - 0 1") {
      XCTAssertTrue(result2 == GameStatus.checkmated, "Game status should be checkmate")
    } else {
      XCTFail("Status should not be nil")
    }

    // Stalemate position (Expected: 2)
    if let result3 = await bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 b - - 0 1") {
      XCTAssertTrue(result3 == GameStatus.stalemate, "Game status should be stalemate")
    } else {
      XCTFail("Status should not be nil")
    }

    // Normal position (Expected: 0)
    if let result4 = await bridge.getGameStatus("6k1/6P1/6KP/8/8/8/8/8 w - - 0 1") {
      XCTAssertTrue(result4 == GameStatus.normal, "Game status should be normal")
    } else {
      XCTFail("Status should not be nil")
    }
  }
}
