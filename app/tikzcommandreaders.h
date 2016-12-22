/***************************************************************************
 *   Copyright (C) 2016 by G. Prudhomme									*
 *	 <gprud@users.noreply.github.com>									*
 *																		 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	 *
 *   (at your option) any later version.								   *
 *																		 *
 *   This program is distributed in the hope that it will be useful,	   *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of		*
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		 *
 *   GNU General Public License for more details.						  *
 *																		 *
 *   You should have received a copy of the GNU General Public License	 *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef TIKZCOMMANDREADERS_H
#define TIKZCOMMANDREADERS_H

#include "tikzcommand.h"

class TikzCommandReaders
{
private:
	TikzCommandReaders();
public:

	static TikzCommandList fromXml(const QString& filePath, QList<TikzCommand> *tikzCommandsList);
	static TikzCommandFileContent fromXml(const QString& filePath);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	static TikzCommandList fromJson(const QString& filePath, QList<TikzCommand> *tikzCommandsList);
	static TikzCommandFileContent fromJson(const QString& filePath);
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

};

#endif // TIKZCOMMANDREADERS_H
