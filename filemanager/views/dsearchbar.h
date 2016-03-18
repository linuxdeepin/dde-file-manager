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

private:
    QListWidget * m_list;
    QListWidget * m_hList;
    QCompleter * m_completer;
    QCompleter * m_historyCompleter;
    QAction * m_clearAction;
    QStringListModel * m_stringListMode;
    QStringList m_historyList;
    DDirModel * m_dirModel;
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
