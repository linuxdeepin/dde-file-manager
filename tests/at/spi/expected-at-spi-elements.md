# Expected AT-SPI Elements — dde-file-manager

## Naming Convention

All AT-SPI names follow PascalCase, English only, max 64 chars, alphanumeric + underscore only.

Pattern: `<ClassName>_<WidgetRole>[_<Suffix>]`

For existing code with `setObjectName` but no `setAccessibleName`, the accessible name will reuse the object name (adding `setAccessibleName` alongside `setObjectName`).

## Existing Named Elements (Already Have setObjectName or setAccessibleName)

| AT-SPI Name | Role | File:Line | Source |
|-------------|------|-----------|--------|
| CentralView | Form | `filemanagerwindow.cpp:1308` | setObjectName |
| PropertyDialog-QScrollArea | ScrollPane | `filepropertydialog.cpp:68` | setObjectName |
| OpenWithDialog-QScrollArea | ScrollPane | `openwithdialog.cpp:270` | setObjectName |
| OpenWithListWidget | List | `openwithwidget.cpp:44` | setObjectName |
| FileDialogStatusBarAcceptButton | Button | `filedialogstatusbar.cpp:279` | setObjectName |
| StatusBar | StatusBar | `filepreviewdialog.cpp:264` | setObjectName |
| OptionQSlider | Slider | `settingdialog.cpp:335` | setAccessibleName |
| verticalScrollBar | ScrollBar | `sheetbrowser.cpp:51` | setAccessibleName |
| horizontalScrollBar | ScrollBar | `sheetbrowser.cpp:54` | setAccessibleName |
| View_ImageList | List | `thumbnailwidget.cpp:32` | setAccessibleName |
| Placeholder | Label | `titlebarwidget.cpp:315` | setObjectName |
| NameTextEdit | Text | `editstackedwidget.cpp:38` | setObjectName |
| AlertTooltip | ToolTip | Multiple files | setObjectName |
| tagActionWidget | Widget | `tagcolorlistwidget.cpp:22` | setObjectName |
| titleBar | Widget | `collectiontitlebar.cpp:191` | setObjectName |
| RenameBarLabel | Label | `renamebar_p.cpp:76-127` | setObjectName |
| WallpaperList-QScrollArea | ScrollPane | `wallpaperlist.cpp:404` | setObjectName |
| SectionKeyLabel | Label | `sharecontrolwidget.cpp:63` | setObjectName |

## New Elements to Add (Gaps)

See [at-spi-implementation-checklist.md](./at-spi-implementation-checklist.md) for the complete gap list with implementation details.