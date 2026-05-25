import MicroMaxOnAppleSilicon
import SwiftUI

let testCommands = [
  "xboard",
  "new",
  "white",
  "force",
  "st 1",
  "go",
]

struct ContentView: View {
  @State private var commandResponsePairs: [(command: String, response: String)] = []
  @State private var testCommandIndex: Int = 0
  @State private var inputText: String = testCommands[0]
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
    commandResponsePairs = []
    inputText = testCommands[0]
    engineError = ""
    print("Engine stopped")
  }

  func updateResponse(at index: Int, _ response: String?) {
    let responseText = response ?? "nil"
    print("response →", responseText)
    if index < commandResponsePairs.count {
      commandResponsePairs[index].response = responseText
    }

    testCommandIndex += 1
    if testCommandIndex >= testCommands.count {
      inputText = ""
    } else {
      inputText = testCommands[testCommandIndex]
    }

    engineError = ""
  }

  func submitCommand() {
    guard let bridge else { return }
    let command = inputText
    
    // Add command immediately with empty response
    let index = commandResponsePairs.count
    commandResponsePairs.append((command: command, response: ""))
    
    // Update response asynchronously when it arrives
    Task {
      let response = await bridge.sendCommand(command)
      updateResponse(at: index, response)
    }
  }

  var body: some View {
    VStack(spacing: 0) {
      if bridge == nil {
        Button("Start Engine") { Task { await startEngine() } }
          .padding()
          .foregroundColor(.white)
          .background(Color.blue)
          .cornerRadius(10)
      }

      if bridge != nil {
        GeometryReader { geometry in
          ScrollViewReader { proxy in
            ScrollView {
              VStack(alignment: .trailing, spacing: 8) {
                Spacer()
                
                ForEach(Array(commandResponsePairs.enumerated()), id: \.offset) { index, pair in
                  HStack(alignment: .top, spacing: 16) {
                    // Command on the left
                    Text(pair.command)
                      .font(.caption)
                      .frame(maxWidth: .infinity, alignment: .leading)

                    // Response on the right
                    Text(pair.response.isEmpty ? "..." : pair.response)
                      .font(.caption)
                      .foregroundColor(.white)
                      .padding()
                      .frame(maxWidth: .infinity, alignment: .leading)
                      .background(Color.black)
                  }
                  .id(index)
                }
              }
              .frame(minHeight: geometry.size.height - 50)
              .padding()
            }
            .onChange(of: commandResponsePairs.count) { _ in
              if let lastIndex = commandResponsePairs.indices.last {
                withAnimation {
                  proxy.scrollTo(lastIndex, anchor: .bottom)
                }
              }
            }
          }
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)

        // Input and button at the bottom
        HStack(spacing: 8) {
          TextField("Enter text here", text: $inputText)
            .textFieldStyle(RoundedBorderTextFieldStyle())

          Button("Submit") { submitCommand() }
            .padding(.horizontal)
            .foregroundColor(.white)
            .background(Color.blue)
            .cornerRadius(10)
        }
        .padding()

        if !engineError.isEmpty {
          Text("Error: \(engineError)")
            .foregroundColor(.red)
            .font(.caption)
            .padding(.horizontal)
        }

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
