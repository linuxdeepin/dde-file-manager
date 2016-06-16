#ifndef DBASEDIALOG_H
#define DBASEDIALOG_H

#include "dmovabledialog.h"

class QPushButton;
class QButtonGroup;
class QLabel;
class QCloseEvent;
class QVBoxLayout;

class DBaseDialog : public DMoveableDialog
{
    Q_OBJECT
public:
    explicit DBaseDialog(QWidget *parent = 0);
    ~DBaseDialog();
    void initUI(const QString& icon,
                const QString& message,
                const QString& tipMessage,
                const QStringList& buttonKeys,
                const QStringList& buttonTexts);
    void initConnect();

    QString getIcon();
    QString getMessage();
    QString getTipMessage();
    QStringList getButtons();
    QStringList getButtonTexts();
    QButtonGroup* getButtonsGroup();

signals:
    void aboutToClose();
    void closed();
    void buttonClicked(QString key);
    void buttonClicked(int index);

public slots:
   void setIcon(const QString& icon);
   void setMessage(const QString& message);
   void setTipMessage(const QString& tipMessage);
   void setButtons(const QStringList& buttons);
   void setButtonTexts(const QStringList& buttonTexts);
   void handleButtonsClicked(int id);
   void handleKeyEnter();
   QVBoxLayout* getMessageLayout();

protected:
   void closeEvent(QCloseEvent* event);
   void resizeEvent(QResizeEvent* event);

private:
    QString m_icon;
    QString m_message;
    QString m_tipMessage;
    QStringList m_buttonKeys;
    QStringList m_buttonTexts;
    int m_defaultWidth = 380;
    int m_defaultHeight = 120;

    QLabel* m_iconLabel;
    QLabel* m_messageLabel;
    QLabel* m_tipMessageLabel;
    QButtonGroup* m_buttonGroup;

    QVBoxLayout* m_messageLayout;
    int m_messageLabelMaxWidth;
    QPushButton* m_closeButton;
};

#endif // DBASEDIALOG_H
