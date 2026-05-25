# Micro-Max On Apple Silicon ♟️

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2Fmesqueeb%2FMicroMaxOnAppleSilicon%2Fbadge%3Ftype%3Dswift-versions)](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon)
[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2Fmesqueeb%2FMicroMaxOnAppleSilicon%2Fbadge%3Ftype%3Dplatforms)](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon)

```
.package(url: "https://github.com/mesqueeb/MicroMaxOnAppleSilicon", from: "3.0.0")
```

Micro-Max On Apple Silicon is the [µ-Max C Chess engine](https://home.hccnet.nl/h.g.muller/max-src2.html) by H.G. Muller to play Chess games. Wrapped and built as multi-platform Swift Package for iOS, macOS and visionOS.

## Installation

You can add `MicroMaxOnAppleSilicon` by adding it as a dependency to your `Package.swift` or add it via Xcode by searching for the name.

## Usage

You can use WinBoard protocol commands to interact with the engine.

```swift
import MicroMaxOnAppleSilicon

let bridge = MicroMaxBridge()

do {
  _ = try await bridge.startEngine()
  print(await bridge.sendCommand("xboard")) // nil
  print(await bridge.sendCommand("new")) // nil
  print(await bridge.sendCommand("white")) // nil
  print(await bridge.sendCommand("force")) // nil
  print(await bridge.sendCommand("st 1")) // nil
  print(await bridge.sendCommand("go")) // "move c2c4"
} catch {
  print("something went wrong... error:", error)
}

// Don't forget to stop the engine when done
await bridge.stopEngine()
```

The library comes with some useful types and helper functions, be sure to check out the [Swift wrapper's source code here](./MicroMaxOnAppleSilicon/Sources/SwiftBridge/).

One example is that you can use FEN state strings to request moves from the engine:

```swift
let fenState: String = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

do {
  let (from, to) = try await bridge.requestAiMove(fenState: fenState)
  guard let from, let to else { throw fatalError("no result") }

  print("MicroMax moves from \(from) to \(to)") // Eg. from "B7" to "B6"
} catch {
  print("something went wrong... error:", error)
}
```

### Sample Project

There is a sample Xcode project provided as part of the repo that you reference here: [MicroMaxTestApp](./MicroMaxTestApp/).

### Documentation

See the [documentation](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon/documentation/micromaxonapplesilicon) for more info.

### Advanced Implementation via fmax.ini customizations

The library uses an [fmax.ini](./MicroMaxOnAppleSilicon/Sources/SwiftBridge/Resources/fmax.ini) file bundled with the package to configure the engine's behavior. If you're familiar with C and Chess, you can customize this file to modify how the engine plays.

Currently, the engine always uses the `fmax.ini` file from the package bundle. To use a custom `fmax.ini` from your app's Bundle Resources, modify `startEngine()` in [MicroMaxBridge.swift](./MicroMaxOnAppleSilicon/Sources/SwiftBridge/MicroMaxBridge.swift) to check `Bundle.main` before falling back to `Bundle.module`. PRs welcome!

## Development

Open the workspace [MicroMaxOnAppleSilicon.xcworkspace](./MicroMaxOnAppleSilicon.xcworkspace) in Xcode to run the test apps and XCTests.

# Other Projects

Also check out Fuego on Apple Silicon ⚫️⚪️, the Fuego Go engine wrapped for Apple Silicon.

→ [github.com/mesqueeb/FuegoOnAppleSilicon](https://github.com/mesqueeb/FuegoOnAppleSilicon)
