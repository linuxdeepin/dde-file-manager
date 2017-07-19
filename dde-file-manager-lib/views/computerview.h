#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <QScrollArea>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QStringList>
#include <QMouseEvent>
#include <QShowEvent>
#include <QIcon>
#include <QFrame>

#include "fileitem.h"
#include "progressline.h"
#include "durl.h"
#include "dabstractfileinfo.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dstatusbar.h"
#include "dfmbaseview.h"

DFM_USE_NAMESPACE

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

class DScrollArea: public QScrollArea{
public:
    explicit DScrollArea(QWidget* parent =0);

protected:
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
};

class ComputerViewItem: public FileIconItem
{
    Q_OBJECT

public:
    explicit ComputerViewItem(QWidget *parent = 0);

    QIcon getIcon(int size);
    DAbstractFileInfoPointer info() const;
    void setInfo(const DAbstractFileInfoPointer &info);

    UDiskDeviceInfoPointer deviceInfo() const;
    void setDeviceInfo(UDiskDeviceInfoPointer deviceInfo);

    inline ProgressLine* getProgressLine()
    { return m_progressLine; }

    int windowId();

    int iconSize() const;
    void setIconSize(int size);

    bool checked() const;
    void setChecked(bool checked);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString name() const;
    void setName(const QString &name);

    bool getHasMemoryInfo() const;
    void setHasMemoryInfo(bool hasMemoryInfo);

    int getPixelWidth() const;
    void setPixelWidth(int pixelWidth);
    void updateIconPixelWidth();

    DUrl getUrl() const;

public slots:
    void updateStatus();

signals:
    void checkChanged(bool isChecked);

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    bool event(QEvent *event);

private:
    void adjustPosition();
    void setIconSizeState(int iconSize, QIcon::Mode mode = QIcon::Normal);
    ProgressLine* m_progressLine;
    QLabel* m_sizeLabel;
    DAbstractFileInfoPointer m_info;
    UDiskDeviceInfoPointer m_deviceInfo;
    int m_iconSize = 64;
    bool m_checked = false;
    QString m_name;
    QString m_displayName;
    bool m_hasMemoryInfo = false;
    int m_pixelWidth;
};


class ComputerView : public QFrame, public DFMBaseView
{
    Q_OBJECT

public:
    explicit ComputerView(QWidget *parent = 0);
    ~ComputerView();

    static int ViewInstanceCount;

    static QString scheme();
    QString viewId() const;

    void initData();
    void initUI();
    void initConnect();

    void loadSystemItems();
    void loadNativeItems();
    void loadCustomItems();
    void loadCustomItemsByNameUrl(const QString& id, const QString& rootUrl);
    void updateStatusBar();

    bool isDiskConfExisted();
    QString getDiskConfPath();

    void loadViewState();
    void saveViewState();

    QWidget *widget() const Q_DECL_OVERRIDE;
    DUrl rootUrl() const Q_DECL_OVERRIDE;
    bool setRootUrl(const DUrl &url) Q_DECL_OVERRIDE;

public slots:
    void volumeAdded(UDiskDeviceInfoPointer device);
    void volumeRemoved(UDiskDeviceInfoPointer device);
    void mountAdded(UDiskDeviceInfoPointer device);
    void mountRemoved(UDiskDeviceInfoPointer device);
    void updateComputerItemByDevice(UDiskDeviceInfoPointer device);
    void enlargeIcon();
    void shrinkIcon();
    void resizeAllItemsBySizeIndex(int index);
    void updateItemBySizeIndex(const int& index, ComputerViewItem* item);

protected:
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent* event);

private:
    QString m_viewId;
    TitleLine* m_systemTitleLine=NULL;
    FlowLayout* m_systemFlowLayout = NULL;
    TitleLine* m_nativeTitleLine=NULL;
    FlowLayout* m_nativeFlowLayout = NULL;
    TitleLine* m_removableTitleLine=NULL;
    FlowLayout* m_removableFlowLayout = NULL;
    QTimer* m_testTimer;

    DStatusBar* m_statusBar;
    DScrollArea* m_contentArea;

    QList<int> m_iconSizes;
    int m_currentIconSizeIndex = 1;

    QStringList m_systemPathKeys;

    QMap<QString, ComputerViewItem*> m_systemItems;
    QMap<QString, ComputerViewItem*> m_nativeItems;
    QMap<QString, ComputerViewItem*> m_removableItems;
};

#endif // COMPUTERVIEW_H
