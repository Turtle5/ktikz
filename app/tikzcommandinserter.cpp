/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014          *
 *     by Glad Deschrijver <glad.deschrijver@gmail.com>                    *
 *   Copyright (C) 2016 by G. Prudhomme                                    *
 *     <gprud@users.noreply.github.com>                                    *
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

#include "tikzcommandinserter.h"

#include <QtGui/QTextCursor>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QApplication>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolTip>
#else
#include <QtGui/QApplication>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMenu>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolTip>
#endif

#include "tikzeditorhighlighter.h"
#include "tikzcommandwidget.h"
#include "../common/utils/combobox.h"
#include "tikzcommand.h"
#include "tikzcommandmanager.h"
//#include "tikzcommandreaders.h"

static const QString s_completionPlaceHolder(0x2022);

TikzCommandInserter::TikzCommandInserter(QWidget *parent)
	: QObject(parent)
	, m_tikzCommands()
	, m_mainEdit(0)
	, m_commandsCombo(0)
	, m_commandsStack(0)
{
}

/// @remarks TODO: set params (for filtering TikzCommand, for example)
///
void TikzCommandInserter::loadCommands()
{
	TikzCommandManager::RequestParams params;
	// TODO: set params


	TikzCommandManager::Request* request = new TikzCommandManager::Request(
				 params,
				 this,
				 "internalTikzCommandFechted(TikzCommandManagerResult)"
				 );

	m_tikzCommands.manager.pushRequest(
				request
				);
}

/***************************************************************************/

static QString removeOptionsAndSpecialCharacters(const QString &text)
{
	bool isInOption = false;
	for (int i = 0; i < text.length(); ++i)
	{
		if (isInOption)
			continue;
		QChar::Category cat = text.at(i).category();
		if (cat == QChar::Letter_Lowercase || cat == QChar::Letter_Uppercase || text.at(i) == QLatin1Char('\\'))
			return text.mid(i);
		if (text.at(i) == QLatin1Char('<'))
		{
			isInOption = true;
			continue;
		}
		else if (text.at(i) == QLatin1Char('>'))
		{
			isInOption = false;
			continue;
		}
	}
	return QString();
}

QStringList TikzCommandInserter::getCommandWords()
{
	QStringList words;

//	QRegExp rx1(QLatin1String("^([^a-z\\\\<>]*<[^>]*>)*"));
//	QRegExp rx2(QLatin1String("^[^a-z\\\\]*"));
//	QString allowedLetters = QLatin1String("abcdefghijklmnopqrstuvwxyz\\");
	QList<TikzCommand> tikzCommandsList(*tikzCommandFlatList());
	for (int i = 0; i < tikzCommandsList.size(); ++i)
	{
		const TikzCommand& tikzCommand = tikzCommandsList[i];
		QString word = tikzCommand.description;
		// remove all special characters and <options> at the beginning of the word
/*
		if (!word.isEmpty() && !allowedLetters.contains(word.at(0))) // minimize the number of uses of QRegExp
		{
			word.remove(rx1);
			word.remove(rx2);
		}
		if (!word.isEmpty())
			words.append(word);
		else
		{
			word = m_tikzCommandsList.at(i).command;
			// remove all special characters and <options> at the beginning of the word
			if (!word.isEmpty() && !allowedLetters.contains(word.at(0))) // minimize the number of uses of QRegExp
			{
				word.remove(rx1);
				word.remove(rx2);
			}
			if (!word.isEmpty())
				words.append(word);
		}
*/
		if (word.isEmpty())
			word = tikzCommand.command;
		word = removeOptionsAndSpecialCharacters(word);
		if (!word.isEmpty())
			words.append(word);
	}

	return words;
}

/*!
 * \name Menu with TikZ commands
 */
//@{

void TikzCommandInserter::updateDescriptionToolTip()
{
	QAction *action = qobject_cast<QAction*>(sender());
	if (action)
	{
		const int num = action->data().toInt();
		const TikzCommand cmd = tikzCommandFlatList()->at(num);
		QString description = cmd.description;
		description.replace(QLatin1Char('&'), QLatin1String("&amp;"));
		description.replace(QLatin1Char('<'), QLatin1String("&lt;"));
		description.replace(QLatin1Char('>'), QLatin1String("&gt;"));
		QMenu *menu = qobject_cast<QMenu*>(action->parentWidget());
		const QRect rect = menu->actionGeometry(action);
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
//		QToolTip::showText(menu->mapToGlobal(rect.topRight()), QLatin1String("<p>") + description + QLatin1String("</p>"), menu, rect, 5000);
		QToolTip::showText(menu->mapToGlobal(QPoint(rect.x() + rect.width(), rect.y() - rect.height() / 2)), QLatin1String("<p>") + description + QLatin1String("</p>"), menu, rect, 5000);
#else
//		QToolTip::showText(menu->mapToGlobal(rect.topRight()), QLatin1String("<p>") + description + QLatin1String("</p>"), menu, rect);
		QToolTip::showText(menu->mapToGlobal(QPoint(rect.x() + rect.width(), rect.y() - rect.height() / 2)), QLatin1String("<p>") + description + QLatin1String("</p>"), menu, rect);
#endif
	}
}

QMenu *TikzCommandInserter::getMenu(const TikzCommandList &commandList, QWidget *parent)
{
	QMenu *menu = new QMenu(commandList.title, parent);
	const int numOfCommands = commandList.commands.size();
	int whichSection = 0;

	for (int i = 0; i < numOfCommands; ++i)
	{
		const QString name = commandList.commands.at(i).name;
		//if (name.isEmpty()) // add separator or submenu
		//{
		if (commandList.commands.at(i).type == TikzCommand::Special_Separator)
		{
			Q_ASSERT(name.isEmpty());
			menu->addSeparator();
		}
		else if  (commandList.commands.at(i).type == TikzCommand::Special_SubMenu)
		// type == -1, so add submenu; this assumes that the i-th command with type == -1
		// corresponds with the i-th submenu (see getCommands())
		{
			Q_ASSERT(name.isEmpty());
			menu->addMenu(getMenu(commandList.children.at(whichSection), parent));
			++whichSection;
		}
		// }
		else // add command
		{
			QAction *action = menu->addAction(name);
			action->setData(commandList.commands.at(i).number); // link to the corresponding item in m_tikzCommandsList
			action->setStatusTip(commandList.commands.at(i).description);
			connect(action, SIGNAL(triggered()), this, SLOT(insertTag()));
			connect(action, SIGNAL(hovered()), this, SLOT(updateDescriptionToolTip()));
		}
	}

	return menu;
}

/*!
 * This function returns a menu containing a list of TikZ commands
 * which can be inserted in the main text.
 * \return a menu with TikZ commands
 */

QMenu *TikzCommandInserter::getMenu()
{
	return getMenu(*tikzCommandNodes(), qobject_cast<QWidget*>(parent()));
}

//@}

/*!
 * \name Dock widget with TikZ commands
 */
//@{

void TikzCommandInserter::addListWidgetItems(QListWidget *listWidget, const QPalette &standardPalette, const TikzCommandList &commandList, bool addChildren)
{
	QFont titleFont = qApp->font();
	titleFont.setBold(true);
//	QColor titleBg(standardPalette.color(QPalette::Normal, QPalette::Highlight));
//	titleBg = titleBg.lighter(120);
	QColor titleBg(standardPalette.color(QPalette::Normal, QPalette::Window));
	titleBg = titleBg.darker(200);
	QColor titleFg(standardPalette.color(QPalette::Normal, QPalette::HighlightedText));
//	QColor separatorBg(standardPalette.color(QPalette::Normal, QPalette::AlternateBase));
//	if (separatorBg == standardPalette.color(QPalette::Normal, QPalette::Base))
//		separatorBg = separatorBg.darker(110);

	for (int i = 0; i < commandList.commands.size(); ++i)
	{
		if (commandList.commands.at(i).type == -1) // if we have an empty command corresponding to a submenu, then don't add the command, the submenus will be added later
			continue;
		QString itemText = commandList.commands.at(i).name;
		if (itemText.isEmpty())
			continue;

		QListWidgetItem *item = new QListWidgetItem(listWidget);
		item->setText(itemText.remove(QLatin1Char('&')));

//		if (itemText.isEmpty())
//			item->setBackgroundColor(separatorBg);
//		else
			item->setData(Qt::UserRole, commandList.commands.at(i).number); // link to the corresponding item in m_tikzCommandsList
	}

	if (!addChildren) return;

	for (int i = 0; i < commandList.children.size(); ++i)
	{
		QListWidgetItem *item = new QListWidgetItem(listWidget);
		QString itemText = commandList.children.at(i).title;
		item->setText(itemText.remove(QLatin1Char('&')));

		item->setBackgroundColor(titleBg);
		item->setTextColor(titleFg);
		item->setFont(titleFont);

		addListWidgetItems(listWidget, standardPalette, commandList.children.at(i));
	}
}

void TikzCommandInserter::showItemsInDockWidget()
{
	Q_ASSERT_X(m_commandsCombo, "TikzCommandInserter::showItemsInDockWidget()", "TikzCommandInserter::getDockWidget(QWidget *parent) should be run before using this function");
	Q_ASSERT_X(m_commandsStack, "TikzCommandInserter::showItemsInDockWidget()", "TikzCommandInserter::getDockWidget(QWidget *parent) should be run before using this function");
	QListWidget *tikzListWidget = qobject_cast<QListWidget*>(m_commandsStack->widget(0));
	QPalette standardPalette = QApplication::style()->standardPalette(); // this is slow, so we call this only once here and pass this as argument to addListWidgetItems instead of calling this each time in addListWidgetItems
	addListWidgetItems(tikzListWidget, standardPalette, *tikzCommandNodes(), false); // don't add children

	TikzCommandList m_tikzSections = *tikzCommandNodes();
	for (int i = 0; i < m_tikzSections.children.size(); ++i)
	{
		QListWidget *tikzListWidget = new QListWidget;
		addListWidgetItems(tikzListWidget, standardPalette, m_tikzSections.children.at(i));
		tikzListWidget->setMouseTracking(true);
		connect(tikzListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(setListStatusTip(QListWidgetItem*)));
		connect(tikzListWidget, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(setListStatusTip(QListWidgetItem*)));
		connect(tikzListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(insertTag(QListWidgetItem*)));
//		connect(tikzListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(insertTag(QListWidgetItem*)));

		QString comboItemText = m_tikzSections.children.at(i).title;
		m_commandsCombo->addItem(comboItemText.remove(QLatin1Char('&')));
		m_commandsStack->addWidget(tikzListWidget);
	}
}

/*!
 * This function returns a dock widget containing a list of TikZ commands
 * which can be inserted in the main text by clicking on them.
 * \param parent the parent widget
 * \return a dock widget with TikZ commands
 */

QDockWidget *TikzCommandInserter::getDockWidget(QWidget *parent)
{
	QDockWidget *tikzDock = new QDockWidget(parent);
	tikzDock->setObjectName(QLatin1String("CommandsDock"));
	tikzDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	tikzDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
	tikzDock->setWindowTitle(tikzCommandNodes()->title);
	tikzDock->setWhatsThis(tr("<p>This is a list of TikZ "
	                          "commands.  You can insert these commands in your code by "
	                          "clicking on them.  You can obtain more commands by "
	                          "changing the category in the combo box.</p>"));

	QAction *focusTikzDockAction = new QAction(parent);
	focusTikzDockAction->setShortcut(QKeySequence(tr("Alt+I")));
	tikzDock->addAction(focusTikzDockAction);
	connect(focusTikzDockAction, SIGNAL(triggered()), tikzDock, SLOT(setFocus()));

	QLabel *commandsComboLabel = new QLabel(tr("Category:"));
	m_commandsCombo = new ComboBox;
	m_commandsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	m_commandsStack = new QStackedWidget;
	connect(m_commandsCombo, SIGNAL(currentIndexChanged(int)), m_commandsStack, SLOT(setCurrentIndex(int)));

	QListWidget *tikzListWidget = new QListWidget;
	tikzListWidget->setMouseTracking(true);
	connect(tikzListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(setListStatusTip(QListWidgetItem*)));
	connect(tikzListWidget, SIGNAL(itemEntered(QListWidgetItem*)), this, SLOT(setListStatusTip(QListWidgetItem*)));
	connect(tikzListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(insertTag(QListWidgetItem*)));
//	connect(tikzListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(insertTag(QListWidgetItem*)));
	m_commandsCombo->addItem(tr("General"));
	m_commandsStack->addWidget(tikzListWidget);

	QGridLayout *tikzLayout = new QGridLayout;
	tikzLayout->addWidget(commandsComboLabel, 0, 0);
	tikzLayout->addWidget(m_commandsCombo, 0, 1);
	tikzLayout->addWidget(m_commandsStack, 1, 0, 1, 2);
	tikzLayout->setMargin(5);

	TikzCommandWidget *tikzWidget = new TikzCommandWidget;
	tikzWidget->setLayout(tikzLayout);
	tikzDock->setWidget(tikzWidget);
	tikzDock->setFocusProxy(m_commandsCombo);

	return tikzDock;
}

void TikzCommandInserter::setListStatusTip(QListWidgetItem *item)
{
	if (item && !item->font().bold() && !item->text().isEmpty())
	{
		const int num = item->data(Qt::UserRole).toInt();
		Q_EMIT showStatusMessage(tikzCommandFlatList()->at(num).description, 10000);
	}
	else
		Q_EMIT showStatusMessage(QString());
}

//@}

/*!
 * Returns a list of default formats which will be used by TikzHighlighter
 * to highlight the different types of highlighting rules given by
 * getHighlightTypeNames().  Any value in the list returned by
 * getHighlightTypeNames() serves as a key in this list.
 * \return a list of default formats
 */

QMap<QString, QTextCharFormat> TikzCommandInserter::getDefaultHighlightFormats()
{
	QMap<QString, QTextCharFormat> formatList;
	QStringList highlightTypeNames = getHighlightTypeNames();

	QTextCharFormat commandFormat;
	commandFormat.setForeground(QColor(QLatin1String("#004080")));
	commandFormat.setFont(qApp->font());
	commandFormat.setFontWeight(QFont::Bold);
	formatList[highlightTypeNames.at(0)] = commandFormat;

	QTextCharFormat drawFormat;
	drawFormat.setForeground(Qt::darkRed);
	drawFormat.setFont(qApp->font());
	drawFormat.setFontWeight(QFont::Normal);
	formatList[highlightTypeNames.at(1)] = drawFormat;

	QTextCharFormat optionFormat;
	optionFormat.setForeground(QColor(QLatin1String("#004000")));
	optionFormat.setFont(qApp->font());
	optionFormat.setFontWeight(QFont::Normal);
	formatList[highlightTypeNames.at(2)] = optionFormat;

	return formatList;
}

/*!
 * Returns the list of the names of the different types of highlighting
 * rules provided by this class.  This list contains the names which may
 * be presented in a user interface and which will be translated when
 * another language is used.
 * \return a list of types of highlighting rules
 * \see getHighlightTypeNames
 */

QStringList TikzCommandInserter::getTranslatedHighlightTypeNames()
{
	QStringList translatedHighlightTypeNames;
	translatedHighlightTypeNames << tr("Commands") << tr("Draw to") << tr("Options");
	return translatedHighlightTypeNames;
}

/*!
 * Returns the list of the names of the different types of highlighting
 * rules provided by this class.  This list contains the untranslated
 * versions of the type names and should not be used in the user interface.
 * \return a list of types of highlighting rules
 * \see getTranslatedHighlightTypeNames
 */
QStringList TikzCommandInserter::getHighlightTypeNames()
{
	QStringList highlightTypeNames;
	highlightTypeNames << QLatin1String("Commands") << QLatin1String("Draw to") << QLatin1String("Options");
	return highlightTypeNames;
}

/*!
 * Returns a vector with the highlighting rules derived from the TikZ
 * commands which are available in the menu/dock widget.  These rules
 * are used by TikzHighlighter to highlight the commands in the text.
 * \return a vector containing the highlighting rules
 */

QVector<HighlightingRule> TikzCommandInserter::getHighlightingRules()
{
	QVector<HighlightingRule> highlightingRules;
	HighlightingRule rule;
	QStringList highlightTypeNames = getHighlightTypeNames();

	QList<TikzCommand> tikzCommandsList(*tikzCommandFlatList());

	for (int i = 0; i < tikzCommandsList.size(); ++i)
	{
		const TikzCommand& tikzCommand = /*  m_tikzCommandsList.at(i) */
				tikzCommandsList[i] ; // We can use safely the operator[] because the list is localy store.
		QString command = tikzCommand.command.isEmpty() ?
					tikzCommand.description
				  : tikzCommand.command;
		const TikzCommand::TikzCommandType type = tikzCommand.type;
		int end;
		rule.isRegExp = false;
		if (!tikzCommand.highlightString.isEmpty())
		{
			rule.pattern = QRegExp(tikzCommand.highlightString);
			rule.isRegExp = true;
		}
		switch (type)
		{
			case TikzCommand::Command: //=1
			{
				const int end1 = command.indexOf(QLatin1Char(' '), 0);
				const int end2 = command.indexOf(QLatin1Char('['), 0);
				const int end3 = command.indexOf(QLatin1Char('{'), 0);
				end = end1;
				if (end < 0 || (end2 >= 0 && end2 < end))
					end = end2;
				if (end < 0 || (end3 >= 0 && end3 < end))
					end = end3;

				command = command.left(end);
//				command = command.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
				rule.type = highlightTypeNames.at(0);
//				rule.pattern = QRegExp(command);
//				rule.pattern.setPattern(command);
				rule.matchString = command;
				highlightingRules.append(rule);
				break;
			}
			case TikzCommand::DrawToNextPoint: //=2
//				command = command.replace("()", "\\([^\\)]*\\)");
//				command = command.replace("(,)", "\\([^\\)]*\\)");
//				command = command.replace("(:::)", "\\([^\\)]*\\)");
				command = command.remove(QLatin1Char('+'));
				command = command.remove(QLatin1String(" ()"));
				command = command.remove(QLatin1String(" (,)"));
				command = command.remove(QLatin1String(" (:::)"));
				command = command.remove(QLatin1String(" {} "));
				rule.type = highlightTypeNames.at(1);
//				rule.pattern = QRegExp(command);
//				rule.pattern.setPattern(command);
				rule.matchString = command;
				highlightingRules.append(rule);
				break;
			case TikzCommand::Option: //=3
//				command = command.replace(QLatin1Char('|'), QLatin1String("\\|"));
				end = command.indexOf(QLatin1Char('='), 0) + 1;
				if (end > 0)
					command = command.left(end);
				rule.type = highlightTypeNames.at(2);
//				rule.pattern = QRegExp(command);
//				rule.pattern.setPattern(command);
				rule.matchString = command;
				highlightingRules.append(rule);
				break;
		}
	}

	return highlightingRules;
}

void TikzCommandInserter::insertTag()
{
	QAction *action = qobject_cast<QAction*>(sender());
	if (action)
	{
		const int num = action->data().toInt();
		const TikzCommand cmd = tikzCommandFlatList()->at(num);
		Q_EMIT showStatusMessage(cmd.description, 0);
		const QString command = cmd.command.isEmpty() ? cmd.description : cmd.command;
		insertTag(command, cmd.dx, cmd.dy);
	}
}

void TikzCommandInserter::insertTag(QListWidgetItem *item)
{
	if (item && !item->font().bold() && !item->text().isEmpty())
	{
		const int num = item->data(Qt::UserRole).toInt();
		const TikzCommand& cmd = tikzCommandFlatList()->at(num);
		Q_EMIT showStatusMessage(cmd.description, 0);
		const QString command = cmd.command.isEmpty() ? cmd.description : cmd.command;
		insertTag(command, cmd.dx, cmd.dy);
	}
}

void TikzCommandInserter::internalTikzCommandFechted(const TikzCommandManagerResult &results)
{
	m_tikzCommands = results;
	Q_EMIT tikzCommandFechted(results);
	Q_EMIT highlightingRulesChanged();
}

void TikzCommandInserter::setEditor(QPlainTextEdit *textEdit)
{
	m_mainEdit = textEdit;
}

/*!
 * Inserts a TikZ command and moves the cursor to a position located
 * dx characters to the right of and dy characters below the
 * start of the inserted tag.
 * \param tag the TikZ command to be inserted
 * \param dx the x-offset of the new cursor position w.r.t. the beginning of the inserted tag
 * \param dy the y-offset of the new cursor position w.r.t. the beginning of the inserted tag
 */

void TikzCommandInserter::insertTag(const QString &tag, int dx, int dy)
{
	Q_ASSERT_X(m_mainEdit, "TikzCommandInserter::insertTag(const QString &tag, int dx, int dy)", "m_mainEdit should be set using TikzCommandInserter::setEditor() before using this function");

	QTextCursor cur = m_mainEdit->textCursor();
	const int pos = cur.position();

	// replace all options (between <...>) by a place holder
	QString insertWord = tag;
	const QRegExp rx(QLatin1String("<[^<>]*>"));
	insertWord.replace(rx, s_completionPlaceHolder);

	// insert tag
	m_mainEdit->insertPlainText(insertWord);
	cur.setPosition(pos, QTextCursor::MoveAnchor);

	// move the text cursor to the first option or to the specified place
	if (insertWord.contains(s_completionPlaceHolder))
	{
		cur = m_mainEdit->document()->find(s_completionPlaceHolder, cur);
		m_mainEdit->setTextCursor(cur);
	}
	else if (dx > 0 || dy > 0)
	{
		if (dy > 0)
		{
			cur.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, dy);
			cur.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor, 1);
		}
		if (dx > 0)
			cur.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, dx);
		m_mainEdit->setTextCursor(cur);
	}
	// else we are only inserting a string with no placeholders and no positioning, so the cursor must come at the end of the string (this is done automatically by Qt)

	m_mainEdit->setFocus();
}



