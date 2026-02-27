# Folder Preview Plugin Example

A DDE File Manager preview plugin that provides a rich visual preview of
directory contents when the user presses **Space** on a folder.

## Preview UI

```
┌─────────────────────────────────────────────────────────────┐
│  📁  Documents                                              │
│      /home/alice/Documents                                  │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌────────────────┐            │
│  │    📄    │  │    📁    │  │      💾        │            │
│  │    12    │  │    3     │  │   4.7 MB       │            │
│  │  Files   │  │ Folders  │  │  Total Size    │            │
│  └──────────┘  └──────────┘  └────────────────┘            │
├─────────────────────────────────────────────────────────────┤
│  Name               Type          Size      Modified        │
│  ───────────────────────────────────────────────────────── │
│  📁 Archive          Folder        —        2024-11-01      │
│  📁 Reports          Folder        —        Today 09:35     │
│  📄 budget.xlsx      Spreadsheet   88.2 KB  Yesterday 18:00 │
│  📄 notes.md         Markdown      4.1 KB   Today 10:22     │
│  📄 photo.jpg        JPEG image    2.3 MB   2024-10-15      │
│  …                                                          │
└─────────────────────────────────────────────────────────────┘
```

### Features

- **Header panel** – folder icon, full name, absolute path
- **Stat badges** – file count, sub-folder count, cumulative file size
- **File list** – sortable columns: Name · Type · Size · Modified
- **Async scanning** – powered by `QtConcurrent`, UI stays responsive
- **Adaptive theme** – automatically adjusts colours for dark / light mode

## Project Layout

```
folder-preview-plugin/
├── CMakeLists.txt
├── README.md
├── debian/
│   ├── changelog        # Package version history
│   ├── compat           # Debhelper compatibility level
│   ├── control          # Package metadata & build dependencies
│   ├── copyright        # License information
│   └── rules            # Build rules (cmake)
└── src/
    ├── dde-folder-preview-plugin.json   # MIME type metadata
    ├── folderpreviewplugin.h/.cpp       # Qt plugin factory
    ├── folderpreview.h/.cpp             # AbstractBasePreview impl
    └── foldercontentwidget.h/.cpp       # Main UI widget
```

## Dependencies

Check `debian/control` for the full build-time dependency list, or use
`cmake` to report missing packages. For a quick start you need at least:

```bash
sudo apt install dde-file-manager-dev
```

## Build & Install

### CMake (development build)

```bash
# Configure
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr

# Build
cmake --build build -j$(nproc)

# Install (requires root)
sudo cmake --build build --target install
```

### Debian package (for distribution)

A `debian/` folder is provided to build a proper `.deb` package under
Deepin / UOS Linux:

```bash
# Install build dependencies declared in debian/control
sudo apt build-dep .

# Build the binary package (no signing)
dpkg-buildpackage -uc -us -nc
```

The resulting `.deb` installs the plugin to the correct system path
automatically.

After installation, the plugin `.so` and `.json` metadata file land in:

```
/usr/lib/<arch>/dde-file-manager/plugins/previews/
```

The preview activates automatically the next time you press **Space** on a
folder — no file manager restart is required. The file manager spawns the
preview process (`dde-file-manager-preview`) on demand, and that process
scans the plugin directory each time it starts.

## Key Implementation Notes

| Topic | Detail |
|-------|--------|
| **MIME key** | `inode/directory` – matches all local directories |
| **Async scan** | `QtConcurrent::run` + `QFutureWatcher` keeps UI thread free for the file list |
| **Total size** | `du -sb <path>` runs in parallel via `QProcess`; result includes all sub-directories recursively |
| **Icons** | `QFileIconProvider` provides system-native per-file icons |
| **MIME descriptions** | `QMimeDatabase::mimeTypeForFile` with `MatchExtension` |
| **Dark mode** | Palette lightness heuristic selects colour tokens at runtime |

## Extending This Example

- **Hidden files toggle** – add a `QAction` to the header to show/hide
  entries matching `QDir::Hidden`
- **Double-click to navigate** – call the file manager's D-Bus API to
  open a sub-folder in a new browser tab
- **Per-entry size** – for sub-directory rows in the list, run a secondary
  `du -sb <subdir>` and fill in the Size column asynchronously
