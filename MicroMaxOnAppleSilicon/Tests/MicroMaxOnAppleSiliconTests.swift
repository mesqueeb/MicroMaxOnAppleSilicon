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
    bridge = MicroMaxBridge()
  }

  override func tearDownWithError() throws {
    // Cleanup handled in individual tests
  }

  // MARK: - Coordinate Tests

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

  // MARK: - FEN Parser Tests

  func testFENParserStartingPosition() {
    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    guard let result = FENParser.parse(fen) else {
      XCTFail("Failed to parse starting position FEN")
      return
    }

    XCTAssertEqual(result.activeColor, "w", "Active color should be white")
    XCTAssertEqual(result.pieces.count, 32, "Starting position should have 32 pieces")

    // Check some specific pieces
    let whitePawns = result.pieces.filter { $0.piece == "P" }
    XCTAssertEqual(whitePawns.count, 8, "Should have 8 white pawns")

    let blackPawns = result.pieces.filter { $0.piece == "p" }
    XCTAssertEqual(blackPawns.count, 8, "Should have 8 black pawns")

    // Check white king at e1 (file 4, rank 0)
    let whiteKing = result.pieces.first { $0.piece == "K" }
    XCTAssertNotNil(whiteKing)
    XCTAssertEqual(whiteKing?.file, 4, "White king should be on file e (4)")
    XCTAssertEqual(whiteKing?.rank, 0, "White king should be on rank 1 (0)")

    // Check black king at e8 (file 4, rank 7)
    let blackKing = result.pieces.first { $0.piece == "k" }
    XCTAssertNotNil(blackKing)
    XCTAssertEqual(blackKing?.file, 4, "Black king should be on file e (4)")
    XCTAssertEqual(blackKing?.rank, 7, "Black king should be on rank 8 (7)")
  }

  func testFENParserMidGamePosition() {
    // Position after 1.e4 e5 2.Nf3
    let fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
    guard let result = FENParser.parse(fen) else {
      XCTFail("Failed to parse mid-game FEN")
      return
    }

    XCTAssertEqual(result.activeColor, "b", "Active color should be black")

    // Check knight on f3 (file 5, rank 2)
    let whiteKnight = result.pieces.first { $0.piece == "N" && $0.file == 5 && $0.rank == 2 }
    XCTAssertNotNil(whiteKnight, "White knight should be on f3")

    // Check pawn on e4 (file 4, rank 3)
    let whitePawn = result.pieces.first { $0.piece == "P" && $0.file == 4 && $0.rank == 3 }
    XCTAssertNotNil(whitePawn, "White pawn should be on e4")

    // Check pawn on e5 (file 4, rank 4)
    let blackPawn = result.pieces.first { $0.piece == "p" && $0.file == 4 && $0.rank == 4 }
    XCTAssertNotNil(blackPawn, "Black pawn should be on e5")
  }

  func testFENParserInvalidFEN() {
    // Test various invalid FEN strings
    XCTAssertNil(FENParser.parse("invalid"), "FEN without space should be invalid")
    XCTAssertNil(FENParser.parse("invalid fen"), "FEN with invalid characters should be invalid")
    XCTAssertNil(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1"), "Invalid active color should be invalid")
    XCTAssertNil(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"), "FEN with only 7 ranks should be invalid")
    XCTAssertNil(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/EXTRA w KQkq - 0 1"), "FEN with 9 ranks should be invalid")
  }

  // MARK: - Engine Tests

  func testStartEngineReturnsInitBanners() async throws {
    let initOutput = try await bridge.startEngine()

    XCTAssertNotNil(initOutput, "Engine should return init banners")
    XCTAssertTrue(initOutput?.contains("Fairy-Max") == true, "Init output should contain Fairy-Max")

    let isRunning = await bridge.engineRunning
    XCTAssertTrue(isRunning, "Engine should be running after start")

    await bridge.stopEngine()
  }

  func testSendCommandNoResponse() async throws {
    _ = try await bridge.startEngine()

    let response = await bridge.sendCommand("new")
    XCTAssertNil(response, "'new' command should return nil (no response)")

    await bridge.stopEngine()
  }

  func testSendCommandWithResponse() async throws {
    _ = try await bridge.startEngine()

    _ = await bridge.sendCommand("new")
    _ = await bridge.sendCommand("force")
    _ = await bridge.sendCommand("st 1")

    let response = await bridge.sendCommand("go")
    XCTAssertNotNil(response, "'go' command should return a response")
    XCTAssertTrue(response?.starts(with: "move ") == true, "Response should be a move like 'move e2e4'")

    await bridge.stopEngine()
  }

  func testEngineStartStop() async throws {
    _ = try await bridge.startEngine()

    let isRunning = await bridge.engineRunning
    XCTAssertTrue(isRunning, "Engine should be running after start")

    await bridge.stopEngine()

    let isStoppedNow = await bridge.engineRunning
    XCTAssertFalse(isStoppedNow, "Engine should not be running after stop")
  }

  // MARK: - requestAiMove Tests

  func testRequestAiMoveStartingPosition() async throws {
    _ = try await bridge.startEngine()

    // Starting position - white to move
    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    let move = try await bridge.requestAiMove(fenState: fen, thinkTime: 1)

    XCTAssertNotNil(move.from, "Move should have a source square")
    XCTAssertNotNil(move.to, "Move should have a destination square")

    // Verify it's a valid starting move (piece from rank 1 or 2)
    if let from = move.from, let fromRank = coordinateToFileRank(from)?.rank {
      XCTAssertTrue(fromRank == 0 || fromRank == 1, "White's first move should be from rank 1 or 2")
    }

    await bridge.stopEngine()
  }

  func testRequestAiMoveMidGame() async throws {
    _ = try await bridge.startEngine()

    // Position after 1.e4 - black to move
    let fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    let move = try await bridge.requestAiMove(fenState: fen, thinkTime: 1)

    XCTAssertNotNil(move.from, "Move should have a source square")
    XCTAssertNotNil(move.to, "Move should have a destination square")

    // Verify it's a valid black move (piece from rank 6 or 7)
    if let from = move.from, let fromRank = coordinateToFileRank(from)?.rank {
      XCTAssertTrue(fromRank >= 6, "Black's move should be from rank 7 or 8 (index 6 or 7)")
    }

    await bridge.stopEngine()
  }

  func testRequestAiMoveNotRunning() async {
    // Don't start the engine
    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    do {
      _ = try await bridge.requestAiMove(fenState: fen)
      XCTFail("Should throw engineNotRunning error")
    } catch let error as MicroMaxError {
      XCTAssertEqual(error, .engineNotRunning)
    } catch {
      XCTFail("Wrong error type: \(error)")
    }
  }

  func testRequestAiMoveInvalidFEN() async throws {
    _ = try await bridge.startEngine()

    do {
      // "invalid fen" should be invalid (invalid characters, not 8 ranks, invalid color)
      _ = try await bridge.requestAiMove(fenState: "invalid fen")
      XCTFail("Should throw invalidFEN error")
    } catch let error as MicroMaxError {
      XCTAssertEqual(error, .invalidFEN)
    } catch {
      XCTFail("Wrong error type: \(error)")
    }

    await bridge.stopEngine()
  }
}
