#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <QScrollArea>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QStringList>
#include <QMouseEvent>
#include <QShowEvent>
#include "fileitem.h"
#include "utils/durl.h"
#include "models/abstractfileinfo.h"
#include "deviceinfo/udiskdeviceinfo.h"

class FlowLayout;

class TitleLine: public QFrame
{
    Q_OBJECT

public:
    explicit TitleLine(const QString& title, QWidget *parent = 0);

    void initUI();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QString m_title;
    QLabel* m_titleLable=NULL;
    QLabel* m_lineLable=NULL;

};

class ComputerViewItem: public FileIconItem
{
    Q_OBJECT

public:
    explicit ComputerViewItem(QWidget *parent = 0);

    QIcon getIcon(int size);
    AbstractFileInfoPointer info() const;
    void setInfo(const AbstractFileInfoPointer &info);

    UDiskDeviceInfo *deviceInfo() const;
    void setDeviceInfo(UDiskDeviceInfo *deviceInfo);

    int windowId();

    int iconSize() const;
    void setIconSize(int size);

    bool checked() const;
    void setChecked(bool checked);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString name() const;
    void setName(const QString &name);

public slots:
    void updateStatus();

signals:
    void checkChanged(bool isChecked);

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    void setIconSizeState(int iconSize, QIcon::Mode mode = QIcon::Normal);
    AbstractFileInfoPointer m_info;
    UDiskDeviceInfo* m_deviceInfo;
    int m_iconSize = 64;
    bool m_checked = false;
    QString m_name;
    QString m_displayName;
};


class ComputerView : public QScrollArea
{
    Q_OBJECT

public:
    explicit ComputerView(QWidget *parent = 0);
    ~ComputerView();

    static DUrl url();

    void initData();
    void initUI();
    void initConnect();

    void loadSystemItems();
    void loadNativeItems();
    void loadCustomItems();
    void loadCustomItemsByNameUrl(const QString& id, const QString& url);

    bool isDiskConfExisted();
    QString getDiskConfPath();

signals:

public slots:
    void volumeAdded(UDiskDeviceInfo * device);
    void volumeRemoved(UDiskDeviceInfo * device);
    void mountAdded(UDiskDeviceInfo * device);
    void mountRemoved(UDiskDeviceInfo * device);
    void enlargeIcon();
    void shrinkIcon();
    void resizeItemBySizeIndex(int index);

protected:
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void showEvent(QShowEvent* event);

private:
    TitleLine* m_systemTitleLine=NULL;
    FlowLayout* m_systemFlowLayout = NULL;
    TitleLine* m_nativeTitleLine=NULL;
    FlowLayout* m_nativeFlowLayout = NULL;
    TitleLine* m_removableTitleLine=NULL;
    FlowLayout* m_removableFlowLayout = NULL;
    QTimer* m_testTimer;

    QList<int> m_iconSizes;
    int m_currentIconSizeIndex = 1;

    QStringList m_systemPathKeys;

    QMap<QString, ComputerViewItem*> m_systemItems;
    QMap<QString, ComputerViewItem*> m_nativeItems;
    QMap<QString, ComputerViewItem*> m_removableItems;
};

#endif // COMPUTERVIEW_H
