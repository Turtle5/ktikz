/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014                *
 *     by Glad Deschrijver <glad.deschrijver@gmail.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#ifndef TIKZCOMMANDINSERTER_H
#define TIKZCOMMANDINSERTER_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

class QDockWidget;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QPalette;
class QPlainTextEdit;
class QStackedWidget;
class QTextCharFormat;
class ComboBox;
class HighlightingRule;

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
	/// Check if value is acceptable and convert it to TikzCommandType
	static TikzCommandType intToStandardCommandType(int value);

	QString name;
	QString description;
	QString command;
	QString highlightString;
	int dx;
	int dy;
	TikzCommandType type;
	int number;
};

struct TikzCommandList
{
	QString title;
	QList<TikzCommand> commands;
	QList<TikzCommandList> children;
};

class TikzCommandInserter : public QObject
{
	Q_OBJECT

public:
	explicit TikzCommandInserter(QWidget *parent = 0);

	static void loadCommands();
	static QStringList getCommandWords();
	QMenu *getMenu();
	void showItemsInDockWidget();
	QDockWidget *getDockWidget(QWidget *parent = 0);
	static QMap<QString, QTextCharFormat> getDefaultHighlightFormats();
	static QStringList getTranslatedHighlightTypeNames();
	static QStringList getHighlightTypeNames();
	static QVector<HighlightingRule> getHighlightingRules();
	void setEditor(QPlainTextEdit *textEdit);

public Q_SLOTS:
	void insertTag(const QString &tag, int dx = 0, int dy = 0);

Q_SIGNALS:
	/*!
	 * This signal is emitted whenever a message should be shown in
	 * the statusbar.
	 * \param message the message to be shown in the status bar
	 * \param timeout the duration in milli-seconds (if non-zero) during which the message must be shown
	 */
	void showStatusMessage(const QString &message, int timeout = 0);

private Q_SLOTS:
	void updateDescriptionToolTip();
	void setListStatusTip(QListWidgetItem *item);
	void insertTag();
	void insertTag(QListWidgetItem *item);

private:
	QMenu *getMenu(const TikzCommandList &commandList, QWidget *parent);
	void addListWidgetItems(QListWidget *listWidget, const QPalette &standardPalette, const TikzCommandList &commandList, bool addChildren = true);

	QPlainTextEdit *m_mainEdit;
	static TikzCommandList m_tikzSections;
	static QList<TikzCommand> m_tikzCommandsList;

	ComboBox *m_commandsCombo;
	QStackedWidget *m_commandsStack;
};

#endif
