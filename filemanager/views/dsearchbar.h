#ifndef DSEARCHBAR_H
#define DSEARCHBAR_H

#include <QLineEdit>
#include <QListWidget>
#include <QListView>
#include <QMenu>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QStringList>

class DDirModel;

class DSearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DSearchBar(QWidget *parent = 0);
    ~DSearchBar();
    QListWidget * getPopupList();
    QAction * setClearAction();
    QAction * removeClearAction();
    bool isActive();
    void setActive(bool active);
    void keyPressEvent(QKeyEvent *e);
    QAction * getClearAction();
    bool hasScheme();
    bool isPath();
private:
    void initData();
    void initUI();
    QListWidget * m_list;
    QListWidget * m_hList;
    QCompleter * m_completer;
    QCompleter * m_historyCompleter;
    QAction * m_clearAction;
    QStringListModel * m_stringListMode;
    QStringList m_historyList;
    DDirModel * m_dirModel;
    bool m_isActive;
    void initConnections();
public slots:
    void doTextChanged(QString text);
    void searchHistoryLoaded(const QStringList &list);
    void historySaved();
protected:
    void focusInEvent(QFocusEvent *e);

signals:
    void searchBarFocused();
};

#endif // DSEARCHBAR_H
