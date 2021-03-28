/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of Pingnoo (https://github.com/nedrysoft/pingnoo)
 *
 * An open-source cross-platform traceroute analyser.
 *
 * Created by Adrian Carpenter on 28/03/2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RouteEngineWorker.h"

#include "Core/IPingEngine.h"
#include "Core/IPingEngineFactory.h"
#include "spdlog.h"

#include <QHostInfo>

constexpr auto DefaultDiscoveryTimeout = 1.0;
constexpr auto MaxRouteHops = 64;

Nedrysoft::RouteEngine::RouteEngineWorker::RouteEngineWorker(
        QString host,
        Nedrysoft::Core::IPingEngineFactory *pingEngineFactory,
        Nedrysoft::Core::IPVersion ipVersion ) {

    m_host = host;
    m_pingEngineFactory = pingEngineFactory;
    m_ipVersion = ipVersion;
    m_isRunning = false;
}

Nedrysoft::RouteEngine::RouteEngineWorker::~RouteEngineWorker() {
    if (m_isRunning) {
        m_isRunning = false;

        thread()->quit();
    }
}

auto Nedrysoft::RouteEngine::RouteEngineWorker::doWork() -> void {
    m_isRunning = true;

    auto pingEngine = m_pingEngineFactory->createEngine(m_ipVersion);

    auto targetAddresses = QHostInfo::fromName(m_host).addresses();

    if (!targetAddresses.count()) {
        Q_EMIT result(QHostAddress(), Nedrysoft::Core::RouteList());

        SPDLOG_ERROR(QString("Failed to find address for %1.").arg(m_host).toStdString());

        this->deleteLater();

        return;
    }

    auto route = Nedrysoft::Core::RouteList();

    for (int hop=1;hop<MaxRouteHops;hop++) {
        if (!m_isRunning) {
            m_pingEngineFactory->deleteEngine(pingEngine);

            return;
        }

        auto result = pingEngine->singleShot(targetAddresses.at(0), hop, DefaultDiscoveryTimeout);

        if (result.code()==Nedrysoft::Core::PingResult::ResultCode::Ok) {
            route.append(result.hostAddress());
            break;
        } else  if (result.code()==Nedrysoft::Core::PingResult::ResultCode::TimeExceeded) {
            route.append(result.hostAddress());
        } else  {
            route.append(QHostAddress());
        }
    };

    SPDLOG_TRACE(QString("Route to %1 (%2) completed, total of %3 hops.")
                         .arg(m_host)
                         .arg(m_targetAddresses[0].toString())
                         .arg(route.length())
                         .toStdString() );

    Q_EMIT result(targetAddresses[0], route);

    m_pingEngineFactory->deleteEngine(pingEngine);

    this->deleteLater();
}