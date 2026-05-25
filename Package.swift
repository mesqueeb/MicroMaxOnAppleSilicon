// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

let package = Package(
  name: "MicroMaxOnAppleSilicon",
  platforms: [.iOS(.v14), .macOS(.v11)],
  products: [
    .library(
      name: "MicroMaxOnAppleSilicon",
      targets: ["MicroMaxCBridge", "MicroMaxOnAppleSilicon"]
    ),
  ],
  targets: [
    .target(
      name: "MicroMaxCBridge",
      path: "MicroMaxOnAppleSilicon/Sources/CBridge",
      publicHeadersPath: "include"
    ),
    .target(
      name: "MicroMaxOnAppleSilicon",
      dependencies: ["MicroMaxCBridge"],
      path: "MicroMaxOnAppleSilicon/Sources/SwiftBridge",
      resources: [.copy("Resources/fmax.ini")]
    ),
    .testTarget(
      name: "MicroMaxOnAppleSiliconTests",
      dependencies: ["MicroMaxCBridge", "MicroMaxOnAppleSilicon"],
      path: "MicroMaxOnAppleSilicon/Tests"
    ),
  ],
  cLanguageStandard: .gnu17
)
