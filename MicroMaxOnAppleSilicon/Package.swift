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
      targets: ["MicroMaxCBridge", "MicroMaxCppBridge", "MicroMaxObjCBridge", "MicroMaxOnAppleSilicon"]
    ),
  ],
  dependencies: [
    .package(url: "https://github.com/mesqueeb/Asyncify", from: "0.0.9"),
  ],
  targets: [
    .target(
      name: "MicroMaxCBridge",
      path: "Sources/CBridge"
    ),
    .target(
      name: "MicroMaxCppBridge",
      dependencies: ["MicroMaxCBridge"],
      path: "Sources/CppBridge"
    ),
    .target(
      name: "MicroMaxObjCBridge",
      dependencies: ["MicroMaxCppBridge"],
      path: "Sources/ObjCBridge",
      resources: [.copy("Resources/fmax.ini")],
      publicHeadersPath: "include"
    ),
    .target(
      name: "MicroMaxOnAppleSilicon",
      dependencies: ["Asyncify", "MicroMaxObjCBridge"],
      path: "Sources/SwiftBridge"
    ),
    .testTarget(
      name: "MicroMaxOnAppleSiliconTests",
      dependencies: ["MicroMaxCBridge", "MicroMaxCppBridge", "MicroMaxObjCBridge", "MicroMaxOnAppleSilicon"],
      path: "Tests"
    ),
  ],
  cLanguageStandard: .gnu17,
  cxxLanguageStandard: .cxx20
)
