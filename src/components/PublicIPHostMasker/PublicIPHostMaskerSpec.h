/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of pingnoo (https://github.com/fizzyade/pingnoo)
 * An open source ping path analyser
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

#ifndef FIZZYADE_PUBLICIPHOSTMASKER_PUBLICIPHOSTMASKERSPEC_H
#define FIZZYADE_PUBLICIPHOSTMASKER_PUBLICIPHOSTMASKERSPEC_H

#if defined(FIZZYADE_COMPONENT_PUBLICIPHOSTMASKER_EXPORT)
#define FIZZYADE_PUBLICIPHOSTMASKER_DLLSPEC Q_DECL_EXPORT
#else
#define FIZZYADE_PUBLICIPHOSTMASKER_DLLSPEC Q_DECL_IMPORT
#endif

#endif // FIZZYADE_PUBLICIPHOSTMASKER_PUBLICIPHOSTMASKERSPEC_H
