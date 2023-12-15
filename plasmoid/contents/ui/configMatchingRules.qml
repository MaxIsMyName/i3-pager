import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.4 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.configuration 2.0
import I3Pager 1.0

Kirigami.FormLayout {
    id: page
    // implicitWidth: Kirigami.Units.gridUnit * 40 // Set a fixed width for the FormLayout
    // height: childrenRect.height

    property alias cfg_matchingRules: matchingRules

    // Component.onCompleted: {
    //     // Set the default matching rules from the configuration file if not already set
    //     if (cfg_matchingRules.length === 0) {
    //         cfg_matchingRules = plasmoid.configuration.matchingRules;
    //     }
    // }

    ColumnLayout {
        Kirigami.FormData.label: i18n("Matching Rules")

        ColumnLayout {
            Repeater {
                model: cfg_matchingRules

                RowLayout {
                    spacing: units.largeSpacing

                    TextField {
                        width: page.width / 4
                        placeholderText: i18n("Match String")
                        text: model.matchString
                        onTextChanged: model.matchString = text
                    }

                    ComboBox {
                        width: page.width / 4
                        model: ["xclass", "instance", "title"]
                        onCurrentTextChanged: model.matchProperty = currentText
                    }

                    CheckBox {
                        text: i18n("Use Regex")
                        checked: model.useRegex
                        onCheckedChanged: model.useRegex = checked
                    }

                    TextField {
                        width: page.width / 4
                        placeholderText: i18n("Icon")
                        text: model.icon
                        onTextChanged: model.icon = text
                    }

                    Button {
                        text: i18n("Remove")
                        onClicked: cfg_matchingRules.remove(index)
                    }
                }
            }
        }
    }

    Button {
        text: i18n("Add Matching Rule")
        onClicked: cfg_matchingRules.push({
            "matchString": "",
            "matchProperty": "xclass",
            "useRegex": false,
            "icon": ""
        })
    }
}

