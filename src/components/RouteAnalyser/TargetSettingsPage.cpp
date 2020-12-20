/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of Pingnoo (https://github.com/fizzyade/pingnoo)
 *
 * An open-source cross-platform traceroute analyser.
 *
 * Created by Adrian Carpenter on 20/12/2020.
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
// You may need to build the project (run Qt uic code generator) to get "ui_LatencySettingsPage.h" resolved

#include "TargetSettingsPage.h"

#include "TargetSettingsPageWidget.h"

Nedrysoft::RouteAnalyser::TargetSettingsPage::TargetSettingsPage(QWidget *parent) {

}

Nedrysoft::RouteAnalyser::TargetSettingsPage::TargetSettingsPage::~TargetSettingsPage() {

}

QString Nedrysoft::RouteAnalyser::TargetSettingsPage::section() {
    return "Route Analyser";
}


QString Nedrysoft::RouteAnalyser::TargetSettingsPage::category() {
    return "Target";
}

QString Nedrysoft::RouteAnalyser::TargetSettingsPage::description() {
    return "";
}

QIcon Nedrysoft::RouteAnalyser::TargetSettingsPage::icon() {
    return QIcon(":/RouteAnalyser/icons/analyser.png");
}

QWidget *Nedrysoft::RouteAnalyser::TargetSettingsPage::widget() {
    return new TargetSettingsPageWidget;
}

bool Nedrysoft::RouteAnalyser::TargetSettingsPage::canAcceptSettings() {
    return true;
}

void Nedrysoft::RouteAnalyser::TargetSettingsPage::acceptSettings() {

}
