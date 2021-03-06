/**********************************************************************
 * Copyright 2011 Arto Jalkanen
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
#include <QSettings>
#include <QUuid>
#include <QDebug>

#include "configuration.h"
#include "model/presencerule.h"

Configuration::Configuration()
{
}

void
Configuration::writeRules(const QList<Rule> &rules) {
    qDebug("Configuration::write()");

    // IMPROVE constants.h
    // So wrong by the API specifications, but so right by the end results (no, I don't like doing it this way)
    QSettings s("ProfileMatic", "rules");
    s.clear();
    s.setValue("rulesVersion", RULES_VERSION);
    s.beginWriteArray("rules", rules.size());
    for (int i = 0; i < rules.size(); ++i) {
        s.setArrayIndex(i);

        const Rule &r = rules.at(i);
        // s.setValue("ruleActive", r.ruleActive);

        s.setValue("ruleId", r.getRuleId());
        s.setValue("ruleName", r.getRuleName());
        _writeIntList(s, "days", "dayId", r.getDays().toList());
        _writeIntList(s, "locationCells", "cellId", r.getLocationCells().toList());
        _writeStringList(s, "wlans", "wlanName", r.getWlan().toList());
        s.setValue("wlanTimeout", r.getWlanTimeout());
        s.setValue("timeStart", r.getTimeStart().toString());
        s.setValue("timeEnd", r.getTimeEnd().toString());
        s.setValue("profile", r.getProfile());
        s.setValue("restoreProfile", r.getRestoreProfile());
        s.setValue("profileVolume", r.getProfileVolume());
        s.setValue("flightMode", r.getFlightMode());
        s.setValue("restoreFlightMode", r.getRestoreFlightMode());
        s.setValue("powerSavingMode", r.getPowerSavingMode());
        s.setValue("restorePowerSavingMode", r.getRestorePowerSavingMode());
        s.setValue("blueToothMode", r.getBlueToothMode());
        s.setValue("restoreBlueToothMode", r.getRestoreBlueToothMode());
        s.setValue("cellularMode", r.getCellularMode());
        s.setValue("commandLine", r.getCommandLine());
        _writePresenceRuleList(s, r.presenceRules());
        s.setValue("presenceStatusMessage", r.getPresenceStatusMessage());
        s.setValue("restorePresence", r.getRestorePresence());
        s.setValue("presenceChangeType", int(r.getPresenceChangeType()));
    }
    s.endArray();
}

void
Configuration::readRules(QList<Rule> &rules, int *rules_version_return) {
    // IMPROVE constants.h
    // So wrong by the API specifications, but so right by the end results (no, I don't like doing it this way)
    QSettings s("ProfileMatic", "rules");
    // QSettings s("ajalkane", "ProfileMatic");
    Rule defaultRule = Rule::createDefaultRule();
    int rules_version = s.value("rulesVersion", 0).toInt();
    if (rules_version_return != 0) {
        *rules_version_return = rules_version;
    }
    int size = s.beginReadArray("rules");
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);

        Rule r;
        // r.ruleActive = s.value("ruleActive").toBool();

        _assignRuleId(r, s.value("ruleId"));
        r.setRuleName(s.value("ruleName").toString());
        QList<int> daysList;
        _readIntList(s, "days", "dayId", daysList);
        r.setDays(QSet<int>::fromList(daysList));

        QList<int> locationCells;
        _readIntList(s, "locationCells", "cellId", locationCells);
        r.setLocationCells(QSet<int>::fromList(locationCells));

        QList<QString> wlans;
        _readStringList(s, "wlans", "wlanName", wlans);
        r.setWlan(QSet<QString>::fromList(wlans));

        bool wlanTimeoutOk = false;
        int wlanTimeout = s.value("wlanTimeout").toInt(&wlanTimeoutOk);
        if (wlanTimeoutOk) {
            r.setWlanTimeout(wlanTimeout);
        }

        QString timeStartStr = s.value("timeStart").toString();
        QString timeEndStr = s.value("timeEnd").toString();
        r.setTimeStart(QTime::fromString(timeStartStr));
        r.setTimeEnd(rules_version == 0
                     ? r.getTimeStart().addSecs(60)
                     : QTime::fromString(timeEndStr));
        r.setProfile(s.value("profile").toString());
        r.setRestoreProfile(s.value("restoreProfile", false).toBool());
        bool profileVolumeOk = false;
        int profileVolume = s.value("profileVolume").toInt(&profileVolumeOk);
        if (profileVolumeOk) {
            r.setProfileVolume(profileVolume);
        }

        bool flightModeOk = false;
        int flightMode = s.value("flightMode").toInt(&flightModeOk);
        if (flightModeOk) {
            r.setFlightMode(flightMode);
        }
        r.setRestoreFlightMode(s.value("restoreFlightMode", false).toBool());

        bool powerSavingModeOk = false;
        int powerSavingMode = s.value("powerSavingMode").toInt(&powerSavingModeOk);
        if (powerSavingModeOk) {
            r.setPowerSavingMode(powerSavingMode);
        }
        r.setRestorePowerSavingMode(s.value("restorePowerSavingMode", false).toBool());

        bool blueToothModeOk = false;
        int blueToothMode = s.value("blueToothMode").toInt(&blueToothModeOk);
        if (blueToothModeOk) {
            r.setBlueToothMode(blueToothMode);
        }
        r.setRestoreBlueToothMode(s.value("restoreBlueToothMode", false).toBool());

        bool cellularModeOk = false;
        int cellularMode = s.value("cellularMode").toInt(&cellularModeOk);
        if (cellularModeOk) {
            r.setCellularMode(cellularMode);
        }
        r.setCommandLine(s.value("commandLine").toString());

        QList<PresenceRule *> presenceRules;
        _readPresenceRuleList(s, presenceRules);
        r.setPresenceRules(presenceRules);

        qDeleteAll(presenceRules);
        presenceRules.clear();

        r.setPresenceStatusMessage(s.value("presenceStatusMessage").toString());
        r.setRestorePresence(s.value("restorePresence", r.getRestorePresence()).toBool());
        r.setPresenceChangeType((Rule::PresenceChangeType) s.value("presenceChangeType", (int) Rule::CustomPresenceType).toInt());

        // Make sure default rule is always last, and is created if it does not exist
        if (!r.isDefaultRule()) {
            rules << r;
        } else {
            defaultRule.actionsFrom(r);
        }
        qDebug("Configuration: index %d, ruleId: %s, ruleName: %s", i, qPrintable(r.getRuleId()), qPrintable(r.getRuleName()));
    }
    rules << defaultRule;
    s.endArray();

    // Write rules to finalize conversion
    if (rules_version == 0) {
        qDebug("Writing rules after conversion");
        writeRules(rules);
    }
}

void
Configuration::writePreferences(const Preferences &p) {
    qDebug("Configuration::writePreferences()");

    // IMPROVE constants.h
    // So wrong by the API specifications, but so right by the end results (no, I don't like doing it this way)
    QSettings s("ProfileMatic", "preferences");
    s.clear();
    s.setValue("isActive", p.isActive);
}

void
Configuration::readPreferences(Preferences &p) {
    // IMPROVE constants.h
    // So wrong by the API specifications, but so right by the end results (no, I don't like doing it this way)
    QSettings s("ProfileMatic", "rules");
    p.isActive = s.value("isActive", true).toBool();
}

void
Configuration::_assignRuleId(Rule &r, const QVariant &ruleIdVar) {
    // First released version did not set ruleIds, and it was assumed to be int.
    // Now it is changed to be UUID, a string. If the ruleId is a string, create
    // a new UUID
    bool ok = true;
    int oldIntId = ruleIdVar.toInt(&ok);
    QString ruleId;
    if (ok) {
        ruleId = QUuid::createUuid().toString();
        qDebug("ruleId was in old format (%d), creating UUID %s", oldIntId, qPrintable(ruleId));
    } else {
        ruleId = ruleIdVar.toString();
    }
    r.setRuleId(ruleId);
}

void Configuration::_writePresenceRuleList(QSettings &s, const QList<PresenceRule *> &rules)
{
    s.beginWriteArray("presenceRules");
    for (int row = 0; row < rules.length(); row++) {
        const PresenceRule *rule = rules[row];
        s.setArrayIndex(row);
        s.setValue("accountId", QVariant::fromValue(rule->accountId()));
        s.setValue("serviceName", rule->serviceName());
        s.setValue("action", int(rule->action()));
        s.setValue("statusMessage", rule->statusMessage());
    }
    s.endArray();
}

void Configuration::_readPresenceRuleList(QSettings &s, QList<PresenceRule *> &rules)
{
    int size = s.beginReadArray("presenceRules");
    for (int row = 0; row < size; row++) {
        s.setArrayIndex(row);

        if (!s.value("accountId").canConvert<Accounts::AccountId>()
                || !s.value("action").canConvert<int>()) {
            qWarning() << "Skipping invalid configuration entry.";
            continue;
        }

        Accounts::AccountId accountId
                = s.value("accountId").value<Accounts::AccountId>();
        QString serviceName
                = s.value("serviceName").toString();
        PresenceRule::Action action
                = (PresenceRule::Action) s.value("action").toInt();
        QString statusMessage
                = s.value("statusMessage").toString();

        rules.append(new PresenceRule(accountId, serviceName, action, statusMessage));
    }
    s.endArray();
}

void
Configuration::_writeIntList(QSettings &s, const QString &prefix, const QString &key, const QList<int> &values) {
    qDebug("QSettings:__writeIntList prefix %s, key %s, size %d", qPrintable(prefix), qPrintable(key), values.size());
    s.beginWriteArray(prefix, values.size());
    for (int i = 0; i < values.size(); ++i) {
        qDebug("Writing prefix %s, key %s for index %d, size %d", qPrintable(prefix), qPrintable(key), i, values.size());
        s.setArrayIndex(i);
        s.setValue(key, values.at(i));
    }
    s.endArray();
}

void
Configuration::_readIntList(QSettings &s, const QString &prefix, const QString &key, QList<int> &values) {
    int size = s.beginReadArray(prefix);
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        bool ok = false;
        QString valueStr = s.value(key).toString();
        int value = valueStr.toInt(&ok);
        if (ok) {
            values << value;
        } else {
            qDebug("Configuration: invalid setting (not an int) at %s/%d/%s: %s",
                   qPrintable(prefix), i, qPrintable(key), qPrintable(valueStr));
        }
    }
    s.endArray();
}

void
Configuration::_writeStringList(QSettings &s, const QString &prefix, const QString &key, const QList<QString> &values) {
    qDebug("QSettings:__writeStringList prefix %s, key %s, size %d", qPrintable(prefix), qPrintable(key), values.size());
    s.beginWriteArray(prefix, values.size());
    for (int i = 0; i < values.size(); ++i) {
        qDebug("Writing prefix %s, key %s for index %d, size %d", qPrintable(prefix), qPrintable(key), i, values.size());
        s.setArrayIndex(i);
        s.setValue(key, values.at(i));
    }
    s.endArray();
}

void
Configuration::_readStringList(QSettings &s, const QString &prefix, const QString &key, QList<QString> &values) {
    int size = s.beginReadArray(prefix);
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        QString valueStr = s.value(key).toString();
        values << valueStr;
    }
    s.endArray();
}
