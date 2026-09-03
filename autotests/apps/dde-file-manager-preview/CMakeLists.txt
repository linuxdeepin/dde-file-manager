# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Unit tests for src/apps/dde-file-manager-preview/pluginpreviews plugins.
set(DFM_PREVIEW_PLUGIN_DIR "${DFM_SOURCE_DIR}/apps/dde-file-manager-preview/pluginpreviews")
add_subdirectory(dciicon-preview)
add_subdirectory(image-preview)
add_subdirectory(markdown-preview)
add_subdirectory(text-preview)
add_subdirectory(music-preview)
add_subdirectory(pdf-preview)
add_subdirectory(video-preview)
