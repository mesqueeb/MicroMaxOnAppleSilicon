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

  func testStartEngineReturnsInitBanners() async throws {
    let initOutput = try await bridge.startEngine()
    
    // Engine should return initialization banners containing "Fairy-Max"
    XCTAssertNotNil(initOutput, "Engine should return init banners")
    XCTAssertTrue(initOutput?.contains("Fairy-Max") == true, "Init output should contain Fairy-Max")
    
    // Verify engine is running
    let isRunning = await bridge.engineRunning
    XCTAssertTrue(isRunning, "Engine should be running after start")
    
    // Stop the engine
    await bridge.stopEngine()
  }

  func testSendCommandNoResponse() async throws {
    _ = try await bridge.startEngine()
    
    // Commands like "new" should return nil (no response)
    let response = await bridge.sendCommand("new")
    XCTAssertNil(response, "'new' command should return nil (no response)")
    
    await bridge.stopEngine()
  }

  func testSendCommandWithResponse() async throws {
    _ = try await bridge.startEngine()
    
    // Set up for a move
    _ = await bridge.sendCommand("new")
    _ = await bridge.sendCommand("force")
    _ = await bridge.sendCommand("st 1")  // 1 second think time
    
    // "go" command should return a move
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
}
