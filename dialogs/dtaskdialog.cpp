#include "dtaskdialog.h"
#include "utils.h"
#include <dcircleprogress.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QCloseEvent>

#include "xutil.h"

MoveCopyTaskWidget::MoveCopyTaskWidget(const QMap<QString, QString> &jobDetail, QWidget *parent):
    QFrame(parent),
    m_jobDetail(jobDetail)
{
    initUI();
    initConnect();
    if (m_jobDetail.contains("target")){
        setTargetObj(m_jobDetail.value("target"));
    }

    if (m_jobDetail.contains("destination")){
        setDestinationObj(m_jobDetail.value("destination"));
    }
}

void MoveCopyTaskWidget::initUI(){

    m_bgLabel = new QLabel(this);
    m_bgLabel->setObjectName("Background");
    m_bgLabel->setAutoFillBackground(true);
    m_bgLabel->setWindowFlags(Qt::WindowStaysOnBottomHint);

    m_closeButton = new QPushButton;
    m_closeButton->setObjectName("StopButton");
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setAttribute(Qt::WA_NoMousePropagation);

    m_animatePad = new CircleProgressAnimatePad;
    m_animatePad->setFixedSize(54, 54);
    m_animatePad->setBackgroundColor(QColor("#E9E9E9"));
    m_animatePad->setChunkColor(QColor("#3cadff"));
    m_animatePad->setLineWidth(3);
    m_animatePad->setFontSize(14);

    m_closeButton->hide();
    setMouseTracking(true);

    m_speedLabel = new QLabel;
    m_remainLabel = new QLabel;
    m_speedLabel->setFixedHeight(18);
    m_remainLabel->setFixedHeight(18);
    m_speedLabel->setObjectName("TaskTipMessageLabel");
    m_remainLabel->setObjectName("TaskTipMessageLabel");

    m_msg1Label = new QLabel;
    m_msg2Label = new QLabel;
    m_msg1Label->setFixedHeight(22);
    m_msg2Label->setFixedHeight(22);
    m_msg1Label->setObjectName("MessageLabel");
    m_msg2Label->setObjectName("MessageLabel");

    QGridLayout* msgGridLayout = new QGridLayout;
    msgGridLayout->addWidget(m_msg1Label, 0, 0, Qt::AlignVCenter);
    msgGridLayout->addWidget(m_speedLabel, 0, 1,Qt::AlignRight|Qt::AlignVCenter);
    msgGridLayout->addWidget(m_msg2Label, 1, 0, Qt::AlignVCenter);
    msgGridLayout->addWidget(m_remainLabel, 1, 1,Qt::AlignRight|Qt::AlignVCenter);
    msgGridLayout->setColumnMinimumWidth(0, 300);

    initButtonFrame();
    m_buttonFrame->hide();
    m_buttonFrame->setAttribute(Qt::WA_AlwaysStackOnTop);

    QFrame* lineLabel = new QFrame;
    lineLabel->setFixedHeight(1);
    lineLabel->setObjectName("LineLabel");

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addStretch(1);
    rightLayout->addLayout(msgGridLayout);
    if ((m_buttonFrame)){
        rightLayout->addWidget(m_buttonFrame);
    }
    rightLayout->addStretch(1);
    rightLayout->addWidget(lineLabel, 0, Qt::AlignBottom);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_animatePad);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(rightLayout);
    mainLayout->addSpacing(5);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(24);
    setLayout(mainLayout);
    setFixedHeight(80);

    setStyleSheet("QPushButton#StopButton{"
                    "image: url(:/icons/images/icons/stop_normal.png);"
                    "border: none;"
                  "}"
                  "QPushButton#StopButton:hover{"
                    "image: url(:/icons/images/icons/stop_hover.png);"
                  "}"
                  "QPushButton#StopButton:pressed{"
                    "image: url(:/icons/images/icons/stop_press.png);"
                  "}"
                  "QPushButton#OptionButton{"
                    "border: 1px solid #eeeeee;"
                    "border-radius: 4px;"
                    "background: white;"
                  "}"
                  "QPushButton#OptionButton:hover{"
                    "border: 1px solid #66ccf9;"
                    "border-radius: 4px;"
                    "background: white;"
                  "}"
                  "QPushButton#OptionButton:pressed{"
                    "border: 1px solid #66ccf9;"
                    "border-radius: 4px;"
                    "background: #66ccf9;"
                    "color: white;"
                  "}"
                  );
}

void MoveCopyTaskWidget::initButtonFrame(){
    m_buttonFrame = new QFrame;

    m_buttonGroup = new QButtonGroup;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    m_keepBothButton = new QPushButton(tr("Keep both"));
    m_skipButton = new QPushButton(tr("Skip"));
    m_replaceButton = new QPushButton(tr("Repalce"));
    m_keepBothButton->setFixedSize(80, 25);
    m_skipButton->setFixedSize(80, 25);
    m_replaceButton->setFixedSize(80, 25);

    m_keepBothButton->setProperty("code", 0);
    m_replaceButton->setProperty("code", 1);
    m_skipButton->setProperty("code", 2);

    m_keepBothButton->setObjectName("OptionButton");
    m_replaceButton->setObjectName("OptionButton");
    m_skipButton->setObjectName("OptionButton");

    buttonLayout->addWidget(m_skipButton);
    buttonLayout->addWidget(m_replaceButton);
    buttonLayout->addWidget(m_keepBothButton);
    buttonLayout->addStretch(1);

    buttonLayout->setContentsMargins(0, 0, 0, 0);

    m_checkBox = new QCheckBox(tr("Do not ask again"));
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_checkBox);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(buttonLayout);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonFrame->setLayout(mainLayout);
}

void MoveCopyTaskWidget::initConnect(){
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(handleClose()));
    connect(m_keepBothButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
    connect(m_skipButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
    connect(m_replaceButton, &QPushButton::clicked, this, &MoveCopyTaskWidget::handleResponse);
}

void MoveCopyTaskWidget::updateMessage(const QMap<QString, QString> &data){
    QString file, destination, speed, remainTime, progress, status, srcPath, targetPath;
    QString msg1, msg2;
    if (data.contains("file")){
        file = data.value("file");
    }
    if (data.contains("destination")){
        destination = data.value("destination");
    }
    if (data.contains("speed")){
        speed = data.value("speed");
    }
    if (data.contains("remainTime")){
        remainTime = data.value("remainTime");
    }

    if (data.contains("progress")){
        progress = data.value("progress");
    }

    if(data.contains("sourcePath"))
        srcPath = data.value("sourcePath");

    if(data.contains("targetPath"))
        targetPath = data.value("targetPath");

    if(data.contains("status"))
        status = data.value("status");

    QString speedStr = "%1";
    QString remainStr = "%1";

    if (m_jobDetail.contains("type")){
        if (m_jobDetail.value("type") == "copy"){
            msg1 = tr("Copying %1").arg(file);
            msg2 = tr("Copy to %2").arg(destination);

        }else if (m_jobDetail.value("type") == "move"){
            msg1 = tr("Moving %1").arg(file);
            msg2 = tr("Move to %2").arg(destination);
        }else if (m_jobDetail.value("type") == "restore"){
            msg1 = tr("Restoring %1").arg(file);
            msg2 = tr("Restore to %2").arg(destination);
        }else if (m_jobDetail.value("type") == "delete"){
            msg1 = tr("Deleting %1").arg(file);
            msg2 = tr("");
        }
        if(status == "calculating"){
            msg2 = tr("Calculating space, please wait");
            m_animatePad->startAnimation();
        } else if(status == "conflict"){
            m_animatePad->stopAnimation();
            msg1 = QString(tr("Target folder already exists file named %1")).arg(file);
            msg2 = QString(tr("Original path %1 target path %2")).arg(srcPath, targetPath);
        } else{
            m_animatePad->stopAnimation();
        }


        QFontMetrics fm(m_msg1Label->font());
        msg1 = fm.elidedText(msg1, Qt::ElideRight, m_msg1Label->width());
        msg2 = fm.elidedText(msg2, Qt::ElideRight, m_msg2Label->width());

        speedStr = speedStr.arg(speed);
        remainStr = remainStr.arg(remainTime);
        setMessage(msg1, msg2);
        setTipMessage(speedStr, remainStr);
    }
    setProgress(progress);
}

void MoveCopyTaskWidget::updateTipMessage(){
//    QString tipMessage = tr("Current speed:%1 time left:%2 ")
//               .arg(QString::number(m_speed), QString::number(m_timeLeft));
//    setTipMessage(tipMessage);
    setTipMessage(QString::number(m_speed), QString::number(m_timeLeft));
}

void MoveCopyTaskWidget::showConflict(){
    setFixedHeight(130);
    m_buttonFrame->show();
    emit heightChanged();
    emit conflictShowed(m_jobDetail);
}

void MoveCopyTaskWidget::hideConflict(){
    setFixedHeight(100);
    m_buttonFrame->hide();
    emit heightChanged();
    emit conflictHided(m_jobDetail);
}

bool MoveCopyTaskWidget::event(QEvent *e)
{
    if(e->type() == QEvent::Enter){
        m_closeButton->show();
        m_speedLabel->hide();
        m_remainLabel->hide();
        m_bgLabel->setStyleSheet("QLabel#Background{"
                                    "background-color: #f3f3f3;"
                                    "border: 1px solid #f3f3f3;"
                                    "border-radius: 4px;"
                                 "}");
        m_bgLabel->setFixedSize(size()- QSize(20,0));
        m_bgLabel->move(10,0);
    } else if (e->type() == QEvent::Leave ){
        m_speedLabel->show();
        m_remainLabel->show();
        m_closeButton->hide();
        m_bgLabel->setStyleSheet("QLabel#Background{"
                                    "background-color: #fff;"
                                    "border: 1px solid #fff;"
                                    "border-radius: 4px;"
                                 "}");
    }

    return QFrame::event(e);
}

void MoveCopyTaskWidget::handleClose()
{
    emit closed(m_jobDetail);
}

void MoveCopyTaskWidget::handleResponse(){
    QObject* who = sender();
    const int& code = who->property("code").toInt();
    m_response.insert("code", code);
    m_response.insert("applyToAll", m_checkBox->isChecked());
    hideConflict();
    emit conflictResponseConfirmed(m_jobDetail, m_response);
}

QString MoveCopyTaskWidget::getTargetObj(){
    return m_targetObj;
}

void MoveCopyTaskWidget::setTargetObj(QString targetObj){
    m_targetObj = targetObj;
}

QString MoveCopyTaskWidget::getDestinationObj(){
    return m_destinationObj;
}

void MoveCopyTaskWidget::setDestinationObj(QString destinationObj){
    m_destinationObj = destinationObj;
}

int MoveCopyTaskWidget::getProgress(){
    return m_progress;
}

void MoveCopyTaskWidget::setProgress(int value){
    m_progress = value;
    m_animatePad->setCurrentValue(value);
}

void MoveCopyTaskWidget::setProgress(QString value){
    m_progress = value.toInt();
    m_animatePad->setCurrentValue(value.toInt());
}

float MoveCopyTaskWidget::getSpeed(){
    return m_speed;
}

void MoveCopyTaskWidget::setSpeed(float speed){
    m_speed = speed;
}

int MoveCopyTaskWidget::getTimeLeft(){
    return m_timeLeft;
}

void MoveCopyTaskWidget::setTimeLeft(int time){
    m_timeLeft = time;
}

void MoveCopyTaskWidget::setMessage(const QString& operateStr, const QString& destinateStr){
    m_operateMessage = operateStr;
    m_destinationMessage = destinateStr;
    m_msg1Label->setText(m_operateMessage);
    m_msg2Label->setText(m_destinationMessage);
}

void MoveCopyTaskWidget::setTipMessage(const QString& speedStr, const QString& remainStr){
    m_speedMessage = speedStr;
    m_remainMessage = remainStr;
    m_speedLabel->setText(m_speedMessage);
    m_remainLabel->setText(m_remainMessage);
}


DTaskDialog::DTaskDialog(QWidget *parent) :
    QWidget(parent)
{
    DPlatformWindowHandle handle(this);
    Q_UNUSED(handle)

    setFixedWidth(m_defaultWidth);
    initUI();
    initConnect();

    QRect rect = geometry();
    QPoint center = qApp->desktop()->geometry().center();
    rect.moveCenter(center);
    move(rect.x(), rect.y());
}

void DTaskDialog::initUI(){

    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);

    m_taskListWidget = new QListWidget;
    m_taskListWidget->setSelectionMode(QListWidget::NoSelection);

    m_titleBar = new DTitlebar;
    m_titleBar->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
    m_titleBar->setFixedHeight(20);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 8, 0, 0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_taskListWidget);
    mainLayout->addStretch(1);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);
}

void DTaskDialog::initConnect(){

}

QListWidget *DTaskDialog::getTaskListWidget()
{
    return m_taskListWidget;
}

void DTaskDialog::setTitle(QString title){
    m_titleBar->setTitle(title);
}

void DTaskDialog::setTitle(int taskCount){
    QString title;
    if (taskCount == 1){
        title = tr("1 task in progress");
    }else{
        title = tr("%1 tasks in progress").arg(QString::number(taskCount));
    }
    setTitle(title);
}

void DTaskDialog::addTask(const QMap<QString, QString> &jobDetail){
    if (jobDetail.contains("jobId")){
        MoveCopyTaskWidget* moveWidget = new MoveCopyTaskWidget(jobDetail);
        moveWidget->setFixedHeight(80);
        connect(moveWidget, SIGNAL(closed(QMap<QString,QString>)),
                this, SLOT(handleTaskClose(QMap<QString,QString>)));
        connect(moveWidget, SIGNAL(conflictResponseConfirmed(QMap<QString,QString>,QMap<QString,QVariant>)),
                this, SLOT(handleConflictResponse(QMap<QString,QString>,QMap<QString,QVariant>)));
        connect(moveWidget, SIGNAL(heightChanged()), this, SLOT(adjustSize()));
        connect(moveWidget, SIGNAL(conflictShowed(QMap<QString,QString>)),
                this, SIGNAL(conflictShowed(QMap<QString,QString>)));
        connect(moveWidget, SIGNAL(conflictHided(QMap<QString,QString>)),
                this, SIGNAL(conflictHided(QMap<QString,QString>)));
        QListWidgetItem* item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(item->sizeHint().width(), 60));
        m_taskListWidget->addItem(item);
        m_taskListWidget->setItemWidget(item, moveWidget);
        m_jobIdItems.insert(jobDetail.value("jobId"), item);
        setTitle(m_taskListWidget->count());
        adjustSize();
        show();
        QTimer::singleShot(100, this, &DTaskDialog::raise);
    }
}

void DTaskDialog::addConflictTask(const QMap<QString, QString> &jobDetail){
    if (jobDetail.contains("jobId")){
        MoveCopyTaskWidget* moveWidget = new MoveCopyTaskWidget(jobDetail);
        moveWidget->setFixedHeight(85);
        connect(moveWidget, SIGNAL(closed(QMap<QString,QString>)),
                this, SLOT(handleTaskClose(QMap<QString,QString>)));
        connect(moveWidget, SIGNAL(conflictResponseConfirmed(QMap<QString,QString>,QMap<QString,QVariant>)),
                this, SLOT(handleConflictResponse(QMap<QString,QString>,QMap<QString,QVariant>)));
        connect(moveWidget, SIGNAL(heightChanged()), this, SLOT(adjustSize()));
        connect(moveWidget, SIGNAL(conflictShowed(QMap<QString,QString>)),
                this, SIGNAL(conflictShowed(QMap<QString,QString>)));
        connect(moveWidget, SIGNAL(conflictHided(QMap<QString,QString>)),
                this, SIGNAL(conflictHided(QMap<QString,QString>)));
        QListWidgetItem* item = new QListWidgetItem();
        item->setFlags(Qt::NoItemFlags);
        item->setSizeHint(QSize(item->sizeHint().width(), 85));
        m_taskListWidget->addItem(item);
        m_taskListWidget->setItemWidget(item, moveWidget);
        m_jobIdItems.insert(jobDetail.value("jobId"), item);
        setTitle(m_taskListWidget->count());
        adjustSize();
        show();
        QTimer::singleShot(100, this, &DTaskDialog::raise);
    }
}

void DTaskDialog::adjustSize(){
    int listHeight = 2;
    int maxHeight = 0;
    for(int i=0; i < m_taskListWidget->count(); i++){
        QListWidgetItem* item = m_taskListWidget->item(i);
        int h = m_taskListWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
        if (i == 5){
            maxHeight = listHeight;
        }
    }
    if (m_taskListWidget->count() >= 6){
        m_taskListWidget->setFixedHeight(maxHeight);
        resize(width(), maxHeight + 60);
    }else{
        m_taskListWidget->setFixedHeight(listHeight);
        resize(width(), listHeight + 60);
    }
//    if (listHeight < qApp->desktop()->availableGeometry().height() - 40){
//        m_taskListWidget->setFixedHeight(listHeight);
//        setFixedHeight(listHeight + 60);
//    }else{
//        setFixedHeight(qApp->desktop()->availableGeometry().height());
//    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);
}

void DTaskDialog::removeTaskByPath(QString jobId){
    if (m_jobIdItems.contains(jobId)){
        QListWidgetItem* item = m_jobIdItems.value(jobId);
        m_taskListWidget->removeItemWidget(item);
        m_taskListWidget->takeItem(m_taskListWidget->row(item));
        m_jobIdItems.remove(jobId);
        setTitle(m_taskListWidget->count());
        if (m_taskListWidget->count() == 0){
            close();
        }
    }
}

void DTaskDialog::showConflictDiloagByJob(const QMap<QString, QString> &jobDetail){
    qDebug() << jobDetail;
    if (jobDetail.contains("jobId")){
        QString jobId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(jobId)){
            QListWidgetItem* item = m_jobIdItems.value(jobId);
            MoveCopyTaskWidget* w = static_cast<MoveCopyTaskWidget*>(m_taskListWidget->itemWidget(item));
            w->showConflict();
        }
    }
}

void DTaskDialog::handleConflictResponse(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response){

    emit conflictRepsonseConfirmed(jobDetail, response);
}

void DTaskDialog::handleMinimizeButtonClick()
{
    showMinimized();
}

void DTaskDialog::handleTaskClose(const QMap<QString, QString> &jobDetail){
    qDebug() << jobDetail;
    removeTask(jobDetail);
    setTitle(m_taskListWidget->count());
    if (jobDetail.contains("type")){
        emit abortTask(jobDetail);
    }
}

void DTaskDialog::removeTask(const QMap<QString, QString> &jobDetail){
    if (jobDetail.contains("jobId")){
        removeTaskByPath(jobDetail.value("jobId"));
        adjustSize();
    }
}

void DTaskDialog::removeTaskImmediately(const QMap<QString, QString> &jobDetail)
{
    if(m_taskListWidget->count() > 1)
        delayRemoveTask(jobDetail);
    else
        removeTask(jobDetail);
}

void DTaskDialog::delayRemoveTask(const QMap<QString, QString> &jobDetail)
{
    QTimer::singleShot(2000, this, [=](){
        removeTask(jobDetail);
    });
}

void DTaskDialog::handleUpdateTaskWidget(const QMap<QString, QString> &jobDetail,
                                         const QMap<QString, QString> &data){
    if (jobDetail.contains("jobId")){
        QString jobId = jobDetail.value("jobId");
        if (m_jobIdItems.contains(jobId)){
            QListWidgetItem* item = m_jobIdItems.value(jobId);
            MoveCopyTaskWidget* w = static_cast<MoveCopyTaskWidget*>(m_taskListWidget->itemWidget(item));
            w->updateMessage(data);
        }
    }
}


void DTaskDialog::closeEvent(QCloseEvent *event){
    foreach (QListWidgetItem* item, m_jobIdItems.values()) {
        if (item){
            if (m_taskListWidget->itemWidget(item)){
                MoveCopyTaskWidget* w = static_cast<MoveCopyTaskWidget*>(m_taskListWidget->itemWidget(item));
                w->handleClose();
            }
        }
    }
    QWidget::closeEvent(event);
    emit closed();
}
