# Micro-Max On Apple Silicon ♟️

[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2Fmesqueeb%2FMicroMaxOnAppleSilicon%2Fbadge%3Ftype%3Dswift-versions)](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon)
[![](https://img.shields.io/endpoint?url=https%3A%2F%2Fswiftpackageindex.com%2Fapi%2Fpackages%2Fmesqueeb%2FMicroMaxOnAppleSilicon%2Fbadge%3Ftype%3Dplatforms)](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon)

```
.package(url: "https://github.com/mesqueeb/MicroMaxOnAppleSilicon", from: "2.2.1")
```

Micro-Max On Apple Silicon is the [µ-Max C Chess engine](https://home.hccnet.nl/h.g.muller/max-src2.html) by H.G. Muller to play Chess games. Built as multi-platform XCframework for iOS, macOS and visionOS. Wrapped as a modernised Swift Package that can be included in any Swift project and can build on all Apple platforms.

## Installation

You can add `MicroMaxOnAppleSilicon` by adding it as a dependency to your `Package.swift` or add it via Xcode by searching for the name.

## Usage

```swift
import MicroMaxOnAppleSilicon

let bridge = MicroMaxBridge()

bridge.connectToEngine()

/// You need to feed the engine FEN state strings to be able to request moves
let fenState: String = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

do {
  let (from, to) = try await bridge.requestAiMove(fenState: inputText)
  guard let from, let to else { throw fatalError("no result") }

  print("MicroMax moves from \(from) to \(to)") // Eg. from "B7" to "B6"
} catch {
  print("something went wrong... error:", error)
}
```

The library comes with some useful types and helper functions, be sure to check out the [Swift wrapper's source code here](./MicroMaxOnAppleSilicon/SwiftBridge/).

### Sample Project

There is a sample Xcode project provided as part of the repo that you reference here: [MicroMaxTestApp](./MicroMaxTestApp/).

### Documentation

See the [documentation](https://swiftpackageindex.com/mesqueeb/MicroMaxOnAppleSilicon/documentation/micromaxonapplesilicon) for more info.

### Advanced Implementation via fmax.ini customizations

The library currently uses an [fmax.ini](./MicroMaxOnAppleSilicon/Resources/fmax.ini) file bundled with the package which has information on how the engine behaves. If you are familiar with C, C++ and Chess, you can look through the source code and determine how to customize this file to your liking.

If you want to bring your own `fmax.ini` file you can simply include it in your project's Bundle Resources, and the source code will use that file instead of the default behaviour. You can copy the base file and paste it into your project's directory and add it to the Bundle Resources of your target:

![](./docs/copy_fmax_ini_file.jpg)
![](./docs/add_to_bundle_resources.jpg)

That's all! Now the engine will use your own `fmax.ini` file in your project.

PS: Here is the code that loads the `fmax.ini` file: [MicroMaxOnAppleSilicon/Sources/ObjCBridge/EngineContext.m](./MicroMaxOnAppleSilicon/Sources/ObjCBridge/EngineContext.m). Let me know if there are any issues with this. PRs welcome!

## Development

Open the workspace [MicroMaxOnAppleSilicon.xcworkspace](./MicroMaxOnAppleSilicon.xcworkspace) in Xcode to run the test apps and XCTests.

# Other Projects

Also check out Fuego on Apple Silicon ⚫️⚪️, the Fuego Go engine wrapped for Apple Silicon.

→ [github.com/mesqueeb/FuegoOnAppleSilicon](https://github.com/mesqueeb/FuegoOnAppleSilicon)
