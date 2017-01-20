#ifndef DIALOG_H
#define DIALOG_H

#include <QFrame>
#include <QColor>
#include <QLabel>
#include <QStackedWidget>

#include <DTitlebar>
#include <dwidget.h>
#include <QDialog>
#include <dplatformwindowhandle.h>

#include "widgets/progressline.h"

#include "mainpage.h"
#include "warnpage.h"
#include "formatingpage.h"
#include "finishpage.h"
#include "errorpage.h"

#include "../partman/partitionmanager.h"

#define BORDER_WIDTH 1
#define BORDER_COLOR QColor(0,0,0,120)
#define BORDER_RADIUS 5
#define WINDOW_SIZE QSize(320, 410)

#define SHADOW_BLUR_RADIUS 10
#define SHADOW_COLOR QColor(0,0,0,20)

class QComboBox;
class QProgressBar;
class QPushButton;

DWIDGET_USE_NAMESPACE

class MainWindow : public QDialog
{
    Q_OBJECT
    enum FormatStep{
        Normal = 0,
        Warn,
        Formating,
        Finished,
        FormattError,
        RemovedWhenFormattingError
    };

public:
    MainWindow(const QString& path, QWidget *parent = 0);
    ~MainWindow();

    void initUI();
    void initStyleSheet();
    void initConnect();
    void formartDevice();
    void unMountDevice();
    bool checkBackup();

signals:
    void taskFinished(const bool& result);

public slots:
    void nextStep();
    void onFormatingFinished(const bool& successful);
    void preHandleTaskFinished(const bool& result);

private:
    QPushButton* m_comfirmButton = NULL;
    QStackedWidget* m_pageStack = NULL;
    FormatStep m_currentStep = Normal;
    MainPage* m_mainPage = NULL;
    WarnPage* m_warnPage = NULL;
    FormatingPage* m_formatingPage = NULL;
    FinishPage* m_finishPage = NULL;
    ErrorPage* m_errorPage = NULL;
    QString m_formatPath;
    QString m_formatType;

    PartMan::PartitionManager partitionManager;

};

#endif // DIALOG_H
