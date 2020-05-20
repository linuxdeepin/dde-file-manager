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
#include <DMessageBox>

DWIDGET_USE_NAMESPACE

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
    m_pwdEdit->lineEdit()->setMaxLength(24);

    m_helpBtn = new QPushButton(this);
    m_helpBtn->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));
    m_helpBtn->setFixedSize(40, 36);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_pwdEdit);
    layout->addWidget(m_helpBtn);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    connect(m_helpBtn, &QPushButton::clicked, [this]{
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)){
            strPwdHint.insert(0, tr("Password hint:"));
            m_pwdEdit->showAlertMessage(strPwdHint);
        }
    });
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

void VaultPasswordPage::showAlertMessage(const QString &text, int duration)
{
    m_pwdEdit->showAlertMessage(text, duration);
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
    m_keyEdit = new QPlainTextEdit(this);
    m_keyEdit->setPlaceholderText(tr("Input the 32-digit recovery key"));
    m_keyEdit->installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_keyEdit);
    layout->setContentsMargins(10, 10, 10, 0);
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

QPlainTextEdit *VaultKeyPage::plainTextEdit()
{
    return m_keyEdit;
}

void VaultKeyPage::onRecoveryKeyChanged()
{
    QString key = m_keyEdit->toPlainText();
    int length = key.length();
    int maxLength = MAX_KEY_LENGTH + 7;

    // 限制密钥输入框只能输入数字、字母、以及+/-
    QRegExp rx("[a-zA-Z0-9-+/]+");
    QString res("");
    int pos = 0;
    while ((pos = rx.indexIn(key, pos)) != -1)
    {
        res += rx.cap(0);
        pos += rx.matchedLength();
    }
    key = res;

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

void DFMVaultRemovePages::removeFileInDir(const QString &path)
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
        }
    }

    QDir temp_dir;
    //删除文件夹
    temp_dir.rmdir(path);
}

void DFMVaultRemovePages::showEvent(QShowEvent *event)
{
    // 重置界面状态
    m_stackedLayout->setCurrentWidget(m_pages[PageType::Password]);
    qobject_cast<VaultPasswordPage *>(m_pages[PageType::Password])->clear();
    qobject_cast<VaultKeyPage *>(m_pages[PageType::Key])->clear();
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
                passwordPage->lineEdit()->setStyleSheet("background-color:rgb(245, 218, 217)");
                passwordPage->showAlertMessage(tr("Wrong password"));

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
                QToolTip::showText(keyPage->mapToGlobal(keyPage->plainTextEdit()->pos() + QPoint(0, keyPage->plainTextEdit()->height()/3)), tr("Wrong recovery key"));

                return;
            }
        }

        // 管理员权限认证
        if (VaultController::checkAuthentication()){
            m_bRemoveVault = true;
            // 验证成功，先对保险箱进行上锁
            VaultController::getVaultController()->lockVault();
        }
    }
        break;
    default:
        break;
    }
}

void DFMVaultRemovePages::onLockVault(int state)
{
    if (state == 0 && m_bRemoveVault){
        // 开启线程进行文件删除
        std::thread thread(
                [this]()
                {
                    QString rmPath = VaultController::getVaultController()->vaultLockPath();
                    removeFileInDir(rmPath);
                }
        );
        thread.detach();

        accept();
    }else if (state != 0 && m_bRemoveVault) {
        // something to do
        DMessageBox::information(this, tr("tips"), tr("Remove failed,the error code is %1").arg(state));
    }
    m_bRemoveVault = false;
}
