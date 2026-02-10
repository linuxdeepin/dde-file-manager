// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGET_H
#define WIDGET_H

#include "preview_plugin_global.h"

#include "model.h"

#include <QWidget>
#include <QStackedLayout>
#include <QMap>

namespace plugin_filepreview {
class DocSheet;

struct recordSheetPath
{
    recordSheetPath() {}

    void insertSheet(DocSheet *sheet);

    int indexOfFilePath(const QString &filePath);

    void removeSheet(DocSheet *sheet);

    QList<DocSheet *> getSheets();

    QMap<DocSheet *, QString> docSheetMap;
};

class PdfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PdfWidget(QWidget *parent = nullptr);
    ~PdfWidget();

    /**
     * @brief 根据路径增加一个sheet页后异步进行读取，成功保留，不成功删除,如果路径已经存在则切换到对应的页面
     * @param filePath 需要添加的文档路径
     */
    void addFileAsync(const QString &filePath);

    /**
     * @brief addSheet直接添加一个文档页
     * @param sheet 添加sheet
     */
    void addSheet(DocSheet *sheet);

    /**
     * @brief enterSheet 进入一个文档页，会进行信号父窗口等重新绑定
     * @param sheet 添加sheet
     */
    void enterSheet(DocSheet *sheet);

    /**
     * @brief leaveSheet 离开一个文档页，会进行信号父窗口等解绑
     * @param sheet 删除sheet
     */
    void leaveSheet(DocSheet *sheet);

    /**
     * @brief closeSheet 关闭sheet,成功返回true，失败或者取消返回false。
     * @param sheet (关闭成功后会被删除)
     * @param needToBeSaved
     * @return 关闭是否成功
     */
    bool closeSheet(DocSheet *sheet);

    /**
     * @brief closeAllSheets 进行关闭，会逐个关闭每个sheet，全部关闭完成返回成功
     * @param needToBeSaved 是否需要提示保存
     * @return 关闭是否成功
     */
    bool closeAllSheets();

public slots:

    /**
     * @brief 文档被打开后进行处理
     * @param sheet 被打开的文档
     * @param result 打开结果
     */
    void onOpened(DocSheet *sheet, Document::Error error);

private:
    QStackedLayout *stackedLayout { nullptr };

    recordSheetPath sheetMap;
};
}
#endif   // WIDGET_H
