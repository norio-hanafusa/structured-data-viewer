# Structured Data Viewer

JSON / YAML / XML / HTML に対応したブラウザベースの構造化データビューアです。単一の HTML ファイルで動作し、インストール不要で使えます。

## 機能

- **マルチフォーマット対応** — JSON, YAML, XML, HTML の読み込み・表示
- **ツリービュー / テーブルビュー** — データ構造を2つの視点で閲覧
- **インライン編集** — 値の編集、項目の追加・削除をGUI上で実行
- **ドラッグ＆ドロップ** — ファイルをドロップして即座に読み込み
- **整形・書き出し** — データの整形(フォーマット)とファイルエクスポート
- **Undo / Redo** — 編集履歴の管理 (Ctrl+Z / Ctrl+Y)
- **ダーク / ライトテーマ** — Catppuccin カラーパレットによる美しいUI
- **統計表示** — キー数、深さ、ノード数、ユニークキー数、サイズをリアルタイム表示
- **レスポンシブ対応** — デスクトップ・モバイルどちらでも利用可能

## 使い方

`structured-data-viewer.html` をブラウザで開くだけで利用できます。

1. 左のエディタに JSON / YAML / XML / HTML を貼り付け
2. フォーマットタブで入力形式を選択
3. 「プレビュー」をクリックして表示

## 開発について

このプロジェクトは ChatGPT, Gemini, Claude を活用した Vibe Coding で作成されました。

## 外部ライブラリ

| ライブラリ | 用途 | ライセンス |
|---|---|---|
| [js-yaml](https://github.com/nodeca/js-yaml) | YAML パース | MIT |
| [Google Fonts](https://fonts.google.com/) (JetBrains Mono, Noto Sans JP) | フォント | [SIL Open Font License](https://scripts.sil.org/OFL) / [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0) |

## カラーパレット

UIのカラーパレットは [Catppuccin](https://github.com/catppuccin/catppuccin) をベースにしています。

> Copyright (c) 2021 Catppuccin
> Licensed under the MIT License

## ライセンス

MIT License — 詳細は [LICENSE](LICENSE) を参照してください。
