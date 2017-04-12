#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "basedialog.h"
#include "dabstractfileinfo.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dfmevent.h"
#include <QLabel>
#include <QCloseEvent>
#include <QTextEdit>
#include <QStackedWidget>
#include <QPushButton>
#define EXTEND_FRAME_MAXHEIGHT 160

QT_BEGIN_NAMESPACE
class QFrame;
class QCheckBox;
class QTimer;
class QListWidget;
class QButtonGroup;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DExpandGroup;
DWIDGET_END_NAMESPACE

class DAbstractFileInfo;
class UDiskDeviceInfo;
class DUrl;
class ShareInfoFrame;
class FilesSizeWorker;

DWIDGET_USE_NAMESPACE


class NameTextEdit: public QTextEdit
{
    Q_OBJECT

public:
    explicit NameTextEdit(const QString &text="", QWidget *parent=0);

    bool isCanceled() const;
    void setIsCanceled(bool isCanceled);

signals:
    void editFinished();

public slots:
    void setPlainText(const QString & text);

protected:
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;

private:
    bool m_isCanceled = false;
};


class GroupTitleLabel: public QLabel
{
    Q_OBJECT

public:
    explicit GroupTitleLabel(const QString &text="", QWidget *parent=0, Qt::WindowFlags f=0);
};

class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text="", QWidget *parent=0, Qt::WindowFlags f=0);
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text="", QWidget *parent=0, Qt::WindowFlags f=0);
};


class PropertyDialog : public DDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(const DFMEvent &event, const DUrl url, QWidget *parent = 0);

public:
    void initUI();
    void initConnect();
    void startComputerFolderSize(const DUrl &url);
    void toggleFileExecutable(bool isChecked);
    DUrl getUrl();
    int getFileCount();
    qint64 getFileSize();

    DExpandGroup *expandGroup() const;
    int contentHeight() const;
    
    void loadPluginExpandWidgets();

public slots:
    void raise();
    void updateFolderSize(qint64 size);
    void renameFile();
    void showTextShowFrame();
    void onChildrenRemoved(const DUrl &fileUrl);
    void flickFolderToLeftsidBar();
    void onOpenWithBntsChecked(QAbstractButton *w);
    void onExpandChanged(const bool& e);

signals:
    void requestStartComputerFolderSize();
    void closed(const DUrl& url);
    void aboutToClosed(const DUrl& url);
    void raised();

protected:
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    DFMEvent m_fmevent;
    DUrl m_url;
    QString m_absolutePath;
    qint64 m_size = 0;
    bool m_editDisbaled = false;
    int m_fileCount = 0;
    QLabel *m_icon = NULL;
    NameTextEdit *m_edit = NULL;
    QStackedWidget *m_editStackWidget = NULL;
    QFrame* m_textShowFrame = NULL;
    QPushButton* m_editButton = NULL;
    QCheckBox * m_executableCheckBox = NULL;
    SectionValueLabel* m_folderSizeLabel = NULL;
    QFrame *m_basicInfoFrame = NULL;
    ShareInfoFrame* m_shareinfoFrame = NULL;
    QFrame *m_localDeviceInfoFrame = NULL;
    QFrame *m_deviceInfoFrame = NULL;
    QFrame *m_OpenWithFrame = NULL;
    QListWidget* m_OpenWithListWidget = NULL;
    QButtonGroup* m_OpenWithButtonGroup = NULL;
    DExpandGroup* m_expandGroup = NULL;
    FilesSizeWorker* m_sizeWorker = NULL;
    QVBoxLayout* m_mainLayout = NULL;

    DExpandGroup *addExpandWidget(const QStringList &titleList);

    void initTextShowFrame(const QString& text);
    QFrame *createBasicInfoWidget(const DAbstractFileInfoPointer &info);
    ShareInfoFrame* createShareInfoFrame(const DAbstractFileInfoPointer &info);
    QFrame *createLocalDeviceInfoWidget(const DUrl& url);
    QFrame *createDeviceInfoWidget(UDiskDeviceInfoPointer info);
    QListWidget *createOpenWithListWidget(const DAbstractFileInfoPointer &info);
    QFrame *createAuthorityManagermentWidget(const DAbstractFileInfoPointer &info);
};

#endif // PROPERTYDIALOG_H
