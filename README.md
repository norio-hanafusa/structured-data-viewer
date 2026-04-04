# Structured Data Viewer

A structured data viewer supporting JSON, JSON Lines, YAML, XML, and HTML. Available as a browser-based single HTML file and a cross-platform C++ desktop application.

## Features

- **Multi-format support** — Read and display JSON, JSONL (JSON Lines / NDJSON), YAML, XML, and HTML
- **Tree view / Table view** — Browse data structures from two perspectives
- **Inline editing** — Edit values, add and delete items directly in the GUI
- **Drag & drop** — Drop files to load them instantly
- **Format & export** — Pretty-print data and export to file
- **Undo / Redo** — Edit history management (Ctrl+Z / Ctrl+Y)
- **Dark / Light theme** — Beautiful UI powered by the Catppuccin color palette
- **Statistics** — Real-time display of key count, depth, node count, unique keys, and size

## Demo (Web Version)

https://norio-hanafusa.github.io/structured-data-viewer/index.html

## Web Version

Simply open `structured-data-viewer.html` in your browser. No installation required.

1. Paste JSON / JSONL / YAML / XML / HTML into the editor on the left
2. Select the input format using the format tabs
3. Click "Preview" to display

## C++ Desktop Version

A cross-platform desktop application built with Dear ImGui, GLFW, and OpenGL. All dependencies are automatically downloaded via CMake FetchContent.

### Prerequisites

| Platform | Requirements |
|---|---|
| **Windows** | Visual Studio 2022+ with C++ workload, CMake 3.20+ |
| **macOS** | Xcode Command Line Tools, CMake (`brew install cmake`) |
| **Linux** | GCC 8+ or Clang, CMake 3.20+, OpenGL and X11 dev packages |

Linux packages (Debian/Ubuntu):
```bash
sudo apt install cmake build-essential libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

### Build

```bash
cd structured-data-viewer-cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

On Windows with Visual Studio (from Developer Command Prompt):
```bash
cd structured-data-viewer-cpp
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run

```bash
./build/StructuredDataViewer        # macOS / Linux
build\StructuredDataViewer.exe      # Windows
```

### Notes

- Japanese font support uses system fonts automatically (Meiryo on Windows). On macOS/Linux, the app falls back to the ImGui default font if no Japanese font is found at the expected system path.
- HiDPI displays are supported via GLFW content scaling.

## Development

This project was created through Vibe Coding using ChatGPT, Gemini, and Claude.

## External Libraries

### Web Version

| Library | Purpose | License |
|---|---|---|
| [js-yaml](https://github.com/nodeca/js-yaml) | YAML parsing | MIT |
| [Google Fonts](https://fonts.google.com/) (JetBrains Mono, Noto Sans JP) | Fonts | [SIL OFL](https://scripts.sil.org/OFL) / [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0) |

### C++ Version

| Library | Purpose | License |
|---|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | GUI framework | MIT |
| [GLFW](https://github.com/glfw/glfw) | Windowing / input | Zlib |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing | MIT |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp) | YAML parsing | MIT |
| [pugixml](https://github.com/zeux/pugixml) | XML / HTML parsing | MIT |
| [Portable File Dialogs](https://github.com/samhocevar/portable-file-dialogs) | Native file dialogs | WTFPL |

## Color Palette

The UI color palette is based on [Catppuccin](https://github.com/catppuccin/catppuccin).

> Copyright (c) 2021 Catppuccin
> Licensed under the MIT License

## License

MIT License — See [LICENSE](LICENSE) for details.
