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

#include "ICMPAPIPingComponent.h"

#include "ICMPAPIPingEngineFactory.h"

#include <IComponentManager>
#include <windows.h>
#include <spdlog/spdlog.h>

ICMPAPIPingComponent::ICMPAPIPingComponent()  :
        m_pingEngineFactory(nullptr) {

}

ICMPAPIPingComponent::~ICMPAPIPingComponent() {
    if (m_pingEngineFactory) {
        Nedrysoft::ComponentSystem::removeObject(m_pingEngineFactory);

        delete m_pingEngineFactory;
    }
};

auto ICMPAPIPingComponent::initialiseEvent() -> void {
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,2), &wsaData)!=0) {
        spdlog::warn("error initialising winsock");
    }

    m_pingEngineFactory = new Nedrysoft::ICMPAPIPingEngine::ICMPAPIPingEngineFactory();

    Nedrysoft::ComponentSystem::addObject(m_pingEngineFactory);
}


