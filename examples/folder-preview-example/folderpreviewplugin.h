// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERPREVIEWPLUGIN_H
#define FOLDERPREVIEWPLUGIN_H

#include <dfm-base/interfaces/abstractfilepreviewplugin.h>

namespace example_folderprev {

/**
 * @brief Plugin factory for folder preview.
 *
 * Responds to MIME type "inode/directory" and creates a FolderPreview
 * instance that visualises the immediate contents of a directory.
 */
class FolderPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid
                      FILE "dde-folder-preview-plugin.json")

public:
    explicit FolderPreviewPlugin(QObject *parent = nullptr);

    /**
     * @brief Creates a FolderPreview for the given MIME key.
     * @param key  MIME type string, expected "inode/directory"
     * @return New FolderPreview instance; the caller takes ownership.
     */
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) override;
};

}   // namespace example_folderprev
#endif   // FOLDERPREVIEWPLUGIN_H
