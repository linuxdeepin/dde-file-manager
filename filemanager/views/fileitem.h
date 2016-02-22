#ifndef FILEITEM_H
#define FILEITEM_H

#include <QFrame>
#include "dbusinterface/dbustype.h"

class QLabel;
class GrowingElideTextEdit;
class QPixmap;

class FileItem : public QFrame
{
    Q_OBJECT
public:
    explicit FileItem(const QString& name, const QString& icon, QWidget *parent = 0);
    ~FileItem();

    static int OneTextHeight;
    static int DoubleTextHeight;

    static QPixmap turnSvgToQPixmap(const QString& svgFile, int width=24, int height=24);
    static QPixmap applyShadowToPixmap(const QString filename);

    void init();
    void initUI();
    void initReadableLabel();
    void initPermissionLabel();
    void initConnect();


    QString getFileIcon();
    QPixmap getFileIconPixmap();
    QString getFileName();
    QString getUrl();
    QString getRawUrl();
    bool isChecked();
    bool isHover();
    bool isCuted();
    bool isEditing();
    bool isShowSimpleMode();

    FileItemInfo& getFileItemInfo();
    GrowingElideTextEdit* getTextEdit();
    QLabel* getIconLabel();

    void addImageShadow();
    void addTextShadow();
    void disabledTextShadow();
    void enableTextShadow();

signals:
    void fileIconChanged(QString icon);
    void fileIconChanged(QPixmap& icon);
    void fileNameChanged(QString name);
    void hoverChanged(bool hover);
    void checkedChanged(bool checked);

public slots:
    void setFileIcon(QString icon);
    void setFileIcon(QPixmap icon);
    void setFileName(QString name);
    void setChecked(bool checked, bool isExpanded=false);
    void setHover(bool hover);
    void setCuted();
    void cancelCuted();
    void setEditing(bool flag);
    void showEditing();
    void setUrl(QString url);
    void setRaWUrl(QString url);
    void setFileItemInfo(FileItemInfo& fileItemInfo);

    void showFullWrapName();
    void showSimpWrapName();
    void updateHeight(int textHeight);

    void showContextMenu(QPoint pos);
    void handleCut(QString url);
    void handleCopy(QString url);
    void handlePaste(QString url);
    void handleSelectAll(QString url);

    void setUserReadOnly(bool isUserReadOnly);
    void setUserReadPermisson_000(bool isUserPermisson_000);
    void setReadable(bool isReadable);
    void show();

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

private:
    QString m_fileName;
    QString m_fileIcon;
    QPixmap m_fileIconPixmap;
    QString m_url;
    QString m_rawUrl;
    QLabel* m_iconLabel = NULL;
    QLabel* m_permissionIndicatorLabel = NULL;
    QLabel* m_unReadableIndicatorLabel = NULL;
    GrowingElideTextEdit* m_textedit = NULL;
    bool m_isCuted = false;
    bool m_isInAppGroup = false;
    bool m_mouseRightRelease = false;
    bool m_checked = false;
    bool m_hover = false;
    bool m_isEditing = false;
    bool m_isReadable = false;
    bool m_isUserReadOnly = false;
    bool m_isUserPermisson_000 = false;
    FileItemInfo m_fileItemInfo;
};

#endif // FILEITEM_H
