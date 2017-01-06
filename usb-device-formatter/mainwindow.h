#ifndef DIALOG_H
#define DIALOG_H

#include <QFrame>
#include <QColor>
#include <QLabel>
#include <QStackedWidget>

#include <DTitlebar>
#include <dwidget.h>
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
#define WINDOW_SIZE QSize(320, 420)

#define SHADOW_BLUR_RADIUS 10
#define SHADOW_COLOR QColor(0,0,0,20)

class QComboBox;
class QProgressBar;
class QPushButton;

DWIDGET_USE_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT
    enum FormatStep{
        Normal = 0,
        Warn,
        Formating,
        Finished,
        Error
    };

public:
    MainWindow(const QString& path, QWidget *parent = 0);
    ~MainWindow();

    void initUI();
    void initStyleSheet();
    void initConnect();
    void formartDevice();
    void unMountDevice();

signals:
    void taskFinished(const bool& result);

public slots:
    void nextStep();
    void onFormatingFinished(const bool& successful);
    void preHandleTaskFinished(const bool& result);

private:
    QPushButton* m_comfirmButton;
    QStackedWidget* m_pageStack;
    FormatStep m_currentStep = Normal;
    MainPage* m_mainPage;
    WarnPage* m_warnPage;
    FormatingPage* m_formatingPage;
    FinishPage* m_finishPage;
    ErrorPage* m_errorPage;
    QString m_formatPath;
    QString m_formatType;

    PartMan::PartitionManager partitionManager;

};

#endif // DIALOG_H
