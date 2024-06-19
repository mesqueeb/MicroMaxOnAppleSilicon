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
