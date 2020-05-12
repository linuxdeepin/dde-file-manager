#ifndef DFMVAULTREMOVEPAGES_H
#define DFMVAULTREMOVEPAGES_H

#include <dtkwidget_global.h>
#include <QWidget>
#include <DDialog>

class QPushButton;
class QLabel;
class QProgressBar;
class QToolButton;
class QLineEdit;
class QStackedLayout;
class QFrame;
class QPlainTextEdit;

DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

#define MAX_KEY_LENGTH (32) //凭证最大值，4的倍数
/**
* @brief    保险箱删除完成界面
*/
class VaultRemoveFileDialog : public DDialog
{
    Q_OBJECT
public:
    explicit VaultRemoveFileDialog(QWidget *parent = nullptr);
    ~VaultRemoveFileDialog() override;

    /**
    * @brief    恢复到初始状态
    */
    void clear();

public slots:
    void removeVault();

signals:
    void removeFinished();

    void removeResult(bool res);

private:
    /**
    * @brief    初始化UI界面
    */
    void initUI();

    /**
    * @brief    初始化信号槽
    */
    void initConnect();

    /**
    * @brief    统计文件数量
    * @param    文件夹路径
    */
    bool statisticsFiles(const QString &path);

    /**
    * @brief    删除文件及文件夹
    * @param    文件夹路径
    */
    void removeFileInDir(const QString &path);

private:
    QProgressBar *m_removeProgress;

    int m_iFiles = 0;   //文件、文件夹数量
    int m_iRmFiles = 0; //删除文件数量
    int m_iRmDir = 0;   //删除文件夹数量
};
//-----------------------------------------------------------

/**
* @brief    密码输入界面
*/
class VaultPasswordPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultPasswordPage(QWidget *patent = nullptr);
    ~VaultPasswordPage() override;

    /**
    * @brief    获取密码
    */
    QString getPassword();

    /**
    * @brief    清空密码
    */
    void clear();
private:
    /**
    * @brief    初始化UI界面
    */
    void initUI();

private:
    DPasswordEdit *m_pwdEdit;
    QPushButton *m_helpBtn;
};
//-----------------------------------------------------------

/**
* @brief    凭证输入界面
*/
class VaultKeyPage : public QWidget
{
    Q_OBJECT
public:
    explicit VaultKeyPage(QWidget *parent = nullptr);
    ~VaultKeyPage() override;

    /**
    * @brief    获取凭证
    */
    QString getKey();

    /**
    * @brief    清空凭证
    */
    void clear();
private:
    /**
    * @brief    初始化UI界面
    */
    void initUI();

    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QPlainTextEdit *m_keyEdit;
};

//-----------------------------------------------------------

// 页面类型，Password：密码输入页面， Key：密钥输入界面
enum class PageType { Password, Key};
class DFMVaultRemovePages : public DDialog
{
    Q_OBJECT
public:   
    static DFMVaultRemovePages *instance();

public slots:
    void onButtonClicked(int index, const QString &text);

private:
    explicit DFMVaultRemovePages(QWidget *parent = nullptr);
    ~DFMVaultRemovePages() override {}

    /**
    * @brief    初始化UI界面
    */
    void initUI();

    void initConnect();

    void insertPage(const PageType &pageType, QWidget * widget);

    void closeEvent(QCloseEvent *event) override;

private:
    VaultRemoveFileDialog *m_rmFileDialog;
    QMap<PageType, QWidget*> m_pages;    //存储页面
    QStackedLayout * m_stackedLayout;   //用于页面切换
    PageType m_currentPage; // 当前显示页面类型
};

#endif // DFMVAULTREMOVEPAGES_H
