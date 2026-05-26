# Guidelines

## Running Tests

Always use `./test.sh` — never use `swift test` directly. Run `./test.sh` with no args to see usage and examples.

Key flags:
- `./test.sh` — run the whole suite
- `./test.sh --only 'MicroMaxOnAppleSiliconTests/Suite/test()'` — run a single test (path must match the full `@Suite` nesting chain; non-parameterized tests need trailing `()`)
- `./test.sh --show-prints` — include `print()` output from test code
- `./test.sh --verbose` — pass `--verbose` through to `swift test`

Run `./test.sh` with `run_in_background: true` and check output to detect crashes early — the C engine can SIGSEGV the whole test process (see issue #4).

## Shared C Engine State

The C engine in `Sources/CBridge/MicroMaxEngine.c` uses a `static struct engine_state` — global, single-instance. Multiple `MicroMaxBridge` instances all talk to the same engine.

Swift Testing parallelises tests by default, which races against this global state. Tests that call `startEngine` / `sendCommand` / `requestAiMove` / `stopEngine` **must** live inside a `@Suite(.serialized)`. See `EngineLifecycle` in `Tests/MicroMaxOnAppleSiliconTests.swift` for the pattern. Pure tests (coordinates, FEN parsing) can stay parallel.

## Swift Style

- Comments above variables should use `///` (doc comments) instead of `//`, so they show up on hover in IDEs.
- Format with `swift-format` using the repo-root `.swift-format` config.

## Fixing Bugs

When a bug is observed in Swift but caused in the C/Fairymax layer, fix it at the source — don't add Swift-side workarounds for C-side races or memory issues. The full stack (CBridge, Fairymax.c, SwiftBridge) is ours to modify.
