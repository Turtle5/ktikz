/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014				*
 *	 by Glad Deschrijver <glad.deschrijver@gmail.com>					*
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


#ifndef TIKZCOMMAND_H
#define TIKZCOMMAND_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "tikzlibrary.h"
#include <QSet>

struct TikzCommand
{
	/// Type of TikzCommand.
	/// This includes "special" UI types.
	/// Special types assumes name == "".
	enum TikzCommandType // : qint8
	{
		// 0: plain text
		//   1: command
		//   2: draw to next point
		//   3: option
		PlainText = 0,
		Command = 1,
		DrawToNextPoint = 2,
		Option = 3,
		/// A submenu.
		/// @remarks this assumes that the i-th command with type == -1
		/// corresponds with the i-th submenu (see getCommands())
		Special_SubMenu = -1,
		/// A separator
		Special_Separator = -2
	};

	QString name;
	QString description;
	QString command;
	QString highlightString;
	int dx;
	int dy;
	TikzCommandType type;
	int number;

	TikzLibraryDependencies dependencies;

	/// Check if value is acceptable and convert it to TikzCommandType
	static TikzCommand::TikzCommandType intToStandardCommandType(int value);


};

struct TikzCommandList
{
	QString title;
	QList<TikzCommand> commands;
	QList<TikzCommandList> children;
};

class TikzCommandFileContent
{
public:
	TikzCommandFileContent(){}
	/*TikzCommandFileContent(const TikzCommandList& tcl, const QList<TikzCommand>& flatList ):
		TikzCommandList(tcl),
		flatCommands(flatList)
	{}*/
	const TikzCommandList* commandList() const {return &m_commandList;}
	TikzCommandList* commandList() {return &m_commandList;}
	void setCommandList(const TikzCommandList &commandList){ m_commandList = commandList; }

	const QList<TikzCommand>* flatCommands() const {return &m_flatCommands;}
	QList<TikzCommand>* flatCommands() {return &m_flatCommands;}

private:
	QList<TikzCommand> m_flatCommands;
	TikzCommandList m_commandList;
};

#endif // TIKZCOMMAND_H
