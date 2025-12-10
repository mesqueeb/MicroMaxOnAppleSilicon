import MicroMaxOnAppleSilicon
import SwiftUI

let testCommands = [
  "xboard",
  "new",
  "white",
  "force",
  "st 10",
  "go",
]

struct ContentView: View {
  @State private var sentCommands: [String] = []
  @State private var testCommandIndex: Int = 0
  @State private var inputText: String = testCommands[0]
  @State private var engineResponse: String = ""
  @State private var engineError: String = ""

  @State var bridge: MicroMaxBridge? = nil

  init() {}

  func startEngine() async {
    if bridge == nil {
      bridge = MicroMaxBridge()
      print("Engine starting...")
      do {
        if let initOutput = try await bridge!.startEngine() {
          print("Engine init:", initOutput)
        }
        print("Engine started")
      } catch {
        print("Engine failed to start, error: \(error)")
        engineError = "\(error)"
      }
    }
  }

  func stopEngine() async {
    print("Stopping engine")
    await bridge?.stopEngine()
    testCommandIndex = 0
    bridge = nil
    sentCommands = []
    inputText = testCommands[0]
    engineResponse = ""
    engineError = ""
    print("Engine stopped")
  }

  func handleResponse(_ response: String?) {
    let responseText = response ?? "nil"
    print("response →", responseText)
    engineResponse = responseText

    testCommandIndex += 1
    if testCommandIndex >= testCommands.count {
      inputText = ""
    } else {
      inputText = testCommands[testCommandIndex]
    }

    engineError = ""
  }

  func submitCommand() {
    Task {
      guard let bridge else { return }
      sentCommands.append(inputText)
      let response = await bridge.sendCommand(inputText)
      handleResponse(response)
    }
  }

  var body: some View {
    VStack(spacing: 16) {
      if bridge == nil {
        Button("Start Engine") { Task { await startEngine() } }
          .padding()
          .foregroundColor(.white)
          .background(Color.blue)
          .cornerRadius(10)
      }

      if bridge != nil {
        TextField("Enter text here", text: $inputText)
          .textFieldStyle(RoundedBorderTextFieldStyle())
          .padding()

        Button("Submit") { submitCommand() }
          .padding()
          .foregroundColor(.white)
          .background(Color.blue)
          .cornerRadius(10)

        VStack {
          ForEach(Array(sentCommands.enumerated()), id: \.offset) { index, command in
            Text(command).font(.caption)
          }
        }

        Text("Engine Response: \(engineResponse)")

        Text("Error: \(engineError)")

        Button("Stop Engine") { Task { await stopEngine() } }
          .padding()
      }
    }
    .padding()
    .onDisappear { Task { await stopEngine() } }
  }
}

#Preview {
  ContentView()
}
