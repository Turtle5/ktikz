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

#include "tikzlibrary.h"
#include <QStringBuilder>
#include <QSet>

TikzLibrary::TikzLibrary(const TikzLibraryId &id):
	m_id(id)
{

}
//QString TikzLibrary::pgfRootDir() const
//{
//	return m_pgfRootDir;
//}
//void TikzLibrary::setPgfRootDir(const QString &pgfRootDir)
//{
//	m_pgfRootDir = pgfRootDir;
//}

QString TikzLibrary::useSubLibCommand() const
{
	return m_useSubLibCommand;
}
void TikzLibrary::setUseSubLibCommand(const QString &usePackageForSubLib)
{
	m_useSubLibCommand = usePackageForSubLib;
}
bool TikzLibrary::isDefined() const
{
	return m_isDefined;
}

void TikzLibrary::setDefined(bool isDeclared)
{
	m_isDefined = isDeclared;
}



