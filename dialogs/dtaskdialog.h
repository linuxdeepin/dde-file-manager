#ifndef DTASKDIALOG_H
#define DTASKDIALOG_H

#include "dmoveablewidget.h"
#include "circleprogressanimatepad.h"
#include <dcircleprogress.h>
#include <QLabel>
#include <QListWidget>
#include <QResizeEvent>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QCheckBox>
#include <ddialog.h>
#include <dplatformwindowhandle.h>
#include <DTitlebar>
#include "dcheckbox.h"

DWIDGET_USE_NAMESPACE

class MoveCopyTaskWidget : public QFrame
{
    Q_OBJECT
public:
    explicit MoveCopyTaskWidget(const QMap<QString, QString>& jobDetail, QWidget *parent = 0);
    void initUI();
    void initConnect();
    QString getTargetObj();
    QString getDestinationObj();
    int getProgress();
    float getSpeed();
    int getTimeLeft();
    QString getMessage();
    QString getTipMessage();

    void initConflictDetailFrame();
    void initButtonFrame();

signals:
    void closed(const QMap<QString, QString>& jobDetail);
    void conflictResponseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);
    void heightChanged();
    void conflictShowed(const QMap<QString, QString>& jobDetail);
    void conflictHided(const QMap<QString, QString>& jobDetail);
    void hovereChanged(const bool& hover);

public slots:
    void setTargetObj(QString targetObj);
    void setDestinationObj(QString destinationObj);
    void setProgress(int value);
    void setProgress(QString value);
    void setSpeed(float speed);
    void setTimeLeft(int time);
    void setMessage(const QString &operateStr, const QString &destinateStr);
    void setTipMessage(const QString &speedStr, const QString &remainStr);
    void handleClose();
    void handleResponse();
    void updateMessage(const QMap<QString, QString>& data);
    void updateTipMessage();
    void handleLineDisplay(const int& row, const bool &hover, const int &taskNum);

    void showConflict();
    void hideConflict();

    void updateConflictDetailFrame(const QString& originFilePath, const QString& targetFilePath);
protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

private:
    int m_progress = 0;
    float m_speed = 0.0;
    int m_timeLeft;
    QString m_targetObj;
    QString m_destinationObj;
    QString m_speedMessage;
    QString m_remainMessage;
    QString m_operateMessage;
    QString m_destinationMessage;
    QString m_sourcePath;
    QString m_targetPath;
    DCircleProgress* m_cirleWidget=NULL;
    QLabel* m_speedLabel;
    QLabel* m_remainLabel;
    QLabel* m_msg1Label;
    QLabel* m_msg2Label;

    QPushButton* m_closeButton;
    QPushButton* m_pauseBuuton;
    QPushButton* m_keepBothButton;
    QPushButton* m_skipButton;
    QPushButton* m_replaceButton;
    QMap<QString, QString> m_jobDetail;
    QMap<QString, QVariant> m_response;
    QButtonGroup* m_buttonGroup;
    QFrame* m_buttonFrame=NULL;
    QFrame* m_conflictFrame=NULL;
    QLabel* m_originIconLabel = NULL;
    QLabel* m_originTitleLabel = NULL;
    QLabel* m_originTimeLabel = NULL;
    QLabel* m_originSizeLabel = NULL;
    QLabel* m_targetIconLabel = NULL;
    QLabel* m_targetTitleLabel = NULL;
    QLabel* m_targetTimeLabel = NULL;
    QLabel* m_targetSizeLabel = NULL;

    QCheckBox* m_checkBox=NULL;
    QPushButton* m_enterButton=NULL;
    CircleProgressAnimatePad* m_animatePad;
    QLabel* m_bgLabel;
    QFrame* m_lineLabel = NULL;
};


class DTaskDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DTaskDialog(QWidget *parent = 0);
    void initUI();
    void initConnect();
    static int MaxHeight;
    QListWidget* getTaskListWidget();

signals:
    void abortTask(const QMap<QString, QString>& jobDetail);
    void conflictRepsonseConfirmed(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);
    void conflictShowed(const QMap<QString, QString>& jobDetail);
    void conflictHided(const QMap<QString, QString>& jobDetail);
    void closed();
    void currentHoverRowChanged(const int& row, const bool& hover, const int& taskNum);

public slots:
    void setTitle(QString title);
    void setTitle(int taskCount);
    void addTask(const QMap<QString, QString>& jobDetail);
    void addConflictTask(const QMap<QString, QString>& jobDetail);
    void handleTaskClose(const QMap<QString, QString>& jobDetail);
    void removeTask(const QMap<QString, QString>& jobDetail);
    void removeTaskImmediately(const QMap<QString, QString>& jobDetail);
    void delayRemoveTask(const QMap<QString, QString>& jobDetail);
    void removeTaskByPath(QString jobId);
    void handleUpdateTaskWidget(const QMap<QString, QString>& jobDetail,
                                const QMap<QString, QString>& data);
    void adjustSize();
    void moveYCenter();

    void showConflictDiloagByJob(const QMap<QString, QString>& jobDetail);
    void handleConflictResponse(const QMap<QString, QString>& jobDetail, const QMap<QString, QVariant>& response);

    void handleMinimizeButtonClick();
    void onItemHovered(const bool &hover);



protected:
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent *event);


private:
    int m_defaultWidth = 525;
    int m_defaultHeight = 120;
    QLabel* m_titleLabel=NULL;
    QPushButton* m_titleBarMinimizeButton;
    QPushButton* m_titleBarCloseButton;
    QListWidget* m_taskListWidget=NULL;
    QMap<QString, QListWidgetItem*> m_jobIdItems;
    DTitlebar* m_titleBar;
};

#endif // DTASKDIALOG_H
