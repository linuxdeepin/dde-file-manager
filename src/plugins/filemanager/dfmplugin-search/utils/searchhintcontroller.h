// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHINTCONTROLLER_H
#define SEARCHHINTCONTROLLER_H

#include "dfmplugin_search_global.h"

#include <dfm-base/widgets/viewhintmessage/viewhintmessage.h>

#include <QHash>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QVariantMap>

namespace dfmplugin_search {

class SearchHintController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchHintController)

public:
    enum class HintType {
        None,
        AuthHint,
        IndexFailed,
        IndexPausedBattery,
        IndexPausedPowerSave,
        IndexPausedIdle,
        IndexWaitingUpgrade,
        IndexUpdating,
    };

    static SearchHintController *instance();

    bool shouldShowAuthHint(QString *text) const;
    void authorizeSearchExperience();
    void dismissAuthHint();

    void tryShowHint(quint64 winId);
    void closeAllHints(quint64 winId);
    void closeHint(quint64 winId);

    void startListening();
    void stopListening();

private:
    explicit SearchHintController(QObject *parent = nullptr);
    ~SearchHintController() override;

    QStringList disabledSearchModes() const;

    bool hasSearchWindow() const;
    void onTextStatusResult(const QString &state, const QString &grade, bool success);
    void onOcrStatusResult(const QString &state, const QString &grade, bool success);
    void onTextStatusChanged(const QString &state, const QString &grade);
    void onOcrStatusChanged(const QString &state, const QString &grade);

    void evaluateAndShowAllWindows();
    void evaluateAndShow(quint64 winId);
    HintType evaluateHint(quint64 winId) const;
    QVariantMap buildHintContent(quint64 winId, HintType type) const;
    QString updatingHintText() const;
    void showHintToWindow(quint64 winId, HintType type, const QVariantMap &content);
    void onHintAction(quint64 winId, const QString &id);
    void dismissAndReevaluate(quint64 winId, HintType actedType);

    void openSettingsPage(quint64 winId) const;
    void requestUpdate(HintType type) const;

    struct WindowState
    {
        HintType currentType { HintType::None };
        QString currentText;
        QSet<HintType> dismissedTypes;
    };

    QHash<quint64, QPointer<DFMBASE_NAMESPACE::ViewHintMessage>> hints;
    QHash<quint64, WindowState> windowStates;

    QString m_textState;
    QString m_textGrade;
    QString m_ocrState;
    QString m_ocrGrade;
    bool m_textStatusValid { false };
    bool m_ocrStatusValid { false };
    bool m_listening { false };
};

}

#endif   // SEARCHHINTCONTROLLER_H
