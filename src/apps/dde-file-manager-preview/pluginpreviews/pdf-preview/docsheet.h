// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCSHEET_H
#define DOCSHEET_H

#include "preview_plugin_global.h"

#include "model.h"

#include <DSplitter>

#include <QReadWriteLock>

class QTemporaryDir;

namespace plugin_filepreview {
class SheetBrowser;
class SheetRenderer;
class SheetSidebar;
class EncryptionPage;

struct SheetOperation
{
    LayoutMode layoutMode = SinglePageMode;
    Rotation rotation = kRotateBy0;
    qreal scaleFactor = 0.9;
    bool sidebarVisible = false;
    int sidebarIndex = 0;
    int currentPage = 1;
};

class DocSheet : public Dtk::Widget::DSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(DocSheet)

    friend class SheetSidebar;
    friend class SheetBrowser;
    friend class Database;

public:
    explicit DocSheet(const FileType &fileType, const QString &filePath, QWidget *parent = nullptr);

    ~DocSheet() override;

public:
    /**
     * @brief existSheet
     * 进程是否存在该文档
     * @param sheet
     * @return
     */
    static bool existSheet(DocSheet *sheet);

    /**
     * @brief getSheet
     * 根据uuid返回sheet
     * @param filePath
     * @return
     */
    static DocSheet *getSheetByFilePath(QString filePath);

    /**
     * @brief getSheets
     * 返回所有sheet
     * @return
     */
    static QList<DocSheet *> getSheets();

    /**
     * @brief 全局对象判断读写锁
     */
    static QReadWriteLock lockReadWrite;

    static QStringList uuidList;

    static QList<DocSheet *> sheetList;

public:
    /**
     * @brief 阻塞式打开文档
     * @param password
     */
    bool openFileExec(const QString &password);

    /**
     * @brief 异步式打开文档，完成后会发出sigFileOpened
     * @param password 文档密码
     * @return
     */
    void openFileAsync(const QString &password);

    /**
     * @brief 获取文档总页数
     * @return
     */
    int pageCount();

    /**
     * @brief currentPage
     * 获取当前文档页数 base 1
     * @return
     */
    int currentPage();

    /**
     * @brief currentPage
     * 获取当前文档索引值 base 0
     * @return
     */
    int currentIndex();

    /**
     * @brief jumpToIndex
     * 跳转到索引
     * @param index 索引值
     */
    void jumpToIndex(int index);

    /**
     * @brief jumpToPage
     * 跳转到页
     * @param page 页码
     */
    void jumpToPage(int page);

    /**
     * @brief setLayoutMode
     * 批量布局类型
     * @param mode
     */
    void setLayoutMode(LayoutMode mode);

    /**
     * @brief thumbnail
     * 获取缩略图(需要先设置)
     * @param index
     * @return
     */
    QPixmap thumbnail(int index);

    /**
     * @brief setThumbnail
     * @param index
     * @param pixmap
     */
    void setThumbnail(int index, QPixmap pixmap);

    /**
     * @brief getImage
     * @param index 索引
     * @param width
     * @param height
     * @param slice
     * @return
     */
    QImage getImage(int index, int width, int height, const QRect &slice = QRect());

    /**
     * @brief scaleFactorList
     * 获取缩放因子列表
     * @return
     */
    QList<qreal> scaleFactorList();

    /**
     * @brief maxScaleFactor
     * 获取最大的缩放因子 (产品文档中根据文档大小对缩放因子有限制)
     * @return
     */
    qreal maxScaleFactor();

    /**
     * @brief format
     * 获取类型名称
     * @return
     */
    QString format();

    /**
     * @brief operation
     * 获取用户操作
     * @return
     */
    SheetOperation operation() const;

    /**
     * @brief fileType
     * 获取文件类型
     * @return
     */
    FileType fileType();

    /**
     * @brief filePath
     * 用户想要打开的文档路径
     * @return
     */
    QString filePath();

    /**
     * @brief openedFilePath
     * 实际打开的文档路径
     * @return
     */
    QString openedFilePath();

    /**
     * @brief setSidebarVisible
     * 设置左侧栏显示
     * @param isVisible 显示
     * @param notify 是否通知操作变化
     */
    void setSidebarVisible(bool isVisible, bool notify = true);

    /**
     * @brief opened
     * 获取是否被打开
     * @return
     */
    bool opened();

    /**
     * @brief 获取PAGE真实大小
     * @param index
     * @return
     */
    QSizeF pageSizeByIndex(int index);

    /**
     * @brief renderer
     * 获取渲染器
     * @return
     */
    SheetRenderer *renderer();

    /**
     * @brief showEncryPage
     * 显示解锁页面
     */
    void showEncryPage();

private:
    /**
     * @brief operationRef
     * 获取操作的引用(可以直接修改)
     * @return
     */
    SheetOperation &operationRef();

    /**
     * @brief 重置sidebar,browser的parent
     */
    void resetChildParent();

private slots:

    /**
     * @brief onOpened
     * 打开结果
     * @param error
     */
    void onOpened(Document::Error error);

signals:

    /**
     * @brief sigPageModified
     * 页码被修改
     */
    void sigPageModified(int index);

    /**
     * @brief 文档被打开
     * @param result 结果
     * @param error 错误
     */
    void sigFileOpened(DocSheet *sheet, Document::Error error);

private slots:
    /**
     * @brief onBrowserPageChanged
     * 当前页变化处理
     * @param page 最新页码
     */
    void onBrowserPageChanged(int page);

    /**
     * @brief onExtractPassword
     * 密码执行处理
     * @param password
     */
    void onExtractPassword(const QString &password);

private:
    /**
     * @brief setAlive
     * 设置当前sheet是否存活
     * @param alive
     */
    void setAlive(bool alive);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void childEvent(QChildEvent *c) override;

private:
    SheetOperation sheetOperation;

    //document
    Document *document { nullptr };

    SheetSidebar *sheetSidebar { nullptr };   //操作左侧ui
    SheetBrowser *sheetBrowser { nullptr };   //操作右侧ui
    SheetRenderer *sheetRenderer { nullptr };   //数据渲染器

    QString currentFilePath;
    QString docPassword;
    FileType currentFileType;
    QString uuidFile;
    QTemporaryDir *temporaryDir { nullptr };   //存放临时数据
    QMap<int, QPixmap> thumbnailMap;

    bool documentChanged { false };
    bool bookmarkChanged { false };
    bool fullSiderBarVisible { false };
    EncryptionPage *encrytionPage { nullptr };
};
}
#endif   // DOCSHEET_H
