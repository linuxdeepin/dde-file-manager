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
    m_closeButton = new QPushButton;
    m_closeButton->setObjectName("CloseButton");
    m_closeButton->setFixedSize(27, 23);
    m_closeButton->setAttribute(Qt::WA_NoMousePropagation);

    m_animatePad = new CircleProgressAnimatePad;
    m_animatePad->setFixedSize(54, 54);
    m_animatePad->setBackgroundColor(QColor("#E9E9E9"));
    m_animatePad->setChunkColor(QColor("#3cadff"));
    m_animatePad->setLineWidth(3);
    m_animatePad->setFontSize(14);

    m_messageLabel = new QLabel;
    m_messageLabel->setFixedHeight(32);
    m_messageLabel->setObjectName("MessageLabel");
    m_tipMessageLabel = new QLabel;
    m_tipMessageLabel->setFixedHeight(18);
    m_tipMessageLabel->setObjectName("TaskTipMessageLabel");

    QVBoxLayout* messageLayout = new QVBoxLayout;
    messageLayout->addWidget(m_messageLabel);
    messageLayout->addWidget(m_tipMessageLabel);

    QHBoxLayout* messageBoxLayout = new QHBoxLayout;
    messageBoxLayout->addLayout(messageLayout);
    messageBoxLayout->addWidget(m_closeButton);

    initButtonFrame();
    m_buttonFrame->hide();

    QFrame* lineLabel = new QFrame;
    lineLabel->setFixedHeight(1);
    lineLabel->setObjectName("LineLabel");

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addLayout(messageBoxLayout);
    if ((m_buttonFrame)){
        rightLayout->addWidget(m_buttonFrame);
    }
    rightLayout->addWidget(lineLabel);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_animatePad);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(rightLayout);
    mainLayout->setContentsMargins(5, 0, 5, 0);
    setLayout(mainLayout);
    setFixedHeight(80);
}

void MoveCopyTaskWidget::initButtonFrame(){
    m_buttonFrame = new QFrame;

    QFrame* leftButtonFrame = new QFrame;
    leftButtonFrame->setFixedHeight(22);
    leftButtonFrame->setFixedWidth(208);
    leftButtonFrame->setObjectName("ButtonFrame");
    QStringList buttonKeys, buttonTexts;
    buttonKeys << "Coexists" << "Replace" << "Skip";
    buttonTexts << tr("Keep both") << tr("Replace") << tr("Skip");
    m_buttonGroup = new QButtonGroup;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    foreach (QString label, buttonKeys) {
        int index = buttonKeys.indexOf(label);
        QPushButton* button = new QPushButton(buttonTexts.at(index));
        button->setObjectName("ConflictButton");
        button->setAttribute(Qt::WA_NoMousePropagation);
        button->setCheckable(true);
        button->setFixedHeight(20);
        button->setFixedWidth(68);
        m_buttonGroup->addButton(button, index);
        buttonLayout->addWidget(button);
        if (index < buttonKeys.length() - 1){
            QLabel* label = new QLabel;
            label->setObjectName("VLine");
            label->setFixedWidth(1);
            buttonLayout->addWidget(label);
        }
    }
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    leftButtonFrame->setLayout(buttonLayout);
    m_buttonGroup->button(1)->setChecked(true);

    m_checkBox = new QCheckBox(tr("Do not ask again"));
    m_enterButton = new QPushButton(tr("Ok"));
    m_enterButton->setObjectName("NormalButton");
    m_enterButton->setFixedSize(60, 20);
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addSpacing(2);
    mainLayout->addWidget(leftButtonFrame);
    mainLayout->addWidget(m_checkBox);
    mainLayout->addWidget(m_enterButton);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonFrame->setLayout(mainLayout);
}

void MoveCopyTaskWidget::initConnect(){
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(handleClose()));
    connect(m_enterButton, SIGNAL(clicked()), this, SLOT(handleResponse()));
}

void MoveCopyTaskWidget::updateMessage(const QMap<QString, QString> &data){
    QString file, destination, speed, remainTime, progress, status;
    QString message, tipMessage;
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

    if(data.contains("status"))
        status = data.value("status");



    int fileMaxWidth = 180;
    int destinationMaxWidth = 90;

    QFontMetrics fm = m_messageLabel->fontMetrics();

    if (fm.width(destination) > destinationMaxWidth){
        destination = fm.elidedText(destination, Qt::ElideMiddle, destinationMaxWidth);
    }else{
        fileMaxWidth = 260 - fm.width(destination);
    }

    if (fm.width(file) > fileMaxWidth){
        file = fm.elidedText(file, Qt::ElideMiddle, fileMaxWidth);
    }

    QString speedTime(tr("Current speed:%1 Time left:%2 "));

    if (m_jobDetail.contains("type")){
        if (m_jobDetail.value("type") == "copy"){
            message = tr("<span style=\"color: #3cadff\"> %1 </span> is being copied to <span style=\"color: #3cadff\"> %2 </span>")
                    .arg(file, destination);
            tipMessage = speedTime.arg(speed, remainTime);

        }else if (m_jobDetail.value("type") == "move"){
            message = tr("<span style=\"color: #3cadff\"> %1 </span> is being moved to <span style=\"color: #3cadff\"> %2 </span>")
                    .arg(file, destination);

            tipMessage = speedTime.arg(speed, remainTime);
        }else if (m_jobDetail.value("type") == "restore"){
            message = tr("<span style=\"color: #3cadff\"> %1 </span> is being restored to <span style=\"color: #3cadff\"> %2 </span>")
                    .arg(file, destination);

            tipMessage = speedTime.arg(speed, remainTime);
        }else if (m_jobDetail.value("type") == "delete"){
            message = tr("<span style=\"color: #3cadff\"> %1 </span> is being deleted ").arg(file);
            tipMessage = speedTime.arg(speed, remainTime);
        }
        if(status == "calculating"){
            tipMessage = tr("Calculating space, please wait");
            m_animatePad->startAnimation();
        } else{
            m_animatePad->stopAnimation();
        }

        setMessage(message);
        setTipMessage(tipMessage);
    }
    setProgress(progress);
}

void MoveCopyTaskWidget::updateTipMessage(){
    QString tipMessage = tr("Current speed:%1 time left:%2 ")
               .arg(QString::number(m_speed), QString::number(m_timeLeft));
    setTipMessage(tipMessage);
}

void MoveCopyTaskWidget::showConflict(){
    setFixedHeight(85);
    m_buttonFrame->show();
    emit heightChanged();
    emit conflictShowed(m_jobDetail);
}

void MoveCopyTaskWidget::hideConflict(){
    setFixedHeight(60);
    m_buttonFrame->hide();
    emit heightChanged();
    emit conflictHided(m_jobDetail);
}

void MoveCopyTaskWidget::handleClose()
{
    emit closed(m_jobDetail);
}

void MoveCopyTaskWidget::handleResponse(){
    m_response.insert("code", m_buttonGroup->checkedId());
    m_response.insert("applyToAll", m_checkBox->isChecked());\
    if (m_buttonGroup->checkedId() < 2){
        hideConflict();
    }
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

QString MoveCopyTaskWidget::getMessage(){
    return m_message;
}

void MoveCopyTaskWidget::setMessage(QString message){
    m_message = message;
    m_messageLabel->setText(m_message);
}

QString MoveCopyTaskWidget::getTipMessage(){
    return m_tipMessage;
}

void MoveCopyTaskWidget::setTipMessage(QString tipMessage){
    m_tipMessage = tipMessage;
    m_tipMessageLabel->setText(tipMessage);
}


DTaskDialog::DTaskDialog(QWidget *parent) :
    DMoveableWidget(parent)
{
    resize(m_defaultWidth, 0);
    initUI();
    initConnect();
    moveCenter();
}

void DTaskDialog::initUI(){
    QFrame* contentFrame = new QFrame;
    contentFrame->setObjectName("ContentFrame");

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("titleLable");

    m_titleBarCloseButton = new QPushButton(this);
    m_titleBarCloseButton->setObjectName("CloseButton");
    m_titleBarCloseButton->setFixedSize(27, 23);
    m_titleBarCloseButton->setAttribute(Qt::WA_NoMousePropagation);

    m_titleBarMinimizeButton = new QPushButton(this);
    m_titleBarMinimizeButton->setObjectName("MinimizeButton");
    m_titleBarMinimizeButton->setFixedSize(27, 23);
    m_titleBarMinimizeButton->setAttribute(Qt::WA_NoMousePropagation);


    QFrame* lineLabel = new QFrame;
    lineLabel->setFixedHeight(1);
    lineLabel->setObjectName("LineLabel");

    m_taskListWidget = new QListWidget;
    m_taskListWidget->setSelectionMode(QListWidget::NoSelection);

    QFrame* titleFrame = new QFrame;
    titleFrame->setFixedHeight(25);
    QHBoxLayout* m_titleLayout = new QHBoxLayout;
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_titleBarMinimizeButton);
    m_titleLayout->addWidget(m_titleBarCloseButton);
    m_titleLayout->setSpacing(0);
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout* titleBoxLayout = new QVBoxLayout;
    titleBoxLayout->addLayout(m_titleLayout);
    titleBoxLayout->addWidget(lineLabel);

    QVBoxLayout* m_layout = new QVBoxLayout;
    m_layout->addLayout(titleBoxLayout);
    m_layout->addWidget(m_taskListWidget);
//    m_layout->setSpacing(0);
    contentFrame->setLayout(m_layout);

    QHBoxLayout* contentlayout = new QHBoxLayout;
    contentlayout->addWidget(contentFrame);

    contentlayout->setContentsMargins(5, 5, 5, 5);
    setLayout(contentlayout);

//    setStyleSheet(getQssFromFile(":/qss/dialogs/qss/dark.qss"));
}

void DTaskDialog::initConnect(){
    connect(m_titleBarCloseButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(m_titleBarMinimizeButton, SIGNAL(clicked()), this, SLOT(handleMinimizeButtonClick()));
}

QListWidget *DTaskDialog::getTaskListWidget()
{
    return m_taskListWidget;
}

void DTaskDialog::setTitle(QString title){
    m_titleLabel->setText(title);
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
        moveWidget->setFixedHeight(60);
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
    DMoveableWidget::closeEvent(event);
    emit closed();
}
