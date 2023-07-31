#ifndef I3PAGER_PLASMOID_H
#define I3PAGER_PLASMOID_H

#include "i3listener.h"
#include "workspace.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QList>
#include <QMetaType>
#include <QScreen>
#include <QSettings>
#include <QVariant>
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QObject>
#include <future>
#include <i3ipc++/ipc.hpp>
#include <regex>
#include <unordered_map>

// struct MatchingRule : public QObject{
//     Q_OBJECT
//     Q_PROPERTY(QString matchString MEMBER m_matchString)
//     Q_PROPERTY(QString matchProperty MEMBER m_matchProperty)
//     Q_PROPERTY(bool useRegex MEMBER m_useRegex)
//     Q_PROPERTY(QString icon MEMBER m_icon)

// public:
//     QString m_matchString;
//     QString m_matchProperty;
//     bool m_useRegex;
//     QString m_icon;

//     bool operator==(const MatchingRule& other) const {
//         return m_matchString == other.m_matchString && m_matchProperty == other.m_matchProperty && m_useRegex == other.m_useRegex && m_icon == other.m_icon;
//     }

//     bool operator!=(const MatchingRule& other) const {
//         return !(*this == other);
//     }

//     ~MatchingRule() {}
// };

// Q_DECLARE_METATYPE(MatchingRule);

struct MatchingRule {
    QString m_matchString;
    QString m_matchProperty;
    bool m_useRegex;
    QString m_icon;
};

class IconConfig{
    // Q_OBJECT
    // Q_PROPERTY(QList<MatchingRule> matchingRules MEMBER m_matchingRules)

public:
    IconConfig();
    QString getIcon(const i3ipc::container_t& container) const;

private:
    QList<MatchingRule> m_matchingRules;
};

class I3Pager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentScreen READ getCurrentScreen WRITE setCurrentScreen)
    Q_PROPERTY(QString mode READ getMode NOTIFY modeChanged)

    i3ipc::connection conn;
    IconConfig iconConfig;

    const i3ipc::container_t* getLargestContainerInWorkspace(const i3ipc::container_t& container);
    const i3ipc::container_t* findWorkspaceRecursive(const i3ipc::container_t& container, const std::string& workspaceName);
    QString getIcon(const std::string& workspaceName);

public:
    explicit I3Pager(QObject* parent = 0);
    ~I3Pager();

    QString getCurrentScreen() const;
    void setCurrentScreen(QString screen);
    Q_INVOKABLE void activateWorkspace(QString workspace);

public Q_SLOTS:
    QString getCurrentScreen();
    QList<Workspace> getWorkspaces(bool filterByCurrentScreen, QString orderWorkspacesBy, QList<QString> screenOrder);
    QList<QString> getScreenNames();
    QString getMode();

Q_SIGNALS:
    void workspacesChanged();
    void modeChanged();

private:
    I3ListenerThread* i3ListenerThread;
    QString currentScreenPrivate;
    QString mode;
};

#endif // I3PAGER_PLASMOID_H
