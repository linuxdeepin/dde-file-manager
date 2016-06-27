#ifndef PROPERTYDIALOG_H
#define PROPERTYDIALOG_H

#include "basedialog.h"
#include "../models/abstractfileinfo.h"
#include <QLabel>
#include <QCloseEvent>
QT_BEGIN_NAMESPACE
class QTextEdit;
class QFrame;
class QCheckBox;
class QTimer;
class QListWidget;
class QButtonGroup;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DExpandGroup;
DWIDGET_END_NAMESPACE

class AbstractFileInfo;
class DUrl;

DWIDGET_USE_NAMESPACE

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




class ComupterFolderSizeWorker: public QObject
{
    Q_OBJECT
public:
    explicit ComupterFolderSizeWorker(const QString& dir);

public:
    void coumpueteSize();
    QString dir() const;
    void setDir(const QString &dir);

signals:
    void sizeUpdated(qint64 size);

public:
    void updateSize();

private:
    QString m_dir;
    qint64 m_size;
};



class PropertyDialog : public BaseDialog
{
    Q_OBJECT

public:
    explicit PropertyDialog(const DUrl &url, QWidget *parent = 0);

public:
    void updateFolderSize(qint64 size);
    void startComputerFolderSize(const QString& dir);
    void toggleFileExecutable(bool isChecked);
    DUrl getUrl();
    int getFileCount();
    qint64 getFileSize();

public slots:
    void raise();

signals:
    void requestStartComputerFolderSize();
    void closed(const DUrl& url);
    void aboutToClosed(const DUrl& url);
    void raised();

protected:
    void closeEvent(QCloseEvent* event);

private:
    DUrl m_url;
    qint64 m_size = 0;
    int m_fileCount = 0;
    QLabel *m_icon = NULL;
    QTextEdit *m_edit = NULL;
    QCheckBox * m_executableCheckBox = NULL;
    SectionValueLabel* m_folderSizeLabel = NULL;
    QFrame *m_basicInfoFrame = NULL;
    QFrame *m_OpenWithFrame = NULL;
    QListWidget* m_OpenWithListWidget = NULL;
    QButtonGroup* m_OpenWithButtonGroup;

    DExpandGroup *addExpandWidget(const QStringList &titleList);


    QFrame *createBasicInfoWidget(const AbstractFileInfoPointer &info);
    QListWidget *createOpenWithListWidget(const AbstractFileInfoPointer &info);
    QFrame *createAuthorityManagermentWidget(const AbstractFileInfoPointer &info);

};

#endif // PROPERTYDIALOG_H
