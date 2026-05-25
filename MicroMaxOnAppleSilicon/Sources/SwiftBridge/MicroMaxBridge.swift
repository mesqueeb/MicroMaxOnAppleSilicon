import Foundation
import MicroMaxCBridge

public actor MicroMaxBridge {
  private var isRunning = false

  public init() {}

  /// Start the MicroMax chess engine.
  /// The engine uses WinBoard protocol for communication.
  /// - Returns: The engine's initialization output (banners like "Fairy-Max 4.80"), or nil if empty
  public func startEngine() async throws -> String? {
    guard !isRunning else { return nil }

    // Find the fmax.ini file in the bundle
    guard let iniPath = Bundle.module.path(forResource: "fmax", ofType: "ini") else {
      throw MicroMaxError.iniFileNotFound
    }

    // Run on background thread since C code blocks
    let initOutput = await Task.detached { () -> String in
      guard let result = micromax_engine_start(iniPath) else {
        return ""
      }
      return String(cString: result)
    }.value

    isRunning = true
    return initOutput.isEmpty ? nil : initOutput
  }

  /// Stop the MicroMax chess engine.
  public func stopEngine() {
    guard isRunning else { return }
    micromax_engine_stop()
    isRunning = false
  }

  /// Send a WinBoard protocol command to the engine and get the response.
  /// This method blocks until the engine has processed the command.
  ///
  /// Common commands:
  /// - `xboard` - Initialize WinBoard protocol
  /// - `new` - Start a new game
  /// - `force` - Put engine in force mode (doesn't play automatically)
  /// - `white` / `black` - Set which side the engine plays
  /// - `st <seconds>` - Set time limit for thinking
  /// - `go` - Tell engine to start thinking and make a move
  /// - `edit` - Enter edit mode for setting up positions
  /// - `quit` - Quit the engine
  ///
  /// - Parameter command: The WinBoard command to send
  /// - Returns: The engine's response, or nil if no response
  public func sendCommand(_ command: String) async -> String? {
    guard isRunning else { return nil }

    // Run on background thread since C code blocks until engine responds
    let response = await Task.detached { () -> String in
      guard let result = micromax_engine_send_command(command) else {
        return ""
      }
      return String(cString: result)
    }.value

    return response.isEmpty ? nil : response
  }

  /// Check if the engine is currently running.
  public var engineRunning: Bool {
    isRunning
  }

  // MARK: - Position Setup

  /// Set up a chess position from a FEN string using WinBoard edit mode
  /// - Parameter fen: FEN string describing the position
  private func setupPositionFromFEN(_ fen: String) async throws {
    guard let parsed = FENParser.parse(fen) else {
      throw MicroMaxError.invalidFEN
    }

    // Start fresh game
    _ = await sendCommand("new")

    // Set which side is to move
    _ = await sendCommand(parsed.activeColor == "w" ? "white" : "black")

    _ = await sendCommand("force")
    _ = await sendCommand("edit")
    _ = await sendCommand("#")  // Clear the board

    // Place white pieces (uppercase in FEN)
    for (file, rank, piece) in parsed.pieces where piece.isUppercase {
      let square = "\(Character(UnicodeScalar(97 + file)!))\(rank + 1)"
      _ = await sendCommand("\(piece)\(square)")
    }

    // Switch to black pieces
    _ = await sendCommand("c")

    // Place black pieces (lowercase in FEN, but send as uppercase to WinBoard)
    for (file, rank, piece) in parsed.pieces where piece.isLowercase {
      let square = "\(Character(UnicodeScalar(97 + file)!))\(rank + 1)"
      _ = await sendCommand("\(piece.uppercased())\(square)")
    }

    // End edit mode
    _ = await sendCommand(".")
    _ = await sendCommand("force")
  }

  // MARK: - AI Move Request

  /// Request an AI move for the given position.
  /// - Parameters:
  ///   - fenState: FEN string describing the current position
  ///   - thinkTime: Time in seconds for the engine to think (default: 3)
  /// - Returns: The move as (from, to) coordinates
  /// - Throws: MicroMaxError if engine not running, invalid FEN, or no move returned
  public func requestAiMove(fenState: String, thinkTime: Int = 3) async throws -> MoveResult {
    guard isRunning else {
      throw MicroMaxError.engineNotRunning
    }

    // Set up the position from FEN
    try await setupPositionFromFEN(fenState)

    // Set thinking time
    _ = await sendCommand("st \(thinkTime)")

    // Ask engine to think and make a move
    guard let response = await sendCommand("go") else {
      throw MicroMaxError.noMoveReturned
    }

    // Parse the move from response
    guard let move = parseMoveResponse(response) else {
      throw MicroMaxError.noMoveReturned
    }

    return move
  }

  // MARK: - Move Response Parsing

  /// Parse a move response from the engine
  /// - Parameter response: Engine response like "move e2e4" or multiline with "move e2e4"
  /// - Returns: MoveResult with from/to coordinates, or nil if parsing failed
  private func parseMoveResponse(_ response: String) -> MoveResult? {
    // Look for "move e2e4" pattern in the response
    let lines = response.split(separator: "\n")
    for line in lines {
      let trimmed = line.trimmingCharacters(in: .whitespaces)
      if trimmed.hasPrefix("move ") {
        let moveStr = String(trimmed.dropFirst(5))  // Remove "move "
        return parseMoveString(moveStr)
      }
    }
    return nil
  }

  /// Parse a move string like "e2e4" into coordinates
  /// - Parameter move: Move string in format "e2e4" or "e7e8q" (with promotion)
  /// - Returns: MoveResult with from/to coordinates
  private func parseMoveString(_ move: String) -> MoveResult? {
    guard move.count >= 4 else { return nil }

    let chars = Array(move)

    // Parse source square (e.g., "e2")
    guard let fromFile = chars[0].asciiValue.map({ Int($0) - 97 }),
      let fromRank = Int(String(chars[1])).map({ $0 - 1 }),
      fromFile >= 0, fromFile < 8, fromRank >= 0, fromRank < 8
    else { return nil }

    // Parse destination square (e.g., "e4")
    guard let toFile = chars[2].asciiValue.map({ Int($0) - 97 }),
      let toRank = Int(String(chars[3])).map({ $0 - 1 }),
      toFile >= 0, toFile < 8, toRank >= 0, toRank < 8
    else { return nil }

    let from = fileRankToCoordinate(file: fromFile, rank: fromRank)
    let to = fileRankToCoordinate(file: toFile, rank: toRank)

    return (from: from, to: to)
  }
}

// MARK: - Errors

public enum MicroMaxError: Error, LocalizedError {
  case iniFileNotFound
  case engineNotRunning
  case invalidFEN
  case noMoveReturned

  public var errorDescription: String? {
    switch self {
    case .iniFileNotFound:
      return "Could not find fmax.ini configuration file"
    case .engineNotRunning:
      return "Engine is not running"
    case .invalidFEN:
      return "Invalid FEN string"
    case .noMoveReturned:
      return "Engine did not return a move"
    }
  }
}

// MARK: - Legacy types

public typealias MoveResult = (from: ChessBoardCoordinate?, to: ChessBoardCoordinate?)

public enum GameStatus: String, Sendable {
  case normal  // 0
  case checkmated  // 1
  case stalemate  // 2
  case fiftyMove  // 3
  case insufficient  // 4

  init?(_ code: Int) {
    switch code {
    case 0: self = .normal
    case 1: self = .checkmated
    case 2: self = .stalemate
    case 3: self = .fiftyMove
    case 4: self = .insufficient
    default: return nil
    }
  }
}
