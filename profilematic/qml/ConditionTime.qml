/**********************************************************************
 * Copyright 2011-2012 Arto Jalkanen
 *
 * This file is part of ProfileMatic.
 *
 * ProfileMatic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ProfileMatic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ProfileMatic.  If not, see <http://www.gnu.org/licenses/>
**/
import QtQuick 1.1
import com.nokia.meego 1.0
import Rule 1.0
import "UIConstants.js" as UIConstants

Page {
    id: root
    tools: commonTools
    anchors.margins: UIConstants.DEFAULT_MARGIN

    property Rule    rule;

    SectionHeader {
        id: header
        section: "Time and day condition"
    }

    Flickable {
        anchors.fill: parent
        anchors.topMargin: header.height
        pressDelay: 140
        clip: true
        contentWidth: parent.width
        contentHeight: container.height // contentItem.childrenRect.height

        Column {
            id: container
            spacing: UIConstants.DEFAULT_MARGIN
            anchors.verticalCenter: parent.verticalCenter

            width: parent.width
            height: childrenRect.height

            RuleTopicSummary {
                topic: "Active days"
                summary: daysSummary()
                onTopicClicked: daysEditHandler()
                visible: !rule.isDefaultRule
            }

            RuleTopicSummary {
                topic: "Active start time"
                summary: timeStartSummary()
                onTopicClicked: timeStartEditHandler()
                visible: !rule.isDefaultRule
            }

            RuleTopicSummary {
                topic: "Active end time"
                summary: timeEndSummary()
                onTopicClicked: timeEndEditHandler()
                visible: !rule.isDefaultRule
            }

            Text {
                id: timeSummary
                wrapMode: Text.WordWrap
                width: parent.width
                // visible: isValidRule()
                font.pixelSize: UIConstants.FONT_SMALL;
                color: !theme.inverted ? UIConstants.COLOR_SECONDARY_FOREGROUND : UIConstants.COLOR_INVERTED_SECONDARY_FOREGROUND
                text: root.timeSummary()
                Connections {
                    target: rule
                    onDaysChanged:      timeSummary.text = root.timeSummary()
                    onTimeStartChanged: timeSummary.text = root.timeSummary()
                    onTimeEndChanged:   timeSummary.text = root.timeSummary()
                }
            }

        }
    }

    // Profile functions
    function timeSummary() {
        console.log("timeSummary")
        return backendRulesModel.getTimeSummaryText(rule, "");
    }

    function formatTime(hour, minute) {
        return (hour < 10 ? "0" : "") + hour + ":" + (minute < 10 ? "0" : "") + minute
    }

    TimeDialog {
        id: timeStartDialog
        titleText: "Active start time"
        onAccepted: rule.timeStart = formatTime(hour, minute)
    }

    // Start time functions
    function timeStartEditHandler() {
        var time = (rule.timeStart !== "" ? rule.timeStart : "00:00")
        var timeSplits = time.split(":")

        console.log("starTimeEditHandler timeSplits", timeSplits[0], timeSplits[1])

        timeStartDialog.hour = timeSplits[0]
        timeStartDialog.minute = timeSplits[1]

        timeStartDialog.open();
    }

    function timeStartSummary() {
        console.log("timeStart summary")
        if (rule.timeStart === '') {
            return "Click to set time"
        }
        return rule.timeStart
    }

    TimeDialog {
        id: timeEndDialog
        titleText: "Active end time"
        onAccepted: rule.timeEnd = formatTime(hour, minute)
    }

    // Start time functions
    function timeEndEditHandler() {
        var time = (rule.timeEnd !== "" ? rule.timeEnd : "00:00")
        var timeSplits = time.split(":")

        console.log("timeEndEditHandler timeSplits", timeSplits[0], timeSplits[1])

        timeEndDialog.hour = timeSplits[0]
        timeEndDialog.minute = timeSplits[1]

        timeEndDialog.open();
    }

    function timeEndSummary() {
        console.log("timeEnd summary")
        if (rule.timeEnd=== '') {
            return "Click to set time"
        }
        return rule.timeEnd
    }


    MyMultiSelectionDialog {
        id: daysDialog
        titleText: "Active days"
        platformStyle: SelectionDialogStyle {
            itemSelectedBackgroundColor: UIConstants.COLOR_SELECT
        }
        model: backendDaysModel
        acceptButtonText: "OK"
        onAccepted: rule.days = selectedIndexes
    }

    function daysSummary() {
        console.log("DaysSummary called")
        return backendRulesModel.getDaysSummaryText(rule.days);
    }

    function daysEditHandler() {
        daysDialog.selectedIndexes = rule.days
        daysDialog.open();
    }

}
