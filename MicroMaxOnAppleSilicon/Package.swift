// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

/// This `Package.swift` exists to be able to import the package in an Xcode project in the same repository.
///
/// Compared to the repository root `Package.swift`, everything is the same but the target paths.
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
      path: "Sources/CBridge",
      publicHeadersPath: "include"
    ),
    .target(
      name: "MicroMaxOnAppleSilicon",
      dependencies: ["MicroMaxCBridge"],
      path: "Sources/SwiftBridge",
      resources: [.copy("Resources/fmax.ini")]
    ),
    .testTarget(
      name: "MicroMaxOnAppleSiliconTests",
      dependencies: ["MicroMaxCBridge", "MicroMaxOnAppleSilicon"],
      path: "Tests"
    ),
  ],
  cLanguageStandard: .gnu17
)
