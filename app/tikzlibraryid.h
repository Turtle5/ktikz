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

#ifndef TIKZLIBRARYID_H
#define TIKZLIBRARYID_H

#include <QString>
#include <QHash>
#include <QSet>

/// Tikz library ID for looking-up inside map
class TikzLibraryId
{
  public:
	TikzLibraryId(const QString& mainLibName, const QString& subLibName = QString());

	bool operator ==(const TikzLibraryId &other) const;
	QString mainName() const {return this->m_mainName;}
	QString subName() const {return this->m_subName;}
	QString mainKey() const {return this->m_mainKey;}
	QString subKey() const {return this->m_subKey;}

	QString fullKey() const{return this->m_fullKey;}

	static TikzLibraryId fromFullKey(const QString& key);

private:
	QString m_mainName;
	QString m_subName;
	QString m_mainKey;
	QString m_subKey;
	QString m_fullKey;
};
typedef QSet<TikzLibraryId> TikzLibraryDependencies;

inline uint qHash(const TikzLibraryId &key, uint seed)
{
	return qHash(key.mainKey(), seed) ^ qHash(key.subKey());
}
inline uint qHash(const TikzLibraryId &key)
{
	return qHash(key.mainKey()) ^ qHash(key.subKey());
}
uint qHash(const QSet<TikzLibraryId> &key);
uint qHash(const QSet<TikzLibraryId> &key, uint seed);



#endif // TIKZLIBRARYID_H
