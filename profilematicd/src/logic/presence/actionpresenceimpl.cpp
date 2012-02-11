/**********************************************************************
 * Copyright 2011-2012 Philip Lorenz
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

#include "actionpresenceimpl.h"

#include <TelepathyQt4/PendingReady>
#include <Accounts/Manager>

ActionPresenceImpl::ActionPresenceImpl() :
    _pendingRule(NULL)
{
    Tp::registerTypes();

    _manager = new Accounts::Manager(this);
    _accountManager = Tp::AccountManager::create();
    connect(_accountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

void ActionPresenceImpl::activate(const Rule &rule)
{
    if (!_accountManager->isReady(Tp::AccountManager::FeatureCore)) {
        qDebug() << "Rule was activated while Telepathy Account Manager was not ready - will retry as soon as it is ready.";

        _pendingRule = new Rule(rule);
        _pendingRule->setParent(this);

        return;
    }

    foreach(const PresenceRule *presenceRule, rule.presenceRules()) {
        Accounts::Account *account
                = _manager->account(presenceRule->accountId());
        Accounts::Service *selectedService = NULL;

        foreach (Accounts::Service *service, account->services()) {
            if (service->name() == presenceRule->serviceName()) {
                selectedService = service;
                break;
            }
        }

        if (!selectedService) {
            qWarning() << "Could not find service" << presenceRule->serviceName();
            continue;
        }

        account->selectService(selectedService);

        QString uid = account->valueAsString("tmc-uid");
        if (uid.isEmpty()) {
            qWarning() << "Failed to retrieve tmc-uid for account" << presenceRule->accountId();
            continue;
        }

        Tp::AccountPtr tpAccount =
                _accountManager->accountForPath(QString("/org/freedesktop/Telepathy/Account/%1").arg(uid));

        if (!tpAccount) {
            qWarning() << "Failed to retrieve Telepathy account for" << presenceRule->accountId() << uid;
            continue;
        }

        Tp::Presence presence;

        switch (presenceRule->action()) {
        case PresenceRule::SetOnline:
            if (tpAccount->isOnline())
                return;
            presence = Tp::Presence::available();
            break;
        case PresenceRule::SetOffline:
            presence = Tp::Presence::offline();
            break;
        default:
            continue;
        }

        Tp::PendingOperation *op = tpAccount->setRequestedPresence(presence);
        connect(op,
                SIGNAL(finished(Tp::PendingOperation*)),
                SLOT(onPresenceChangeFinished(Tp::PendingOperation*)));
    }
}

void ActionPresenceImpl::onPresenceChangeFinished(Tp::PendingOperation *op)
{
    if (op->isError())
        qWarning() << "Failed to change presence:" << op->errorMessage();
}

void ActionPresenceImpl::onAccountManagerReady(Tp::PendingOperation *op)
{
    if (op->isError()) {
        qWarning() << "Failed to instantiate account manager.";
        return;
    }

    if (!_pendingRule)
        return;

    activate(*_pendingRule);

    delete _pendingRule;
    _pendingRule = NULL;
}