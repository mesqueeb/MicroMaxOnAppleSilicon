/// Parses FEN strings into piece placements for WinBoard edit mode
struct FENParser {
  /// Result of parsing a FEN string
  struct ParseResult {
    /// Dictionary mapping (file, rank) to piece character (e.g., 'P' for white pawn, 'p' for black pawn)
    let pieces: [(file: Int, rank: Int, piece: Character)]
    /// Active color: "w" for white, "b" for black
    let activeColor: String
  }

  /// Parse a FEN string into piece placements and active color
  /// - Parameter fen: FEN string like "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  /// - Returns: ParseResult with pieces and active color, or nil if invalid
  static func parse(_ fen: String) -> ParseResult? {
    let parts = fen.split(separator: " ")
    guard parts.count >= 2 else { return nil }

    let boardPart = String(parts[0])
    let activeColor = String(parts[1])
    
    // Validate active color
    guard activeColor == "w" || activeColor == "b" else { return nil }

    // Valid chess piece characters
    let validPieces: Set<Character> = ["p", "r", "n", "b", "q", "k", "P", "R", "N", "B", "Q", "K"]
    
    var pieces: [(file: Int, rank: Int, piece: Character)] = []
    let ranks = boardPart.split(separator: "/")
    
    // Must have exactly 8 ranks
    guard ranks.count == 8 else { return nil }

    for (rankIndex, rank) in ranks.enumerated() {
      var fileIndex = 0
      for char in rank {
        if char.isNumber, let skip = Int(String(char)) {
          fileIndex += skip
        } else if validPieces.contains(char) {
          // FEN ranks go from 8 (index 0) to 1 (index 7)
          let actualRank = 7 - rankIndex
          pieces.append((file: fileIndex, rank: actualRank, piece: char))
          fileIndex += 1
        } else {
          // Invalid character in FEN
          return nil
        }
      }
      
      // Each rank must have exactly 8 squares
      guard fileIndex == 8 else { return nil }
    }

    return ParseResult(pieces: pieces, activeColor: activeColor)
  }
}
