import MicroMaxOnAppleSilicon
import SwiftUI

func print(_ messages: Any...) {
  fputs(messages.map { "\($0)" }.joined(separator: " ") + "\n", stderr)
}

struct ContentView: View {
  @State private var inputText: String = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  @State private var aiMove: String = ""
  @State private var aiError: String = ""

  init() {}

  @State var bridge: MicroMaxBridge? = nil

  func doAiMove() {
    Task {
      guard let bridge else { return }
      do {
        let (from, to) = try await bridge.requestAiMove(fenState: inputText)
        guard let from, let to else { fatalError("no result") }
        aiMove = "from \(from) to \(to)"
        aiError = ""
      } catch {
        aiMove = ""
        aiError = error.localizedDescription
      }
    }
  }

  var body: some View {
    VStack {
      TextField("Enter text here", text: $inputText)
        .textFieldStyle(RoundedBorderTextFieldStyle())
        .padding()

      Button("Submit") { doAiMove() }
        .padding()
        .foregroundColor(.white)
        .background(Color.blue)
        .cornerRadius(10)

      Text("Next AI move: \(aiMove)")

      Text("Error: \(aiError)")
    }
    .padding()
    .task {
      if bridge == nil {
        bridge = MicroMaxBridge()
        print("AI connecting...")
        await bridge!.connectToEngine()
        print("AI connected")
      }
    }
  }
}

#Preview {
  ContentView()
}
