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
}

public enum MicroMaxError: Error, LocalizedError {
  case iniFileNotFound
  case engineNotRunning

  public var errorDescription: String? {
    switch self {
    case .iniFileNotFound:
      return "Could not find fmax.ini configuration file"
    case .engineNotRunning:
      return "Engine is not running"
    }
  }
}

// MARK: - Legacy types (kept for potential future use)

public typealias MoveResult = (from: ChessBoardCoordinate?, to: ChessBoardCoordinate?)

public enum GameStatus: String, Sendable {
  case normal // 0
  case checkmated // 1
  case stalemate // 2
  case fiftyMove // 3
  case insufficient // 4

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
