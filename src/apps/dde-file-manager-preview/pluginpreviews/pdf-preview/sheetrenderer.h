// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHEETRENDERER_H
#define SHEETRENDERER_H

#include "preview_plugin_global.h"

#include "docsheet.h"
#include "model.h"

namespace plugin_filepreview {
/**
 * @brief The SheetRenderer class
 * 用于加载文档
 */
class SheetRenderer : public QObject
{
    Q_OBJECT
public:
    explicit SheetRenderer(DocSheet *parent);

    ~SheetRenderer();

    /**
     * @brief openFileExec
     * 阻塞式打开文档
     * @param password
     */
    bool openFileExec(const QString &password);

    /**
     * @brief openFileAsync
     * 异步式打开文档，完成后会发出sigFileOpened
     * @param password 文档密码
     * @return
     */
    void openFileAsync(const QString &password);

    /**
     * @brief opened
     * 是否已打开
     * @return
     */
    bool opened();

    /**
     * @brief handleOpened
     * 处理打开
     * @param result
     * @param error
     */
    void handleOpened(Document::Error error, Document *document, QList<Page *> pages);

    /**
     * @brief getPageCount
     * 获取页数量
     * @return
     */
    int getPageCount();

    /**
     * @brief getImage
     * 获取图片
     * @param index 索引
     * @param width
     * @param height
     * @param slice
     * @return
     */
    QImage getImage(int index, int width, int height, const QRect &slice = QRect());

    /**
     * @brief 文档原始大小
     * param index 对应索引
     * @return
     */
    QSizeF getPageSize(int index) const;

signals:
    /**
     * @brief sigOpened
     * 打开完成
     * @param error
     */
    void sigOpened(Document::Error error);

private:
    DocSheet *docSheet { nullptr };
    Document::Error docError { Document::kNoError };
    bool pageLabelLoaded { false };   //是否已经加载page label
    QMap<QString, int> docPageIndex {};   // 文档下标页码
    Document *documentObj { nullptr };
    QList<Page *> pageList {};
};
}
#endif   // SHEETRENDERER_H
