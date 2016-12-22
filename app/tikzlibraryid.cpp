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

#include "tikzlibraryid.h"

#include <QStringBuilder>

/*TikzLibraryId::TikzLibraryId()
{
}*/


TikzLibraryId::TikzLibraryId(const QString &mainLibName,
												 const QString &subLibName):
	m_mainName(mainLibName),
	m_subName(subLibName),
	m_mainKey(mainLibName.toLower()),
	m_subKey(subLibName.toLower()),
	m_fullKey(m_mainKey % QLatin1Char('.') % m_subKey )
{

}

bool TikzLibraryId::operator ==(const TikzLibraryId &other) const
{
	return m_mainKey == other.m_mainKey
			&&
			m_subKey == other.m_subKey;
}

TikzLibraryId TikzLibraryId::fromFullKey(const QString &key)
{
	int i = key.indexOf(QChar::fromLatin1('.'));
	if( i==-1)
		return TikzLibraryId(key);
	else
		return TikzLibraryId(key.left(i), key.mid(i+1));
}





uint qHash(const QSet<TikzLibraryId> &key)
{
	uint res = 0;
	Q_FOREACH(const TikzLibraryId& id, key)
		res  ^= qHash(id);
	return res;
}
uint qHash(const QSet<TikzLibraryId> &key, uint seed)
{
	uint res = 0;
	Q_FOREACH(const TikzLibraryId& id, key)
		res  ^= qHash(id, seed);
	return res;
}
