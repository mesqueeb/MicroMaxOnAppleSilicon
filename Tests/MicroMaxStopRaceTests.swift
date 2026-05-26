@testable import MicroMaxOnAppleSilicon
import Testing

/// Reproduces a SIGSEGV in `main_fairymax` when `micromax_engine_stop` is called
/// while a search is in flight. The engine thread is detached and can outlive
/// the stop call; closing its pipes and freeing engine_state out from under it
/// is the race. Either `fflush(stdout)` (write path) or `read(__engineFD__)`
/// (read path) faults on the next iteration of the engine loop.
///
/// Nested inside `EngineLifecycle` so it shares the parent's `.serialized`
/// trait — running this in parallel with `EngineTests`/`RequestAiMoveTests`
/// races on the global `engine_state` in MicroMaxEngine.c.
extension EngineLifecycle {
  @Suite struct StopRaceTests {
    @Test func stopEngineDuringActiveSearchDoesNotCrash() async throws {
      // Sweep delays so at least one iteration lands while the engine is actually
      // searching rather than parked on input.
      let delays: [Int] = [0, 10, 30, 60, 100, 150, 250, 500, 1000, 2000, 3000]
      let startingFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

      for (i, delayMs) in delays.enumerated() {
        let bridge = MicroMaxBridge()
        _ = try await bridge.startEngine()

        // Fire the search; don't await — we want it mid-flight when we stop.
        let searchTask = Task.detached {
          _ = try? await bridge.requestAiMove(fenState: startingFen, thinkTime: 30)
        }

        try await Task.sleep(for: .milliseconds(delayMs))

        await bridge.stopEngine()

        // Hold the test alive past the 50ms usleep + a tail so the detached pthread's
        // next fflush/printf/read fires against torn-down state.
        try await Task.sleep(for: .seconds(2))

        searchTask.cancel()
        print("iter \(i) (delay=\(delayMs)ms) survived")
      }
    }
  }
}
