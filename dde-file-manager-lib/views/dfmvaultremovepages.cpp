#include "dfmvaultremovepages.h"
#include "vault/interfaceactivevault.h"
#include "controllers/vaultcontroller.h"

#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QToolButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QToolTip>
#include <QRegExpValidator>
#include <QStackedLayout>
#include <QDir>
#include <QCloseEvent>
#include <QDBusConnection>

#include <DPasswordEdit>
#include <DTextEdit>
#include <DProgressBar>

DWIDGET_USE_NAMESPACE

VaultRemoveFileDialog::VaultRemoveFileDialog(QWidget *parent)
    : DDialog (parent)
{
    this->setTitle(tr("Remove File Vault"));
    this->setMessage(tr("Removing..."));
    this->setIcon(QIcon::fromTheme("dfm_safebox"));
    this->setFixedSize(438, 280);

    initUI();

    initConnect();
}

VaultRemoveFileDialog::~VaultRemoveFileDialog()
{

}

void VaultRemoveFileDialog::initUI()
{
    addButton(tr("Ok"), true, ButtonType::ButtonRecommend);
    getButton(0)->setEnabled(false);
    getButton(0)->setFixedWidth(this->width()/2);

    m_removeProgress = new QProgressBar(this);
    m_removeProgress->setAlignment(Qt::AlignHCenter);
    m_removeProgress->setMinimum(0);
    m_removeProgress->setMaximum(100);

    addContent(m_removeProgress);
    setContentsMargins(10, 0, 10, 0);
}

void VaultRemoveFileDialog::initConnect()
{
    connect(this, &VaultRemoveFileDialog::buttonClicked, this, [=]{
        this->close();
    });
}

bool VaultRemoveFileDialog::statisticsFiles(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return false;

    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);
    int i = 0;
    do{
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.fileName()=="." | fileInfo.fileName()==".."){
            i++;
            continue;
        }

        bool bisDir = fileInfo.isDir();
        if(bisDir){
            m_iFiles++;
            //递归
            statisticsFiles(fileInfo.filePath());
        }else{
            m_iFiles++;
        }

        i++;
    }while(i < list.size());

    return true;
}

void VaultRemoveFileDialog::removeFileInDir(const QString &path)
{
    QDir dir(path);
    QFileInfoList infoList = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);

    if(dir.exists()){
        dir.setFilter(QDir::Files | QDir::NoSymLinks);
        QFileInfoList list = dir.entryInfoList();
    }

    //遍历文件信息列表，进行文件删除
    foreach(QFileInfo fileInfo, infoList){
        if (fileInfo.isDir()){
            //递归
            removeFileInDir(fileInfo.absoluteFilePath());
        }else if (fileInfo.isFile()){
            QFile file(fileInfo.absoluteFilePath());

            //删除文件
            file.remove();
            m_iRmFiles ++;
            m_removeProgress->setValue(100 * (m_iRmFiles / m_iFiles));
        }
    }

    QDir temp_dir;
    //删除文件夹
    temp_dir.rmdir(path);
    m_iRmDir ++;
    m_removeProgress->setValue(100 * (m_iRmFiles + m_iRmDir - 1) / m_iFiles);
}

void VaultRemoveFileDialog::removeVault(const QString &rmPath)
{
    if (statisticsFiles(rmPath)){
        removeFileInDir(rmPath);
        this->setMessage(tr("Removed successfully"));
    }else {
        this->setMessage(tr("Failed to remove"));
    }

    getButton(0)->setEnabled(true);
}

void VaultRemoveFileDialog::clear()
{
    //将相应的值赋0
    m_iFiles = 0;
    m_iRmDir = 0;
    m_iRmFiles = 0;

    m_removeProgress->setValue(0);
    getButton(0)->setEnabled(false);
    this->setMessage(tr("Removing..."));
}
/*****************************************************************/

VaultPasswordPage::VaultPasswordPage(QWidget *parent)
    : QWidget (parent)
{
    initUI();

    connect(m_pwdEdit->lineEdit(), &QLineEdit::textChanged, this, &VaultPasswordPage::onPasswordChanged);
}

VaultPasswordPage::~VaultPasswordPage()
{

}

void VaultPasswordPage::initUI()
{
    m_pwdEdit = new DPasswordEdit(this);
    m_pwdEdit->lineEdit()->setPlaceholderText(tr("Verify your fingerprint or password"));

    m_helpBtn = new QPushButton(this);
    m_helpBtn->setFixedSize(40, 36);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_pwdEdit);
    layout->addWidget(m_helpBtn);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
}

QString VaultPasswordPage::getPassword()
{
    return m_pwdEdit->text();
}

void VaultPasswordPage::clear()
{
    // 重置状态
    m_pwdEdit->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    m_pwdEdit->lineEdit()->setPalette(palette);
    m_pwdEdit->setEchoMode(QLineEdit::Password);
}

QLineEdit *VaultPasswordPage::lineEdit()
{
    return m_pwdEdit->lineEdit();
}

void VaultPasswordPage::onPasswordChanged(const QString &password)
{
    if (!password.isEmpty()){
        QLineEdit edit;
        QPalette palette = edit.palette();
        m_pwdEdit->lineEdit()->setPalette(palette);
    }
}
/*****************************************************************/

VaultKeyPage::VaultKeyPage(QWidget *parent)
    : QWidget (parent)
{
    initUI();

    connect(m_keyEdit, &QPlainTextEdit::textChanged, this, &VaultKeyPage::onRecoveryKeyChanged);
}

VaultKeyPage::~VaultKeyPage()
{

}

void VaultKeyPage::initUI()
{
    //正则表达式，只能输入字母和数字
    //QRegExpValidator *pRevalidotor = new QRegExpValidator(QRegExp("[a-zA-Z0-9]{100}"), this);
    m_keyEdit = new QPlainTextEdit(this);
    m_keyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    m_keyEdit->installEventFilter(this);
    //m_keyEdit->setValidator(pRevalidotor);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_keyEdit);
    //layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);
}

int VaultKeyPage::afterRecoveryKeyChanged(QString &str)
{
    if (str.isEmpty()){
        return -1;
    }

    int location = m_keyEdit->textCursor().position(); // 计算当前光标位置
    int srcLength = str.length();   // 用于计算原有字符串中的“-”数量
    //清除所有的“-”
    str.replace(tr("-"), tr(""));
    int minusNumber = srcLength - str.length(); // 原有字符串中的“-”数量

    int index = 4;
    int minusNum = 0;

    int length = str.length();
    while (index < length) {
        if (index % 4 == 0){
            str.insert(index + minusNum, tr("-"));
            minusNum++;
        }
        index++;
    }

    //计算添加“-”后，重新计算下光标的位置，
    if (minusNum > minusNumber) {
        location += minusNum - minusNumber;
    }

    if (location > str.length()) {
        location = str.length();
    } else if (location < 0) {
        location = 0;
    }

    return location;
}

QString VaultKeyPage::getKey()
{
    QString strKey = m_keyEdit->toPlainText();

    return strKey.replace(tr("-"), tr(""));
}

void VaultKeyPage::clear()
{
    m_keyEdit->clear();
}

void VaultKeyPage::onRecoveryKeyChanged()
{
    QString key = m_keyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    // 限制输入的最大长度
    if (length > maxLength){
        int position = m_keyEdit->textCursor().position();
        QTextCursor textCursor = m_keyEdit->textCursor();
        key.remove(position-(length - maxLength), length - maxLength);
        m_keyEdit->setPlainText(key);
        textCursor.setPosition(position - (length-maxLength));
        m_keyEdit->setTextCursor(textCursor);

        return;
    }

    static bool isEdited = false;
    if (!isEdited){
        isEdited = true;
        int position = afterRecoveryKeyChanged(key);
        m_keyEdit->setPlainText(key);

        QTextCursor textCursor = m_keyEdit->textCursor();
        textCursor.setPosition(position);
        m_keyEdit->setTextCursor(textCursor);
    } else {
        isEdited = false;
    }
}

bool VaultKeyPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress){
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(watched);
        if (edit == m_keyEdit) {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);

            // 过滤换行操作以及“-”
            if (keyEvent->key() == Qt::Key_Enter
                    || keyEvent->key() == Qt::Key_Return
                    || keyEvent->key() == Qt::Key_Minus){
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
/*****************************************************************/

DFMVaultRemovePages::DFMVaultRemovePages(QWidget *parent)
    : DDialog (parent)
    , m_stackedLayout (new QStackedLayout())
{
    this->setTitle(tr("Remove File Vault"));
    this->setMessage(tr("Once the file vault is removed, the files in it will be permanently deleted. This action cannot be undone, please confirm and continue."));
    this->setIcon(QIcon::fromTheme("dfm_safebox"));
    this->setFixedSize(440, 290);

    initUI();

    initConnect();
}

void DFMVaultRemovePages::initUI()
{
    VaultPasswordPage *pwdPage = new VaultPasswordPage(this);
    VaultKeyPage *keyPage = new VaultKeyPage(this);
    m_rmFileDialog = new VaultRemoveFileDialog(this);

    insertPage(PageType::Password, pwdPage);
    insertPage(PageType::Key, keyPage);

    //界面UI布局
    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Use Key") << tr("Remove");
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], true);
    addButton(buttonTexts[2], true);
    getButton(2)->setStyleSheet(tr("color: rgb(255, 85, 0);"));
    setDefaultButton(2);

    QFrame *mianFrame = new QFrame(this);
    mianFrame->setLayout(m_stackedLayout);
    addContent(mianFrame);
    setSpacing(0);
    // 防止点击按钮隐藏界面
    setOnButtonClickedClose(false);
    m_stackedLayout->setCurrentWidget(m_pages[PageType::Password]);
    m_currentPage = PageType::Password;
}

void DFMVaultRemovePages::initConnect()
{
    connect(this, &DFMVaultRemovePages::buttonClicked, this, &DFMVaultRemovePages::onButtonClicked);
    connect(VaultController::getVaultController(), &VaultController::signalLockVault, this, &DFMVaultRemovePages::onLockVault);
}

void DFMVaultRemovePages::insertPage(const PageType &pageType, QWidget *widget)
{
    m_pages.insert(pageType, widget);
    m_stackedLayout->addWidget(widget);
}

void DFMVaultRemovePages::showEvent(QShowEvent *event)
{
    // 重置界面状态
    m_stackedLayout->setCurrentWidget(m_pages[PageType::Password]);
    qobject_cast<VaultPasswordPage *>(m_pages[PageType::Password])->clear();
    qobject_cast<VaultKeyPage *>(m_pages[PageType::Key])->clear();
    m_rmFileDialog->clear();
    getButton(1)->setText(tr("Use Key"));
    m_bRemoveVault = false;
    m_currentPage = PageType::Password;
}

DFMVaultRemovePages *DFMVaultRemovePages::instance()
{
    static DFMVaultRemovePages s_instance;
    return &s_instance;
}

void DFMVaultRemovePages::onButtonClicked(int index, const QString &text)
{
    switch (index) {
    case 0: //点击取消按钮
        close();
        break;
    case 1:{ // 切换验证方式
        if (m_currentPage == PageType::Password){
            getButton(index)->setText(tr("Use Password"));
            m_currentPage = PageType::Key;
        }else {
            getButton(index)->setText(tr("Use Key"));
            m_currentPage = PageType::Password;
        }
        m_stackedLayout->setCurrentWidget(m_pages[m_currentPage]);
    }
        break;
    case 2:{// 删除
        if (m_currentPage == PageType::Password){
            // 密码验证
            VaultPasswordPage *passwordPage = qobject_cast<VaultPasswordPage *>(m_pages[PageType::Password]);
            QString strPwd = passwordPage->getPassword();
            QString strClipher("");

            if (!InterfaceActiveVault::checkPassword(strPwd, strClipher)){
                // 设置密码输入框颜色
                passwordPage->lineEdit()->setStyleSheet(tr("background-color:rgb(245, 218, 217)"));

                //设置QToolTip颜色
                QPalette palette = QToolTip::palette();
                palette.setColor(QPalette::Inactive,QPalette::ToolTipBase,Qt::white);   //设置ToolTip背景色
                palette.setColor(QPalette::Inactive,QPalette::ToolTipText,QColor(255, 85, 0, 255)); 	//设置ToolTip字体色
                QToolTip::setPalette(palette);
                QRect rect(pos(), geometry().size());
                QToolTip::showText(passwordPage->mapToGlobal(QPoint(0, 50 )), tr("Wrong password"));

                return;
            }
        }else {
            // 密钥验证
            VaultKeyPage *keyPage = qobject_cast<VaultKeyPage *>(m_pages[PageType::Key]);
            QString strKey = keyPage->getKey();
            strKey.replace(tr("-"), tr(""));
            QString strClipher("");

            if (!InterfaceActiveVault::checkUserKey(strKey, strClipher)){
                //设置QToolTip颜色
                QPalette palette = QToolTip::palette();
                palette.setColor(QPalette::Inactive,QPalette::ToolTipBase,Qt::white);   //设置ToolTip背景色
                palette.setColor(QPalette::Inactive,QPalette::ToolTipText,QColor(255, 85, 0, 255)); 	//设置ToolTip字体色
                QToolTip::setPalette(palette);
                QRect rect(pos(), geometry().size());
                QToolTip::showText(keyPage->mapToGlobal(QPoint(0, 50 )), tr("Wrong recovery key"));

                return;
            }
        }

        m_bRemoveVault = true;
        // 验证成功，先对保险箱进行上锁
        VaultController::getVaultController()->lockVault();
        this->hide();
    }
        break;
    default:
        break;
    }
}

void DFMVaultRemovePages::onLockVault(int state)
{
    if (state == 0 && m_bRemoveVault){
        QString rmPath = VaultController::getVaultController()->vaultLockPath();        
        m_rmFileDialog->removeVault(rmPath);
        m_rmFileDialog->exec();
        accept();
    }else if (state != 0 && m_bRemoveVault) {
        // something to do
    }
}
