import Asyncify
import Foundation
import MicroMaxObjCBridge

private typealias MoveResultObjC = (src: (file: Int32, rank: Int32), dest: (file: Int32, rank: Int32))
public typealias MoveResult = (from: ChessBoardCoordinate?, to: ChessBoardCoordinate?)

public enum GameStatus: String, Sendable {
  case normal // 0
  case checkmated // 1
  case stalemate // 2
  case fiftyMove // 3
  case insufficient // 4

  init?(_ code: Int) {
    switch code {
    case 0:
      self = .normal
    case 1:
      self = .checkmated
    case 2:
      self = .stalemate
    case 3:
      self = .fiftyMove
    case 4:
      self = .insufficient
    default:
      return nil
    }
  }
}

public class MicroMaxBridge {
  private let bridge: MicroMaxObjCBridge

  public init() {
    bridge = MicroMaxObjCBridge()
  }

  public func connectToEngine() {
    bridge.connectToEngine()
  }

  /// Returns the current game status given a FEN string. `.normal` means normal play is ongoing.
  public func getGameStatus(_ fenState: String) -> GameStatus? {
    return GameStatus(Int(bridge.getGameStatus(fenState)))
  }

  /// Returns wether or not a move from one coordinate to another is legal or not.
  public func isMoveLegal(_ fenState: String, from: ChessBoardCoordinate, to: ChessBoardCoordinate) -> Bool {
    guard let src = coordinateToFileRank(from),
          let dest = coordinateToFileRank(to) else { return false }
    return bridge.isMoveLegal(
      fenState,
      srcFile: Int32(src.file),
      srcRank: Int32(src.rank),
      dstFile: Int32(dest.file),
      dstRank: Int32(dest.rank)
    )
  }

  private static var aiMoveConverterKey: UInt8 = 0

  private var aiMoveConverter: Asyncify<MoveResultObjC> {
    if let converter = objc_getAssociatedObject(self, &MicroMaxBridge.aiMoveConverterKey) as? Asyncify<MoveResultObjC> {
      return converter
    } else {
      let converter = Asyncify<MoveResultObjC>()
      objc_setAssociatedObject(self, &MicroMaxBridge.aiMoveConverterKey, converter, .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
      return converter
    }
  }

  /// Returns the file and rank with a 0 index
  public func requestAiMove(fenState: String) async throws -> MoveResult {
    // TODO: if passed fenState is invalid, throw error?
    // continuation.resume(throwing: error)
    // continuation.resume(throwing: NSError(domain: "UnknownError", code: -1, userInfo: nil))

    let (src, dest) = try await aiMoveConverter.performOperation { completion in
      self.bridge.receiveMove = { srcFile, srcRank, dstFile, dstRank, _ in
        let result = (src: (file: Int32(srcFile), rank: Int32(srcRank)), dest: (file: Int32(dstFile), rank: Int32(dstRank)))
        completion(.success(result))
      }
      self.bridge.request(fenState)
    }
    return (
      from: fileRankToCoordinate(file: Int(src.file), rank: Int(src.rank)),
      to: fileRankToCoordinate(file: Int(dest.file), rank: Int(dest.rank))
    )
  }
}
