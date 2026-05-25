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

@Suite struct CoordinateTests {
  @Test func coordinateToFileRank() {
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B1)) == FileRank(file: 1, rank: 0))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.B3)) == FileRank(file: 1, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.C3)) == FileRank(file: 2, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D2)) == FileRank(file: 3, rank: 1))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D3)) == FileRank(file: 3, rank: 2))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.D8)) == FileRank(file: 3, rank: 7))
    #expect(FileRank(MicroMaxOnAppleSilicon.coordinateToFileRank(.H4)) == FileRank(file: 7, rank: 3))
  }

  @Test func fileRankToCoordinate() {
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 0) == .B1)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 1, rank: 2) == .B3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 2, rank: 2) == .C3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 1) == .D2)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 2) == .D3)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 3, rank: 7) == .D8)
    #expect(MicroMaxOnAppleSilicon.fileRankToCoordinate(file: 7, rank: 3) == .H4)
  }
}

@Suite struct FENParserTests {
  @Test func startingPosition() throws {
    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    let result = try #require(FENParser.parse(fen), "Failed to parse starting position FEN")

    #expect(result.activeColor == "w", "Active color should be white")
    #expect(result.pieces.count == 32, "Starting position should have 32 pieces")

    let whitePawns = result.pieces.filter { $0.piece == "P" }
    #expect(whitePawns.count == 8, "Should have 8 white pawns")

    let blackPawns = result.pieces.filter { $0.piece == "p" }
    #expect(blackPawns.count == 8, "Should have 8 black pawns")

    let whiteKing = try #require(result.pieces.first { $0.piece == "K" })
    #expect(whiteKing.file == 4, "White king should be on file e (4)")
    #expect(whiteKing.rank == 0, "White king should be on rank 1 (0)")

    let blackKing = try #require(result.pieces.first { $0.piece == "k" })
    #expect(blackKing.file == 4, "Black king should be on file e (4)")
    #expect(blackKing.rank == 7, "Black king should be on rank 8 (7)")
  }

  @Test func midGamePosition() throws {
    // Position after 1.e4 e5 2.Nf3
    let fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
    let result = try #require(FENParser.parse(fen), "Failed to parse mid-game FEN")

    #expect(result.activeColor == "b", "Active color should be black")

    #expect(result.pieces.contains { $0.piece == "N" && $0.file == 5 && $0.rank == 2 }, "White knight should be on f3")
    #expect(result.pieces.contains { $0.piece == "P" && $0.file == 4 && $0.rank == 3 }, "White pawn should be on e4")
    #expect(result.pieces.contains { $0.piece == "p" && $0.file == 4 && $0.rank == 4 }, "Black pawn should be on e5")
  }

  @Test func invalidFEN() {
    #expect(FENParser.parse("invalid") == nil, "FEN without space should be invalid")
    #expect(FENParser.parse("invalid fen") == nil, "FEN with invalid characters should be invalid")
    #expect(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1") == nil, "Invalid active color should be invalid")
    #expect(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1") == nil, "FEN with only 7 ranks should be invalid")
    #expect(FENParser.parse("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/EXTRA w KQkq - 0 1") == nil, "FEN with 9 ranks should be invalid")
  }
}

@Suite struct EngineTests {
  @Test func startEngineReturnsInitBanners() async throws {
    let bridge = MicroMaxBridge()
    let initOutput = try await bridge.startEngine()

    #expect(initOutput != nil, "Engine should return init banners")
    #expect(initOutput?.contains("Fairy-Max") == true, "Init output should contain Fairy-Max")

    let isRunning = await bridge.engineRunning
    #expect(isRunning, "Engine should be running after start")

    await bridge.stopEngine()
  }

  @Test func sendCommandNoResponse() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    let response = await bridge.sendCommand("new")
    #expect(response == nil, "'new' command should return nil (no response)")

    await bridge.stopEngine()
  }

  @Test func sendCommandWithResponse() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    _ = await bridge.sendCommand("new")
    _ = await bridge.sendCommand("force")
    _ = await bridge.sendCommand("st 1")

    let response = await bridge.sendCommand("go")
    #expect(response != nil, "'go' command should return a response")
    #expect(response?.starts(with: "move ") == true, "Response should be a move like 'move e2e4'")

    await bridge.stopEngine()
  }

  @Test func engineStartStop() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    let isRunning = await bridge.engineRunning
    #expect(isRunning, "Engine should be running after start")

    await bridge.stopEngine()

    let isStoppedNow = await bridge.engineRunning
    #expect(isStoppedNow == false, "Engine should not be running after stop")
  }
}

@Suite struct RequestAiMoveTests {
  @Test func startingPosition() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    let move = try await bridge.requestAiMove(fenState: fen, thinkTime: 1)

    #expect(move.from != nil, "Move should have a source square")
    #expect(move.to != nil, "Move should have a destination square")

    if let from = move.from, let fromRank = coordinateToFileRank(from)?.rank {
      #expect(fromRank == 0 || fromRank == 1, "White's first move should be from rank 1 or 2")
    }

    await bridge.stopEngine()
  }

  @Test func midGame() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    // Position after 1.e4 - black to move
    let fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
    let move = try await bridge.requestAiMove(fenState: fen, thinkTime: 1)

    #expect(move.from != nil, "Move should have a source square")
    #expect(move.to != nil, "Move should have a destination square")

    if let from = move.from, let fromRank = coordinateToFileRank(from)?.rank {
      #expect(fromRank >= 6, "Black's move should be from rank 7 or 8 (index 6 or 7)")
    }

    await bridge.stopEngine()
  }

  @Test func notRunning() async {
    let bridge = MicroMaxBridge()
    let fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    await #expect(throws: MicroMaxError.engineNotRunning) {
      _ = try await bridge.requestAiMove(fenState: fen)
    }
  }

  @Test func invalidFEN() async throws {
    let bridge = MicroMaxBridge()
    _ = try await bridge.startEngine()

    await #expect(throws: MicroMaxError.invalidFEN) {
      _ = try await bridge.requestAiMove(fenState: "invalid fen")
    }

    await bridge.stopEngine()
  }
}
