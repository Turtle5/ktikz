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

#ifndef TIKZLIBRARY_H
#define TIKZLIBRARY_H

#include <QString>
#include <QUrl>

#include "tikzlibraryid.h"

class TikzLibrary
{
public:
	TikzLibrary(const TikzLibraryId& id);

	const TikzLibraryId& id() const {return m_id;}

//	QString pgfRootDir() const;
//	void setPgfRootDir(const QString &pgfRootDir);

	/// Get the alternative Latex command to include a sublib.
	/// For instance, \usetikzlibrary
	QString useSubLibCommand() const;
	void setUseSubLibCommand(const QString &useSubLibCommand);

	/// If a library is used without declaration, it could be declare later without warning.
	bool isDefined() const;
	void setDefined(bool isDefined);

	void combineWith(const TikzLibrary& other);
	TikzLibrary combine(const TikzLibrary& other) const;

private:
	TikzLibraryId m_id;
	//	QString m_pgfRootDir;
	QString m_useSubLibCommand;
	QUrl m_webPage;
	bool m_isDefined;
};


#endif // TIKZLIBRARY_H
