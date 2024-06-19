// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

/// This package is a stub. It's created just so when referencing the source code of the `MicroMaxOnAppleSilicon` folder
/// In the MicroMaxTestApp, that it will compile because otherwise it complains about not finding MicroMaxObjCBridge.
///
/// With MicroMaxTestApp we can develop the source code of `MicroMaxOnAppleSilicon` live without needing to redeploy.
let package = Package(
  name: "MicroMaxObjCBridge",
  products: [
    // Products define the executables and libraries a package produces, making them visible to other packages.
    .library(
      name: "MicroMaxObjCBridge",
      targets: ["MicroMaxObjCBridge"]),
  ],
  targets: [
    // Targets are the basic building blocks of a package, defining a module or a test suite.
    // Targets can depend on other targets in this package and products from dependencies.
    .target(
      name: "MicroMaxObjCBridge"),
  ])
