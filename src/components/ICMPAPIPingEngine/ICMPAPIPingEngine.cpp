/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of Pingnoo (https://github.com/nedrysoft/pingnoo)
 *
 * An open-source cross-platform traceroute analyser.
 *
 * Created by Adrian Carpenter on 27/03/2020.
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

#include "ICMPAPIPingEngine.h"

#include "ICMPAPIPingTarget.h"
#include "ICMPAPIPingTransmitter.h"

#include <QMutex>
#include <QThread>
#include <WS2tcpip.h>

#include <WinSock2.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>
#include <iostream>

constexpr auto DefaultTransmitTimeout = 1000;
constexpr auto DefaultReplyTimeout = 3000;
constexpr auto PingPayloadLength = 64;
constexpr auto NanosecondsInMillisecond = 1.0e6;

/**
 * @brief       Private class to store the engines instance data.
 */
class Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngineData {
    public:
        /**
         * @brief       Constructs a ICMPAPIPingEngineData.
         *
         * @param[in]   parent the ICMPAPIPingEngine instance that this data belongs to.
         */
        ICMPAPIPingEngineData(Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine *parent) :
                m_pingEngine(parent),
                m_transmitter(nullptr),
                m_transmitterThread(nullptr),
                m_timeout(DefaultReplyTimeout),
                m_ipVersion(Nedrysoft::Core::IPVersion::V4) {

        }

        friend class ICMPAPIPingEngine;

    private:
        Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine *m_pingEngine;

        Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingTransmitter *m_transmitter;

        QThread *m_transmitterThread;

        Nedrysoft::Core::IPVersion m_ipVersion;

        QList<Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingTarget *> m_targetList;

        int m_timeout;
        int m_interval;
};

Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::ICMPAPIPingEngine(Nedrysoft::Core::IPVersion version) :
        d(std::make_shared<Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngineData>(this)) {

    d->m_ipVersion = version;
}

Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::~ICMPAPIPingEngine() {
    doStop();

    d.reset();
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::addTarget(
        QHostAddress hostAddress) -> Nedrysoft::RouteAnalyser::IPingTarget * {

    Q_UNUSED(hostAddress)

    ICMPAPIPingTarget *pingTarget = new ICMPAPIPingTarget(this, hostAddress);

    d->m_targetList.append(pingTarget);

    return(pingTarget);
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::addTarget(
        QHostAddress hostAddress,
        int ttl ) -> Nedrysoft::RouteAnalyser::IPingTarget * {

    ICMPAPIPingTarget *pingTarget = new ICMPAPIPingTarget(this, hostAddress, ttl);

    d->m_targetList.append(pingTarget);

    return(pingTarget);
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::removeTarget(
        Nedrysoft::RouteAnalyser::IPingTarget *pingTarget) -> bool {

    Q_UNUSED(pingTarget)

    return true;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::start() -> bool {
    d->m_transmitter = new Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingTransmitter(this);

    d->m_transmitterThread = new QThread();

    d->m_transmitter->moveToThread(d->m_transmitterThread);

    connect(d->m_transmitterThread, &QThread::started, d->m_transmitter, &ICMPAPIPingTransmitter::doWork);

    connect(d->m_transmitter, &ICMPAPIPingTransmitter::result, this, &ICMPAPIPingEngine::result);

    for (auto target : d->m_targetList) {
        d->m_transmitter->addTarget(target);
    }

    d->m_transmitterThread->start();

    return true;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::stop() -> bool {
    doStop();

    return true;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::setInterval(int interval) -> bool {
    d->m_interval = interval;

    return true;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::interval() -> int {
    return d->m_interval;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::setTimeout(int timeout) -> bool {
    d->m_timeout = timeout;

    return true;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::saveConfiguration() -> QJsonObject {
    return QJsonObject();
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::loadConfiguration(QJsonObject configuration) -> bool {
    Q_UNUSED(configuration)

    return false;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::epoch() -> QDateTime {
    return QDateTime::currentDateTime();
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::singleShot(
        QHostAddress hostAddress,
        int ttl,
        double timeout ) -> Nedrysoft::RouteAnalyser::PingResult {

    QByteArray dataBuffer = QString("pingnoo ping").toLatin1();
    QByteArray replyBuffer;
    HANDLE icmpHandle;
    Nedrysoft::RouteAnalyser::PingResult::ResultCode resultCode =
            Nedrysoft::RouteAnalyser::PingResult::ResultCode::NoReply;

    Nedrysoft::RouteAnalyser::PingResult pingResult;
    QHostAddress replyHost;
    QElapsedTimer timer;
    int returnValue;
    sockaddr_in6 sourceAddress, targetAddress;

    auto epoch = QDateTime::currentDateTime();

#if defined(_WIN64)
    IP_OPTION_INFORMATION32 options;
#else
    IP_OPTION_INFORMATION options;
#endif

    options.Ttl = ttl;
    options.Flags = 0;
    options.OptionsData = nullptr;
    options.OptionsSize = 0;
    options.Tos = 0;

    PIP_OPTION_INFORMATION pipOptions = reinterpret_cast<PIP_OPTION_INFORMATION>(&options);

    if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol) {
        icmpHandle = IcmpCreateFile();

        replyBuffer.resize(sizeof(ICMP_ECHO_REPLY) + dataBuffer.length());
    } else {
        icmpHandle = Icmp6CreateFile();

        replyBuffer.resize(sizeof(ICMPV6_ECHO_REPLY) + dataBuffer.length());

        IN6ADDR_SETANY(&sourceAddress);
        IN6ADDR_SETANY(&targetAddress);

        memcpy(targetAddress.sin6_addr.u.Word, hostAddress.toIPv6Address().c, sizeof(targetAddress.sin6_addr));
    }

    timer.restart();

    if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol) {
        returnValue = IcmpSendEcho2(
            icmpHandle,
            nullptr,
            nullptr,
            nullptr,
            hostAddress.toIPv4Address(),
            dataBuffer.data(),
            static_cast<WORD>(dataBuffer.length()),
            pipOptions,
            replyBuffer.data(),
            static_cast<DWORD>(replyBuffer.length()),
            timeout*1000
        ); // NOLINT(cppcoreguidelines-pro-type-union-access)
    } else {
        returnValue = Icmp6SendEcho2(
            icmpHandle,
            nullptr,
            nullptr,
            nullptr,
            &sourceAddress,
            &targetAddress,
            dataBuffer.data(),
            static_cast<WORD>(dataBuffer.length()),
            pipOptions,
            replyBuffer.data(), static_cast<DWORD>(replyBuffer.length()),
            timeout*1000
        );  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    auto roundTripTime = timer.nsecsElapsed();

    if (returnValue) {
        if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol) {
            PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY) replyBuffer.data();

            replyHost = QHostAddress(ntohl(pEchoReply->Address));

            if (pEchoReply->Status == IP_SUCCESS) {
                resultCode = Nedrysoft::RouteAnalyser::PingResult::ResultCode::Ok;
            } else if (pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT) {
                resultCode = Nedrysoft::RouteAnalyser::PingResult::ResultCode::TimeExceeded;
            }
        } else {
            PICMPV6_ECHO_REPLY pEchoReply = (PICMPV6_ECHO_REPLY) replyBuffer.data();

            sockaddr_in6 replySocketAddress;

            memcpy(
                replySocketAddress.sin6_addr.u.Word,
                pEchoReply->Address.sin6_addr,
                sizeof(pEchoReply->Address.sin6_addr)
            );

            replyHost.setAddress(replySocketAddress.sin6_addr.u.Byte);

            if (pEchoReply->Status == IP_SUCCESS) {
                resultCode = Nedrysoft::RouteAnalyser::PingResult::ResultCode::Ok;
            } else if (pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT) {
                resultCode = Nedrysoft::RouteAnalyser::PingResult::ResultCode::TimeExceeded;
            }
        }
    }

    IcmpCloseHandle(icmpHandle);

    return Nedrysoft::RouteAnalyser::PingResult(
        0,
        resultCode,
        replyHost,
        epoch,
        roundTripTime/1e9,
        nullptr,
        -1
    );
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::targets() -> QList<Nedrysoft::RouteAnalyser::IPingTarget *> {
    QList<Nedrysoft::RouteAnalyser::IPingTarget *> list;

    for (auto target : d->m_targetList) {
        list.append(target);
    }

    return list;
}

auto Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngine::doStop() -> void {
    if (d->m_transmitter) {
        d->m_transmitter->m_isRunning = false;
    }

    if (d->m_transmitterThread) {
        d->m_transmitterThread->quit();

        d->m_transmitterThread->wait();

        delete d->m_transmitterThread;

        d->m_transmitterThread = nullptr;
    }

    if (d->m_transmitter) {
        delete d->m_transmitter;

        d->m_transmitter = nullptr;
    }
}