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
#include "dfmglobal.h"
#include "app/define.h"
#include "interfaces/dfileservices.h"
#include "dabstractfileinfo.h"
#include "shutil/fileutils.h"

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
//    this->setStyleSheet("border:1px solid green");
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
    m_msg1Label->setObjectName("MessageLabel1");
    m_msg2Label->setObjectName("MessageLabel2");

    QGridLayout* msgGridLayout = new QGridLayout;
    msgGridLayout->addWidget(m_msg1Label, 0, 0, Qt::AlignVCenter);
    msgGridLayout->addWidget(m_speedLabel, 0, 1,Qt::AlignRight|Qt::AlignVCenter);
    msgGridLayout->addWidget(m_msg2Label, 1, 0, Qt::AlignVCenter);
    msgGridLayout->addWidget(m_remainLabel, 1, 1,Qt::AlignRight|Qt::AlignVCenter);
    msgGridLayout->setColumnMinimumWidth(0, 300);
    msgGridLayout->setHorizontalSpacing(5);

    initConflictDetailFrame();
    initButtonFrame();

    m_buttonFrame->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_lineLabel = new QFrame;
    m_lineLabel->setFixedHeight(1);
    m_lineLabel->setObjectName("LineLabel");
    m_lineLabel->hide();

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addStretch(1);
    rightLayout->addLayout(msgGridLayout);
    rightLayout->addWidget(m_conflictFrame);
    rightLayout->addSpacing(20);

    rightLayout->addWidget(m_buttonFrame);

    rightLayout->addStretch(1);
    rightLayout->addWidget(m_lineLabel, 0, Qt::AlignBottom);
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

    m_conflictFrame->hide();
    m_buttonFrame->hide();
    m_conflictFrame->hide();
}


void MoveCopyTaskWidget::initConflictDetailFrame()
{

    m_conflictFrame = new QFrame(this);

    m_originIconLabel = new QLabel(this);
//    m_originIconLabel->setStyleSheet("background-color: green");
    m_originIconLabel->setFixedSize(48, 48);
    m_originIconLabel->setScaledContents(true);

    m_originTitleLabel = new QLabel(this);
//    m_originTitleLabel->setStyleSheet("background-color: yellow");
    m_originTitleLabel->setFixedHeight(20);

    m_originTimeLabel = new QLabel(this);
//    m_originTimeLabel->setStyleSheet("background-color: gray");
    m_originTimeLabel->setFixedHeight(20);

    m_originSizeLabel = new QLabel(this);
//    m_originSizeLabel->setStyleSheet("background-color: green");
    m_originSizeLabel->setFixedSize(90, 20);

    m_targetIconLabel = new QLabel(this);
//    m_targetIconLabel->setStyleSheet("background-color: red");
    m_targetIconLabel->setFixedSize(48, 48);
    m_targetIconLabel->setScaledContents(true);

    m_targetTitleLabel = new QLabel(this);
//    m_targetTitleLabel->setStyleSheet("background-color: yellow");
    m_targetTitleLabel->setFixedHeight(20);

    m_targetTimeLabel = new QLabel(this);
//    m_targetTimeLabel->setStyleSheet("background-color: gray");
    m_targetTimeLabel->setFixedHeight(20);

    m_targetSizeLabel = new QLabel(this);
//    m_targetSizeLabel->setStyleSheet("background-color: green");
    m_targetSizeLabel->setFixedSize(90, 20);

    QGridLayout* conflictMainLayout = new QGridLayout(this);

    conflictMainLayout->addWidget(m_originIconLabel, 0, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originTitleLabel, 0, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originTimeLabel, 1, 1, 1, 1,Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_originSizeLabel, 1, 2, 1, 1, Qt::AlignVCenter);


    conflictMainLayout->addWidget(m_targetIconLabel, 2, 0, 2, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetTitleLabel, 2, 1, 1, 2, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetTimeLabel, 3, 1, Qt::AlignVCenter);
    conflictMainLayout->addWidget(m_targetSizeLabel, 3, 2, Qt::AlignVCenter);

    conflictMainLayout->setHorizontalSpacing(4);
    conflictMainLayout->setVerticalSpacing(4);
    conflictMainLayout->setContentsMargins(0, 0, 0, 0);

    m_conflictFrame->setLayout(conflictMainLayout);
    m_conflictFrame->setFixedHeight(120);

}



void MoveCopyTaskWidget::initButtonFrame(){
    m_buttonFrame = new QFrame;

    m_buttonGroup = new QButtonGroup;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    m_keepBothButton = new QPushButton(tr("Keep both"));
    m_skipButton = new QPushButton(tr("Skip"));
    m_replaceButton = new QPushButton(tr("Replace"));

    m_keepBothButton->setFixedSize(70, 25);
    m_skipButton->setFixedSize(70, 25);
    m_replaceButton->setFixedSize(70, 25);

    m_keepBothButton->setProperty("code", 0);
    m_replaceButton->setProperty("code", 1);
    m_skipButton->setProperty("code", 2);

    m_keepBothButton->setObjectName("OptionButton");
    m_replaceButton->setObjectName("OptionButton");
    m_skipButton->setObjectName("OptionButton");
    m_keepBothButton->setCheckable(true);
    m_keepBothButton->setChecked(true);

    buttonLayout->addWidget(m_skipButton);
    buttonLayout->addWidget(m_replaceButton);
    buttonLayout->addWidget(m_keepBothButton);
    buttonLayout->addStretch(1);

    buttonLayout->setContentsMargins(0, 0, 0, 0);

    m_checkBox = new QCheckBox(tr("Do not ask again"));
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(0);
    mainLayout->addWidget(m_checkBox);
    mainLayout->addSpacing(0);
    mainLayout->addLayout(buttonLayout);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonFrame->setLayout(mainLayout);
    m_buttonFrame->setFixedHeight(30);
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

    qDebug() << data;
    if (QFileInfo(srcPath).isDir() &&
            QFileInfo(targetPath).isDir()){
        m_replaceButton->setText(tr("merge"));
    }else{
        m_replaceButton->setText(tr("Replace"));
    }

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
        }else if (m_jobDetail.value("type") == "trash"){
            msg1 = tr("Trashing %1").arg(file);
            msg2 = tr("");
        }

        if (status == "restoring"){
            m_animatePad->startAnimation();
        }else if(status == "calculating"){
            msg2 = tr("Calculating space, please wait");
            m_animatePad->startAnimation();
        } else if(status == "conflict"){
            m_animatePad->stopAnimation();
            msg1 = QString(tr("File named %1 already exists in target folder")).arg(file);
            msg2 = QString(tr("Original path %1 target path %2")).arg(srcPath, targetPath);
            updateConflictDetailFrame(srcPath, targetPath);
        } else{
            m_animatePad->stopAnimation();
        }

        QFontMetrics fm(m_msg1Label->font());
        msg1 = fm.elidedText(msg1, Qt::ElideRight, m_msg1Label->width());
//        msg2 = fm.elidedText(msg2, Qt::ElideRight, m_msg2Label->width());

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

void MoveCopyTaskWidget::handleLineDisplay(const int &row, const bool& hover, const int& taskNum)
{
    if((row -1) == property("row").toInt() || row == property("row").toInt()){
        if(hover)
            m_lineLabel->hide();
        else
            m_lineLabel->show();
    } else{
        if(m_lineLabel->isHidden())
            m_lineLabel->show();
    }

    if(property("row").toInt() == taskNum-1)
        m_lineLabel->hide();
}

void MoveCopyTaskWidget::showConflict(){
    qDebug() << m_buttonFrame->height() << m_conflictFrame->height();
    setFixedHeight(100 + m_buttonFrame->height() + m_conflictFrame->height());
    m_conflictFrame->show();
    m_buttonFrame->show();
    emit heightChanged();
    emit conflictShowed(m_jobDetail);
}

void MoveCopyTaskWidget::hideConflict(){
    setFixedHeight(100);
    m_conflictFrame->hide();
    m_buttonFrame->hide();
    emit heightChanged();
    emit conflictHided(m_jobDetail);
}

void MoveCopyTaskWidget::updateConflictDetailFrame(const QString &originFilePath, const QString &targetFilePath)
{
    qDebug() << originFilePath << targetFilePath << m_originIconLabel << m_targetIconLabel;
    DAbstractFileInfoPointer originInfo = fileService->createFileInfo(NULL, DUrl::fromLocalFile(originFilePath));
    DAbstractFileInfoPointer targetInfo = fileService->createFileInfo(NULL, DUrl::fromLocalFile(targetFilePath));
    if (originInfo && targetInfo){
        QFontMetrics fm(m_originTitleLabel->font());

        m_originIconLabel->setPixmap(originInfo->fileIcon().pixmap(48, 48));
        m_originTimeLabel->setText(QString(tr("Time modified:%1")).arg(originInfo->lastModifiedDisplayName()));
        if (originInfo->isDir()){
            m_originTitleLabel->setText(tr("Original folder:%1").arg(originFilePath));
            m_originSizeLabel->setText(QString(tr("Contains:%1")).arg(originInfo->sizeDisplayName()));
        }else{
            m_originTitleLabel->setText(tr("Original file:%1").arg(originFilePath));
            m_originSizeLabel->setText(QString(tr("Size:%1")).arg(originInfo->sizeDisplayName()));
        }
        QString originMsg = fm.elidedText(m_originTitleLabel->text(), Qt::ElideRight, 300);
        m_originTitleLabel->setText(originMsg);

        m_targetIconLabel->setPixmap(targetInfo->fileIcon().pixmap(48, 48));
        m_targetTimeLabel->setText(QString(tr("Time modified:%1")).arg(targetInfo->lastModifiedDisplayName()));

        if (originInfo->isDir()){
            m_targetTitleLabel->setText(tr("Target folder:%1").arg(targetFilePath));
            m_targetSizeLabel->setText(QString(tr("Contains:%1")).arg(targetInfo->sizeDisplayName()));
        }else{
            m_targetTitleLabel->setText(tr("Target file:%1").arg(targetFilePath));
            m_targetSizeLabel->setText(QString(tr("Size:%1")).arg(targetInfo->sizeDisplayName()));
        }
        QString targetMsg = fm.elidedText(m_targetTitleLabel->text(), Qt::ElideRight, 300);
        m_targetTitleLabel->setText(targetMsg);
    }

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
                                    "border-radius: 8px;"
                                 "}");
        m_bgLabel->setFixedSize(size()- QSize(20,0));
        m_bgLabel->move(10,0);
        emit hovereChanged(true);
    } else if (e->type() == QEvent::Leave ){
        hovereChanged(false);
        m_speedLabel->show();
        m_remainLabel->show();
        m_closeButton->hide();
        m_bgLabel->setStyleSheet("QLabel#Background{"
                                    "background-color: #fff;"
                                    "border: 1px solid #fff;"
                                    "border-radius: 8px;"
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
    m_msg2Label->setText("");
}

void MoveCopyTaskWidget::setTipMessage(const QString& speedStr, const QString& remainStr){
    m_speedMessage = speedStr;
    m_remainMessage = remainStr;
    m_speedLabel->setText(m_speedMessage);
    m_remainLabel->setText(m_remainMessage);
}


int DTaskDialog::MaxHeight = 0;

DTaskDialog::DTaskDialog(QWidget *parent) :
    QDialog(parent)
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
    setWindowFlags(Qt::FramelessWindowHint|Qt::Window);

    m_taskListWidget = new QListWidget;
    m_taskListWidget->setSelectionMode(QListWidget::NoSelection);

    m_titleBar = new DTitlebar;
    m_titleBar->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
    m_titleBar->setFixedHeight(27);
    m_titleBar->layout()->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_titleBar);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(m_taskListWidget);
    mainLayout->addStretch(1);
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
        title = QObject::tr("1 task in progress");
    }else{
        title = QObject::tr("%1 tasks in progress").arg(QString::number(taskCount));
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

        //handle item line display logic
        connect(moveWidget, &MoveCopyTaskWidget::hovereChanged, this, &DTaskDialog::onItemHovered);
        connect(this, &DTaskDialog::currentHoverRowChanged, moveWidget, &MoveCopyTaskWidget::handleLineDisplay);
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

        moveWidget->setProperty("row",m_taskListWidget->count() -1);
        emit currentHoverRowChanged(1, false, m_taskListWidget->count());
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
    for(int i=0; i < m_taskListWidget->count(); i++){
        QListWidgetItem* item = m_taskListWidget->item(i);
        int h = m_taskListWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight < qApp->desktop()->availableGeometry().height() - 60){
        m_taskListWidget->setFixedHeight(listHeight);
        setFixedHeight(listHeight + 60);
        MaxHeight = height();
    }else{
        setFixedHeight(MaxHeight);
    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);
    moveYCenter();
}

void DTaskDialog::moveYCenter()
{
    QRect qr = frameGeometry();
    QPoint cp;
    if (parent()){
        cp = static_cast<QWidget*>(parent())->geometry().center();
    }else{
        cp = qApp->desktop()->availableGeometry().center();
    }
    qr.moveCenter(cp);
    move(x(), qr.y());
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

        for(int i = 0; i < m_taskListWidget->count(); i++){
            QListWidgetItem* item = m_taskListWidget->item(i);
            MoveCopyTaskWidget* w =  qobject_cast<MoveCopyTaskWidget*>(m_taskListWidget->itemWidget(item));
            if(w)
                w->setProperty("row", i);
        }

        emit currentHoverRowChanged(0, false, m_taskListWidget->count());
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

void DTaskDialog::onItemHovered(const bool& hover)
{
    MoveCopyTaskWidget* w = qobject_cast<MoveCopyTaskWidget*>(sender());
    int row = w->property("row").toInt();
    if(row >= 0)
        emit currentHoverRowChanged(row, hover, m_taskListWidget->count());
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
    QDialog::closeEvent(event);
    emit closed();
}

void DTaskDialog::keyPressEvent(QKeyEvent *event)
{
    //handle escape key press event for emitting close event
    if(event->key() == Qt::Key_Escape)
        emit close();
    QDialog::keyPressEvent(event);
}
