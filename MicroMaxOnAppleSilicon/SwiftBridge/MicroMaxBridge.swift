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
    print("src →", src)
    print("dest →", dest)
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

  /// Converts a file and rank with a 0 index to chess coordinates like "A1"
  /// - "file" is the column index
  /// - "rank" is the row index
  public func fileRankToCoordinate(file: Int, rank: Int) -> ChessBoardCoordinate? {
    let letters = ["A", "B", "C", "D", "E", "F", "G", "H"]
    // Convert the tuple to chess coordinates
    let coordinate = letters[file] + "\(rank + 1)"
    return ChessBoardCoordinate(rawValue: coordinate)
  }

  /// Converts a chess coordinate like "A1" to the chess board indexes, eg. (file: 0, rank: 0)
  /// - "file" is the column index
  /// - "rank" is the row index
  public func coordinateToFileRank(_ tile: ChessBoardCoordinate) -> (file: Int, rank: Int)? {
    guard let rowChar = tile.rawValue.last, let row = Int(String(rowChar)),
          let columnChar = tile.rawValue.first
    else { return nil }

    // Calculate row index
    let rank = row - 1

    // Calculate column index
    let colDic: [Character: Int] = ["A": 0, "B": 1, "C": 2, "D": 3, "E": 4, "F": 5, "G": 6, "H": 7]
    guard let file = colDic[columnChar] else { return nil }

    return (file, rank)
  }
}
