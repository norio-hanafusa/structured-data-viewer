# Structured Data Viewer

A browser-based structured data viewer supporting JSON, JSON Lines, YAML, XML, and HTML. Runs as a single HTML file with no installation required.

## Features

- **Multi-format support** — Read and display JSON, JSONL (JSON Lines / NDJSON), YAML, XML, and HTML
- **Tree view / Table view** — Browse data structures from two perspectives
- **Inline editing** — Edit values, add and delete items directly in the GUI
- **Drag & drop** — Drop files to load them instantly
- **Format & export** — Pretty-print data and export to file
- **Undo / Redo** — Edit history management (Ctrl+Z / Ctrl+Y)
- **Dark / Light theme** — Beautiful UI powered by the Catppuccin color palette
- **Statistics** — Real-time display of key count, depth, node count, unique keys, and size
- **Responsive** — Works on both desktop and mobile

## Demo

https://norio-hanafusa.github.io/structured-data-viewer/index.html

## Usage

Simply open `structured-data-viewer.html` in your browser.

1. Paste JSON / JSONL / YAML / XML / HTML into the editor on the left
2. Select the input format using the format tabs
3. Click "Preview" to display

## Development

This project was created through Vibe Coding using ChatGPT, Gemini, and Claude.

## External Libraries

| Library | Purpose | License |
|---|---|---|
| [js-yaml](https://github.com/nodeca/js-yaml) | YAML parsing | MIT |
| [Google Fonts](https://fonts.google.com/) (JetBrains Mono, Noto Sans JP) | Fonts | [SIL Open Font License](https://scripts.sil.org/OFL) / [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) |

## Color Palette

The UI color palette is based on [Catppuccin](https://github.com/catppuccin/catppuccin).

> Copyright (c) 2021 Catppuccin
> Licensed under the MIT License

## License

MIT License — See [LICENSE](LICENSE) for details.
