#include "i3pager.h"

const i3ipc::container_t* I3Pager::getLargestContainerInWorkspace(const i3ipc::container_t& container) {
    const i3ipc::container_t* largestContainer = nullptr;
    int largestArea = 0;

    if (container.type == "con") {
        int area = container.rect.width * container.rect.height;
        if (area > largestArea && !container.name.empty()) {
            largestArea = area;
            largestContainer = &container;
        }
    }

    for (const auto& node : container.nodes) {
        const i3ipc::container_t* childLargestContainer = getLargestContainerInWorkspace(*node);

        if (childLargestContainer) {
            int area = childLargestContainer->rect.width * childLargestContainer->rect.height;
            if (area > largestArea && !childLargestContainer->name.empty()) {
                largestArea = area;
                largestContainer = childLargestContainer;
            }
        }
    }

    return largestContainer;
}

const i3ipc::container_t* I3Pager::findWorkspaceRecursive(const i3ipc::container_t& container, const std::string& workspaceName) {
    if (container.type == "workspace" && container.name == workspaceName) {
        return &container;
    }

    for (const auto& node : container.nodes) {
        if (node->type == "workspace" && node->name == workspaceName) {
            return node.get();
        } else if (!node->nodes.empty()) {
            const i3ipc::container_t* result = findWorkspaceRecursive(*node, workspaceName);
            if (result) {
                return result;
            }
        }
    }

    return nullptr;
}

QString I3Pager::getIcon(const std::string& workspaceName) {
    auto tree = conn.get_tree();
    if (!tree) {
        throw std::runtime_error("Failed to get the tree.");
    }
    
    try {
        const i3ipc::container_t* targetWorkspace = findWorkspaceRecursive(*tree, workspaceName);
        if (!targetWorkspace) {
            throw std::invalid_argument("Workspace not found.");
        }

        const i3ipc::container_t* largestContainer = getLargestContainerInWorkspace(*targetWorkspace);
        if (!largestContainer) {
            throw std::invalid_argument("No container found in the workspace.");
        }

        if (largestContainer->window_properties.xclass.empty()) {
            throw std::invalid_argument("Largest container has empty xclass");
        }

        return iconConfig.getIcon(*largestContainer);
    } catch (std::invalid_argument const &e) {
        qWarning() << "Exception while retrieving container icon: " << e.what();
        return "";
    }
}

// void I3Pager::addMatchingRule(const QString& matchString, const QString& matchProperty, const QString& icon, bool useRegex) {
//     iconConfig.addMatchingRule(matchString, matchProperty, icon, useRegex);
// }

IconConfig::IconConfig()
    : m_matchingRules(
        {
            { "Firefox-esr", "xclass", false, "" },
            { "kitty", "xclass", false, "" },
            { "Code", "xclass", false, "" },
            { "^Burp Suite Professional.*", "title", true, "" }, // 
            { "^Microsoft Teams.*", "title", true, ""},
            { "vmware", "instance", false, ""},
            { "Chromium-browser", "xclass", false, ""},
            { "obsidian", "xclass", false, ""},
        }) {
    qDebug() << "IconConfig::IconConfig " << m_matchingRules.length();
}

// void IconConfig::addMatchingRule(const QString& matchString, const QString& matchProperty, const QString& icon, bool useRegex) {
//     matchingRules.append({matchString, matchProperty, useRegex, icon});
//     saveMatchingRules();
// }

// void IconConfig::loadMatchingRules()
// {
//     // Create a QSettings object to read the matching rules
//     QSettings settings;

//     // Begin the group for the matching rules
//     settings.beginGroup("MatchingRules");

//     // Get the number of rules stored in the settings
//     int numRules = settings.childGroups().size();

//     // Clear the existing matchingRules list
//     m_matchingRules.clear();

//     // Load each rule from the settings and add it to matchingRules
//     for (int i = 0; i < numRules; ++i)
//     {
//         const QString ruleGroup = QString("Rule%1").arg(i);
//         settings.beginGroup(ruleGroup);
//         MatchingRule rule;
//         rule.matchString = settings.value("MatchString").toString();
//         rule.matchProperty = settings.value("MatchProperty").toString();
//         rule.useRegex = settings.value("UseRegex").toBool();
//         rule.icon = settings.value("Icon").toString();
//         m_matchingRules.append(rule);
//         settings.endGroup();
//     }

//     // End the group for the matching rules
//     settings.endGroup();

//     // Emit signal to notify that matchingRules has changed
//     // Q_EMIT matchingRulesChanged();
// }

// void IconConfig::saveMatchingRules() const
// {
//     QSettings settings;
//     settings.beginGroup("MatchingRules/Defaults");

//     // Save default matching rules
//     settings.setValue("count", matchingRules.count());
//     for (int i = 0; i < matchingRules.count(); ++i) {
//         const MatchingRule& rule = matchingRules.at(i);
//         settings.beginGroup(QString::number(i));
//         settings.setValue("matchString", rule.matchString);
//         settings.setValue("matchProperty", rule.matchProperty);
//         settings.setValue("useRegex", rule.useRegex);
//         settings.setValue("icon", rule.icon);
//         settings.endGroup();
//     }

//     settings.endGroup();
//     settings.sync();
// }

//  const QList<IconConfig::MatchingRule>& getMatchingRules() const {
//     return matchingRules;
// }


QString IconConfig::getIcon(const i3ipc::container_t& container) const {
    for (const auto& rule : m_matchingRules) {
        std::string value;

        if (rule.m_matchProperty == "xclass") {
            value = container.window_properties.xclass;
        } else if (rule.m_matchProperty == "instance") {
            value = container.window_properties.instance;
        } else if (rule.m_matchProperty == "title") {
            value = container.window_properties.title;
        } else {
            throw std::invalid_argument("Invalid property specified.");
        }
        qDebug() << "Matching icon (" << QString::fromStdString(value) << " -> " << rule.m_matchString << ")";
        bool match = rule.m_useRegex ? std::regex_match(value, std::regex(rule.m_matchString.toStdString())) : (value == rule.m_matchString.toStdString());

        if (match) {
            return rule.m_icon;
        }
    }

    return ""; // Default Icon
}

I3Pager::I3Pager(QObject* parent)
    : QObject(parent) {
    currentScreenPrivate = QString();
    mode = "default";

    qDebug() << "Starting i3 listener";
    this->i3ListenerThread = new I3ListenerThread(this);
    connect(i3ListenerThread, &I3ListenerThread::modeChanged, this, [=](const QString& mode) {
        this->mode = mode;
        Q_EMIT modeChanged();
    });
    connect(i3ListenerThread, &I3ListenerThread::workspacesChanged, this, [=]() {
        Q_EMIT workspacesChanged();
    });
    connect(i3ListenerThread, &I3ListenerThread::finished, i3ListenerThread, &QObject::deleteLater);
    i3ListenerThread->start();
    qDebug() << "i3 listener started";
}

I3Pager::~I3Pager() {
    qDebug() << "I3Pager destructor";
    this->i3ListenerThread->stop();
    this->i3ListenerThread->wait();
    qDebug() << "I3Pager destructor done";
}

QList<QString> I3Pager::getScreenNames() {
    QList<QString> screenList;
    try {
        auto screens = conn.get_outputs();

        for (auto& screen : screens) {
            if (screen->active) {
                screenList.append(QString::fromStdString(screen->name));
                qDebug() << "Screen name:" << QString::fromStdString(screen->name);
            }
        }
    } catch (std::exception const& e) {
        qWarning() << "Exception while retrieving screen names: " << e.what();
    }

    return screenList;
}

QList<Workspace> I3Pager::getWorkspaces(bool filterByCurrentScreen, QString orderWorkspacesBy, QList<QString> screenOrder) {
    QList<Workspace> workspaceList;
    try {
        auto i3workspaceList = conn.get_workspaces();
        qDebug() << "Loading workspaces:";

        for (auto& i3workspace : i3workspaceList) {
            Workspace workspace;

            auto splitName = QString::fromStdString(i3workspace->name).split(':');
            workspace.id = QString::fromStdString(i3workspace->name);
            workspace.index = splitName.size() == 1 ? splitName[0] : splitName[1];
            workspace.name = splitName.size() == 3 ? splitName[2] : "";
            workspace.icon = getIcon(i3workspace->name);

            workspace.output = QString::fromStdString(i3workspace->output);
            workspace.focused = i3workspace->focused;
            workspace.visible = i3workspace->visible;
            workspace.urgent = i3workspace->urgent;

            qDebug() << "i3Workspace name:" << QString::fromStdString(i3workspace->name);
            qDebug() << "Workspace:"
                    << "id:" << workspace.id
                    << "index:" << workspace.index
                    << "name:" << workspace.name
                    << "icon:" << workspace.icon
                    << "focused:" << workspace.focused
                    << "urgent:" << workspace.urgent
                    << "visible:" << workspace.visible
                    << "output:" << workspace.output;
            workspaceList.append(workspace);
        }
    } catch (std::exception const& e) {
        qWarning() << "Exception while retrieving workspaces:" << e.what();
    }

    if (filterByCurrentScreen) {
        workspaceList = Workspace::filterByCurrentScreen(workspaceList, this->currentScreenPrivate);
    }

    if (orderWorkspacesBy == "screen" && !screenOrder.isEmpty()) {
        workspaceList = Workspace::orderByOutput(workspaceList, screenOrder);
    }
    else if (orderWorkspacesBy == "name") {
        workspaceList = Workspace::orderByName(workspaceList);
    }

    return workspaceList;
}

void I3Pager::activateWorkspace(QString workspace) {
    i3ipc::connection conn;
    conn.send_command("workspace " + workspace.toStdString());
}

void I3Pager::setCurrentScreen(QString screen) {
    this->currentScreenPrivate = screen;
    Q_EMIT workspacesChanged();
}

QString I3Pager::getMode() {
    return mode;
}

QString I3Pager::getCurrentScreen() {
    return this->currentScreenPrivate;
}
