---
name: js-cpp-protocol
description: 'Reference for the communication protocol between the JavaScript editor (CodeMirror or Monaco) and the C++/Qt UI layer via QWebChannel. Use when implementing new editor features, adding messages to the bridge, debugging JS/C++ communication, or understanding how editor commands flow between layers.'
---

# JS Editor / C++ UI Communication Protocol

## Overview

Notepadqq's editor uses **Qt WebEngine** (`QWebEngineView`) with **Qt WebChannel** (`QWebChannel`) for bi-directional communication between the native C++ UI and the JavaScript editor engine (either **CodeMirror** or **Monaco**). The C++ side is unaware of which engine is running — both engines implement the identical message protocol.

## Architecture

```
┌───────────────────────────┐          QWebChannel           ┌──────────────────────────┐
│        C++ / Qt           │  ◄─────────────────────────►   │   JavaScript Editor      │
│                           │                                │                          │
│  Editor (editor.cpp)      │  signal: messageReceivedByJs   │  UiDriver.js             │
│    ├─ JsToCppProxy (QObject)│  ──────────────────────────►  │    ├─ QWebChannel bridge │
│    └─ CustomQWebView      │  ◄──────────────────────────  │    └─ messageReceived()   │
│                           │  slot: receiveMessage()        │                          │
│  EditorTabWidget          │                                │  app.js / app_monaco.js  │
│  MainWindow               │                                │    └─ event handlers     │
└───────────────────────────┘                                └──────────────────────────┘
```

## Key Files

| Side | File | Role |
|------|------|------|
| JS | `src/editor/classes/UiDriver.js` | Core bridge: QWebChannel setup, `sendMessage()`, `registerEventHandler()`, `messageReceived()` dispatcher |
| JS | `src/editor/app.js` | CodeMirror implementation: registers handlers for all `C_CMD_*` / `C_FUN_*` messages |
| JS | `src/editor/app_monaco.js` | Monaco implementation: registers handlers for all `C_CMD_*` / `C_FUN_*` messages |
| JS | `src/editor/index.html` | CodeMirror HTML entry point |
| JS | `src/editor/index_monaco.html` | Monaco HTML entry point |
| C++ | `src/ui/EditorNS/editor.cpp` | Creates QWebChannel, sends messages, receives replies, manages async callbacks |
| C++ | `src/ui/include/EditorNS/editor.h` | Declares `JsToCppProxy`, `Editor`, `AsyncReply` |
| C++ | `src/ui/EditorNS/customqwebview.cpp` | QWebEngineView subclass: event handling, drag-drop, context menu |

## Communication Mechanism

### Setup

1. **C++** (`editor.cpp:47-70`): Creates a `JsToCppProxy` QObject (registered as `"cpp_ui_driver"`), creates a `QWebChannel`, sets it on the WebEngine page, and registers the proxy: `channel->registerObject("cpp_ui_driver", m_jsToCppProxy)`.

2. **JS** (`index.html:7` / `index_monaco.html:7`): Includes `<script src="qrc:///qtwebchannel/qwebchannel.js"></script>` (Qt's built-in WebChannel JS library).

3. **JS** (`UiDriver.js:8-26`): On `DOMContentLoaded`, creates `new QWebChannel(qt.webChannelTransport, callback)`, obtains `cpp_ui_driver = channel.objects.cpp_ui_driver`, and connects to the `messageReceivedByJs` signal.

### C++ → JS (Commands and Function Calls)

Two modes exist:

**Synchronous (legacy/deprecated):** `sendMessage(msg, data)`
- `editor.cpp:374-382`: Emits `messageReceivedByJs` signal, which Qt WebChannel delivers to JS.
- JS receives it via `cpp_ui_driver.messageReceivedByJs.connect(...)` and processes it immediately.

**Asynchronous (preferred):** `asyncSendMessageWithResultP(msg, data)`
- `editor.cpp:389-430`: Generates a unique message ID, wraps the message in `[ASYNC_REQUEST]` prefix + `[ID=N]` suffix.
- JS receives it, dispatches to registered handlers, then sends back `[ASYNC_REPLY]` with the same ID.
- C++ resolves the `QtPromise::QPromise<QVariant>` on receipt.

**Message format for async:**
```
C++ sends:  [ASYNC_REQUEST]C_CMD_GET_VALUE[ID=42]
JS replies: [ASYNC_REPLY]C_CMD_GET_VALUE[ID=42]
```

### JS → C++ (Events)

JS calls `UiDriver.sendMessage("J_EVT_*", data)` which invokes `cpp_ui_driver.receiveMessage(msg, data)` (UiDriver.js:36).

C++ receives it in `JsToCppProxy::receiveMessage()` (editor.h:44), which emits `messageReceived` signal → `Editor::on_proxyMessageReceived()` (editor.cpp:143).

## Message Naming Convention

| Prefix | Direction | Semantics |
|--------|-----------|-----------|
| `C_CMD_*` | C++ → JS | Fire-and-forget command (no return value expected) |
| `C_FUN_*` | C++ → JS | Function call (return value expected via async reply) |
| `J_EVT_*` | JS → C++ | Event notification (no return value) |

## Complete Message Reference

### C_CMD_* — Commands (C++ → JS, no return value)

| Message | Data | Description |
|---------|------|-------------|
| `C_CMD_SET_VALUE` | `QString` | Set full editor text content |
| `C_CMD_MARK_CLEAN` | none | Mark document as clean (no unsaved changes) |
| `C_CMD_MARK_DIRTY` | none | Mark document as dirty (unsaved changes) |
| `C_CMD_SET_LANGUAGE` | language MIME string | Set syntax highlighting language |
| `C_CMD_SET_INDENTATION_MODE` | `{useTabs: bool, size: int}` | Set tab/space indentation |
| `C_CMD_SET_SELECTIONS_TEXT` | `{text: string[], select: "after"\|"before"\|"selected"}` | Replace selected text |
| `C_CMD_SET_SELECTION` | `[fromLine, fromCol, toLine, toCol]` | Set selection range |
| `C_CMD_SET_CURSOR` | `[line, col]` | Set cursor position |
| `C_CMD_SET_RTL` | none | Set text direction to RTL |
| `C_CMD_SET_LTR` | none | Set text direction to LTR |
| `C_CMD_SET_SCROLL_POS` | `[left, top]` | Set scroll position |
| `C_CMD_SELECT_ALL` | none | Select entire document |
| `C_CMD_UNDO` | none | Undo last change |
| `C_CMD_REDO` | none | Redo last undone change |
| `C_CMD_CLEAR_HISTORY` | none | Clear undo history |
| `C_CMD_SET_LINE_WRAP` | `bool` | Toggle line wrapping |
| `C_CMD_SHOW_END_OF_LINE` | `bool` | Show/hide end-of-line characters |
| `C_CMD_SHOW_WHITESPACE` | `bool` | Show/hide whitespace characters |
| `C_CMD_SET_TABS_VISIBLE` | `bool` | Show/hide tab characters |
| `C_CMD_SET_THEME` | `{name: string, path: string}` | Apply editor theme |
| `C_CMD_SET_FONT` | `{family: string, size: int, lineHeight: double}` | Set editor font |
| `C_CMD_SET_LINE_NUMBERS_VISIBLE` | `bool` | Show/hide line numbers |
| `C_CMD_SET_OVERWRITE` | `bool` | Toggle overwrite mode |
| `C_CMD_SET_SMART_INDENT` | `bool` | Toggle smart indent |
| `C_CMD_SET_FOCUS` | none | Focus the editor |
| `C_CMD_BLUR` | none | Blur the editor |
| `C_CMD_DISPLAY_PRINT_STYLE` | none | Switch to print-friendly CSS |
| `C_CMD_DISPLAY_NORMAL_STYLE` | none | Switch back to normal CSS |
| `C_CMD_DUPLICATE_LINE` | none | Duplicate current line |
| `C_CMD_MOVE_LINE_UP` | none | Move current line up |
| `C_CMD_MOVE_LINE_DOWN` | none | Move current line down |
| `C_CMD_TRANSPOSE_LINE` | none | Transpose with previous line |
| `C_CMD_DELETE_LINE` | none | Delete current line |
| `C_CMD_TRIM_LEADING_TRAILING_SPACE` | none | Trim leading and trailing whitespace |
| `C_CMD_TRIM_TRAILING_SPACE` | none | Trim only trailing whitespace |
| `C_CMD_TRIM_LEADING_SPACE` | none | Trim only leading whitespace |
| `C_CMD_ENABLE_MATH` | `bool` | Enable/disable LaTeX math rendering |
| `C_CMD_TAB_TO_SPACE` | none | Convert tabs to spaces |
| `C_CMD_SPACE_TO_TAB_ALL` | none | Convert all spaces to tabs |
| `C_CMD_SPACE_TO_TAB_LEADING` | none | Convert leading spaces to tabs |
| `C_CMD_EOL_TO_SPACE` | none | Replace line endings with spaces |
| `C_CMD_GET_DOCUMENT_INFO` | none | Request document info (replied via J_EVT_DOCUMENT_INFO) |

### C_FUN_* — Function Calls (C++ → JS, return value via async reply)

| Message | Data | Return Type |
|---------|------|-------------|
| `C_FUN_IS_CLEAN` | none | `bool` |
| `C_FUN_GET_HISTORY_GENERATION` | none | `int` |
| `C_FUN_GET_VALUE` | none | `QString` |
| `C_FUN_GET_INDENTATION_MODE` | none | `{useTabs: bool, size: int}` |
| `C_FUN_GET_SELECTIONS_TEXT` | none | `QStringList` |
| `C_FUN_GET_SELECTIONS` | none | `[{anchor: {line, col}, head: {line, col}}]` |
| `C_FUN_GET_TEXT_LENGTH` | none | `int` |
| `C_FUN_GET_LINE_COUNT` | none | `int` |
| `C_FUN_GET_CURSOR` | none | `[line, col]` |
| `C_FUN_GET_SCROLL_POS` | none | `[left, top]` |
| `C_FUN_SEARCH` | `[regex: string, modifiers: string, forward: bool]` | `bool` (found or not) |
| `C_FUN_REPLACE` | `[regex, modifiers, forward, replacement, searchMode]` | `bool` |
| `C_FUN_REPLACE_ALL` | `[regex, modifiers, replacement, searchMode]` | `int` (count) |
| `C_FUN_SEARCH_SELECT_ALL` | `[regex, modifiers]` | `int` (count) |
| `C_FUN_GET_LANGUAGES` | none | `array` of `{name, mime, mode, ext}` |
| `C_FUN_DETECT_INDENTATION_MODE` | none | `{found: bool, useTabs: bool, size: int}` |
| `C_FUN_GET_CURRENT_WORD` | none | `QString` |
| `C_FUN_IS_MATH_ENABLED` | none | `bool` |

### J_EVT_* — Events (JS → C++, no return value)

| Message | Data | When Sent |
|---------|------|-----------|
| `J_EVT_READY` | none | Editor initialization complete |
| `J_EVT_CONTENT_CHANGED` | none | Document content changes (throttled ~50ms) |
| `J_EVT_CLEAN_CHANGED` | `bool` | Clean/dirty state changes |
| `J_EVT_CURSOR_ACTIVITY` | `{cursor: {...}, selections: [...], content: {...}}` | Cursor/selection changes (throttled ~50ms) |
| `J_EVT_DOCUMENT_INFO` | `{cursor, selections, content}` | Reply to `C_CMD_GET_DOCUMENT_INFO` |
| `J_EVT_GOT_FOCUS` | none | Editor receives focus |

## Handler Registration Pattern (JS)

Handlers are registered in `app.js` or `app_monaco.js`:

```javascript
UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.setValue(data);
});
```

Multiple handlers can be registered for the same message. They are called in registration order; each handler receives the previous handler's return value as `prevReturn`.

## Async Flow in Detail

1. **C++ generates a unique ID** (`messageIdentifier` counter, `editor.cpp:387`).
2. **C++ creates a promise** and stores an `AsyncReply{id, message, value, callback}` in the `asyncReplies` list (`editor.cpp:407-412`).
3. **C++ sends** `[ASYNC_REQUEST]C_FUN_GET_CURSOR[ID=42]` via `messageReceivedByJs` signal (`editor.cpp:414-418`).
4. **JS receives** in `UiDriver.messageReceived()` (`UiDriver.js:52-75`), parses the real message and ID via regex `/^\[ASYNC_REQUEST\](.*)\[ID=(\d+)\]$/`, dispatches to handler(s), then **sends back** `[ASYNC_REPLY]C_FUN_GET_CURSOR[ID=42]` with the return value.
5. **C++ receives** in `Editor::on_proxyMessageReceived()` (`editor.cpp:148-176`), parses the ID via regex `\\[ID=(\\d+)\\]$`, looks up the matching `AsyncReply`, resolves the promise and/or calls the callback, then emits `asyncReplyReceived`.

## C++ API for Sending Messages

```cpp
// Legacy synchronous (deprecated — blocks event loop)
void sendMessage(const QString msg, const QVariant data);

// Modern async with QtPromise (preferred)
QtPromise::QPromise<QVariant> asyncSendMessageWithResultP(const QString msg, const QVariant data);

// Legacy future-based (deprecated — spins event loop in while())
std::shared_future<QVariant> asyncSendMessageWithResult(
    const QString msg, const QVariant data,
    std::function<void(QVariant)> callback = nullptr);
```

## Adding a New Message

1. **Choose the prefix**: `C_CMD_*` if no return value, `C_FUN_*` if a return value is needed, `J_EVT_*` for JS-initiated notifications.
2. **JS side**: Register a handler via `UiDriver.registerEventHandler("C_CMD_YOUR_MSG", handler)` in both `app.js` and `app_monaco.js`.
3. **C++ side**: Call `asyncSendMessageWithResultP("C_FUN_YOUR_MSG", data)` (or the legacy API) from `editor.cpp` or a higher-level wrapper method in `editor.h`.
4. **Handle the reply**: If async, `.then()` on the returned promise or connect to `asyncReplyReceived` signal.
5. **JS→C++ events**: Just call `UiDriver.sendMessage("J_EVT_YOUR_MSG", data)` from JS and handle the parsed message in `Editor::on_proxyMessageReceived()`.

## Dual Editor Engine

Notepadqq ships two editor engines:

- **CodeMirror** (default): `index.html` + `app.js`
- **Monaco** (VS Code's editor): `index_monaco.html` + `app_monaco.js`

Both implement the identical message protocol. The C++ side selects the engine via `Editor::useMonaco()` (`editor.cpp:33`). Any new message must be implemented in **both** `app.js` and `app_monaco.js`.

## Key Architecture Notes

- QWebChannel serialises all values as `QVariant` (C++) ↔ plain JS values (JSON-compatible types).
- JS-to-C++ messages use a callback parameter `function(ret) {}` even when the return value is unused — the QWebChannel bridge requires this for the method call to work.
- The `UiDriver` maintains a `msgQueue` for messages sent before the WebChannel is ready; they are flushed once `QWebChannel` initialises.
- C++ messages sent before the editor fires `J_EVT_READY` are queued and delivered once `editorReady` signal fires (`editor.cpp:419-426`).
