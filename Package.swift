// swift-tools-version: 5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

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
      path: "MicroMaxOnAppleSilicon/Sources/CBridge"
    ),
    .target(
      name: "MicroMaxCppBridge",
      dependencies: ["MicroMaxCBridge"],
      path: "MicroMaxOnAppleSilicon/Sources/CppBridge"
    ),
    .target(
      name: "MicroMaxObjCBridge",
      dependencies: ["MicroMaxCppBridge"],
      path: "MicroMaxOnAppleSilicon/Sources/ObjCBridge",
      resources: [.copy("Resources/fmax.ini")],
      publicHeadersPath: "include"
    ),
    .target(
      name: "MicroMaxOnAppleSilicon",
      dependencies: ["Asyncify", "MicroMaxObjCBridge"],
      path: "MicroMaxOnAppleSilicon/Sources/SwiftBridge"
    ),
    .testTarget(
      name: "MicroMaxOnAppleSiliconTests",
      dependencies: ["MicroMaxCBridge", "MicroMaxCppBridge", "MicroMaxObjCBridge", "MicroMaxOnAppleSilicon"],
      path: "MicroMaxOnAppleSilicon/Tests"
    ),
  ],
  cLanguageStandard: .gnu17,
  cxxLanguageStandard: .cxx20
)
