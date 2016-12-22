/***************************************************************************
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014		  *
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


#include "tikzcommandreaders.h"
#include "tikzcommand.h"
#include "tikzlibrary.h"
#include "tikzlibraryid.h"

#include <QtCore/QFile>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonValue>
#endif
#include <QtCore/QXmlStreamReader>
#include <QApplication>
#include <QSet>


class ReaderStackRoot
{
private:
	//QList<TikzCommand> m_flatTikzCommand;
	TikzCommandFileContent* m_fileContent;
	QSet<const TikzLibraryDependencies> m_flatDependencies;

public:

	ReaderStackRoot(TikzCommandFileContent* fc):
		m_fileContent(fc)
	{

	}

	void addCommand(const TikzCommand& cmd)
	{
		//m_flatTikzCommand.append(cmd);
		m_fileContent->flatCommands()->append(cmd);
	}
	size_t commandCount() const{ return m_fileContent->flatCommands()->size();}
	const TikzLibraryDependencies& registerDependencies(const TikzLibraryDependencies d)
	{
		 QSet<const TikzLibraryDependencies>::const_iterator f = m_flatDependencies.constFind(d);
		 if (f != m_flatDependencies.constEnd())
			 return *f;
		 else
		 {
			 return *m_flatDependencies.insert(d);
		 }
	}

};

/// Store the current state of properties of a TikzCommand file.
/// Manage all the herited properties of reader inside the section tree.
/// Used by reader methods.
class ReaderStack
{
private:
	ReaderStackRoot* m_root;
	TikzLibraryDependencies m_dependencies;

public:
	ReaderStack(ReaderStackRoot*  root):
		m_root(root)
	{
		Q_ASSERT(root);
	}
	ReaderStack(const ReaderStack& other) :
		m_root(other.m_root),
		m_dependencies(other.m_dependencies)
	{

	}
	~ReaderStack()
	{
		if (m_root)
			push();
	}
	void push()
	{
		m_root = 0;
	}


	void addCommand(const TikzCommand& cmd)
	{
		m_root->addCommand(cmd);
	}
	size_t commandCount() const{ return m_root->commandCount();}
	void pushDependency(const TikzLibraryId& id)
	{
		m_dependencies.insert(id);
		m_dependencies = m_root->registerDependencies(
						m_dependencies
					);
	}
	TikzLibraryDependencies dependencies() const{return m_dependencies;}
};

/***************************************************************************/

static TikzCommand newCommand(const QString &name,
		const QString &description, const QString &command,
		const QString &highlightString, int dx, int dy, TikzCommand::TikzCommandType type)
{
	// type:
	//   0: plain text
	//   1: command
	//   2: draw to next point
	//   3: option
	TikzCommand tikzCommand;
	tikzCommand.name = name;
	tikzCommand.description = description;
	tikzCommand.command = command;
	tikzCommand.highlightString = highlightString;
	tikzCommand.dx = dx;
	tikzCommand.dy = dy;
	tikzCommand.type = type;

	return tikzCommand;
}

static QString translateOptions(const QString &text)
{
	QString translatedText;
	for (int pos = 0, oldPos = 0; pos >= 0;)
	{
		oldPos = pos;
		pos = text.indexOf(QLatin1Char('<'), pos); // option is between < and >
		translatedText += text.midRef(oldPos, pos - oldPos + 1); // add text between the current option and the previous option; this also adds the end of the original string, except when there are no options
		if (pos >= 0)
		{
			oldPos = pos;
			pos = text.indexOf(QLatin1Char('>'), pos); // option is between < and >
			translatedText += QCoreApplication::translate("TikzCommandInserter", text.mid(oldPos+1, pos - oldPos - 1).toLatin1().data());
		}
	}
	if (!translatedText.isEmpty()) // when there are no options, translatedText is empty
		return translatedText;
	return text;
}

static QString restoreNewLines(const QString &text)
{
	QString newText = text;
	// replace all "\n" not preceded by a backslash (as in "\\node") by a newline character
	for (int pos = 0; ; ++pos)
	{
		pos = newText.indexOf(QLatin1String("\\n"), pos);
		if (pos < 0)
			break;
		if (pos == 0 || newText.at(pos-1) != QLatin1Char('\\'))
			newText.replace(pos, 2, QLatin1Char('\n'));
	}
	return newText;
}

static TikzCommandList getChildCommands(QXmlStreamReader *xml, ReaderStack rStack)
{
	TikzCommandList commandList;
	QList<TikzCommand> commands;

	commandList.title = QApplication::translate("TikzCommandInserter", xml->attributes().value(QLatin1String("title")).toString().toLatin1().data());

	while (xml->readNextStartElement())
	{
		if (xml->name() == QLatin1String("depend"))
		{
			QXmlStreamAttributes xmlAttributes = xml->attributes();
			QString fullKey(xmlAttributes.value(QLatin1String("on")).toString());
			TikzLibraryId id = TikzLibraryId::fromFullKey(fullKey);
			rStack.pushDependency(id);

		}
		if (xml->name() == QLatin1String("item"))
		{
			QXmlStreamAttributes xmlAttributes = xml->attributes();
			QString name = QApplication::translate("TikzCommandInserter", xmlAttributes.value(QLatin1String("name")).toString().toLatin1().data());
			QString description = xmlAttributes.value(QLatin1String("description")).toString();
			QString insertion = xmlAttributes.value(QLatin1String("insert")).toString();
			QString highlightString = xmlAttributes.value(QLatin1String("highlight")).toString();
			QString type = xmlAttributes.value(QLatin1String("type")).toString();

			// currently description contains no newlines, otherwise add code to replace all "\n" not preceded by a backslash (as in "\\node") by a newline character
			description.replace(QLatin1String("\\\\"), QLatin1String("\\"));
			description = translateOptions(description);

			insertion = restoreNewLines(insertion); // this must be done before the next line
			insertion.replace(QLatin1String("\\\\"), QLatin1String("\\"));

			if (description.isEmpty()) // if both name and description are empty, setting the description first ensures that name is also set to insertion
				description = insertion;
			if (name.isEmpty())
			{
				name = description;
				description.remove(QLatin1Char('&')); // we assume that if name.isEmpty() then an accelerator is defined in description
			}
			if (type.isEmpty())
				type = QLatin1Char('0');

			TikzCommand tikzCommand = newCommand(name, description, insertion, highlightString, xmlAttributes.value(QLatin1String("dx")).toString().toInt(), xmlAttributes.value(QLatin1String("dy")).toString().toInt(),
												 TikzCommand::intToStandardCommandType(type.toInt())
												 );
			tikzCommand.number = rStack.commandCount();
			tikzCommand.dependencies = rStack.dependencies();
			commands << tikzCommand;
			rStack.addCommand(tikzCommand);
			xml->skipCurrentElement(); // allow to read the next start element on the same level: this skips reading the current end element which would cause xml.readNextStartElement() to evaluate to false
		}
		else if (xml->name() == QLatin1String("separator"))
		{
			commands << newCommand(QString(), QString(), QString(), QString(), 0, 0, TikzCommand::Special_Separator);
			xml->skipCurrentElement(); // same as above
		}
		else if (xml->name() == QLatin1String("section"))
		{
			commands << newCommand(QString(), QString(), QString(), QString(), 0, 0, TikzCommand::Special_SubMenu); // the i-th command with type == -1 corresponds to the i-th submenu (assumed in getMenu())
			commandList.children << getChildCommands(xml, rStack);
		}
		else
			xml->skipCurrentElement();
	}
	commandList.commands = commands;

	return commandList;
}



static TikzCommandList getCommands(QXmlStreamReader *xml, ReaderStack rStack )
{
	TikzCommandList commandList;

	if (xml->readNextStartElement())
	{
		if (xml->name() == QLatin1String("tikzcommands"))
			commandList = getChildCommands(xml, rStack);
		else
			xml->raiseError(QApplication::translate("TikzCommandInserter", "Cannot parse the TikZ commands file."));
	}
	if (xml->error()) // this should never happen in a final release because tikzcommands.xml is built in the binary
		qCritical("Parse error in TikZ commands file at line %d, column %d:\n%s",
				  int(xml->lineNumber()),
				  int(xml->columnNumber()),
				  qPrintable(xml->errorString()));
	return commandList;
}
TikzCommandList TikzCommandReaders::fromXml(const QString& filePath, QList<TikzCommand> *tikzCommandsList)
{
	TikzCommandFileContent tcfc = fromXml(filePath);

	*tikzCommandsList = *tcfc.flatCommands();
	return *tcfc.commandList();
}


TikzCommandFileContent TikzCommandReaders::fromXml(const QString &filePath)
{

	TikzCommandFileContent tcfc;

	QFile tagsFile(filePath);
	if (!tagsFile.open(QFile::ReadOnly))
		return tcfc;

	ReaderStackRoot root(&tcfc);
	QXmlStreamReader xml(&tagsFile);

	tcfc.setCommandList(
				getCommands(&xml, ReaderStack(&root))
				);


	return tcfc;
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
TikzCommandList loadChildCommandsJson(QJsonObject sectionObject, QList<TikzCommand> *tikzCommandsList)
{
	TikzCommandList commandList;
	QList<TikzCommand> commands;

	if (sectionObject.contains(QLatin1String("title")))
		commandList.title = QCoreApplication::translate("TikzCommandInserter", sectionObject.value(QLatin1String("title")).toString().toLatin1().data());

	if (sectionObject.contains(QLatin1String("commands")))
	{
		QJsonValue commandsArrayObject = sectionObject.value(QLatin1String("commands"));
		if (commandsArrayObject.isArray())
		{
			QJsonArray commandsArray = commandsArrayObject.toArray();
			for (int i = 0; i < commandsArray.size(); ++i)
			{
				if (!commandsArray.at(i).isObject())
					continue;
				QJsonObject commandObject = commandsArray.at(i).toObject();
				const int type = commandObject.value(QLatin1String("type")).toInt();
				if (commandObject.contains(QLatin1String("commands")))
				{
					commands << newCommand(QString(), QString(), QString(), QString(), 0, 0, TikzCommand::Special_SubMenu); // the i-th command with type == -1 corresponds to the i-th submenu (assumed in getMenu())
					commandList.children << loadChildCommandsJson(commandObject, tikzCommandsList);
				}
				else if (type == -1)
				{
					commands << newCommand(QString(), QString(), QString(), QString(), 0, 0, TikzCommand::Special_Separator);
				}
				else
				{
					QString name = QCoreApplication::translate("TikzCommandInserter", commandObject.value(QLatin1String("name")).toString().toLatin1().data());
					QString description = commandObject.value(QLatin1String("description")).toString();
					QString insertion = commandObject.value(QLatin1String("insert")).toString();
					QString highlightString = commandObject.value(QLatin1String("highlight")).toString();

					// currently description contains no newlines, otherwise add code to replace all "\n" not preceded by a backslash (as in "\\node") by a newline character
					description.replace(QLatin1String("\\\\"), QLatin1String("\\"));
					description = translateOptions(description);

					insertion = restoreNewLines(insertion); // this must be done before the next line
					insertion.replace(QLatin1String("\\\\"), QLatin1String("\\"));
//					insertion.replace(QLatin1String("&#8226;"), QString(0x2022));

					if (description.isEmpty()) // if both name and description are empty, setting the description first ensures that name is also set to insertion
						description = insertion;
					if (name.isEmpty())
					{
						name = description;
						description.remove(QLatin1Char('&')); // we assume that if name.isEmpty() then an accelerator is defined in description
					}

					TikzCommand tikzCommand = newCommand(name, description, insertion, highlightString, commandObject.value(QLatin1String("dx")).toInt(), commandObject.value(QLatin1String("dy")).toInt(), type);
					tikzCommand.number = tikzCommandsList->size();
					tikzCommandsList->append(tikzCommand);
					commands << tikzCommand;
				}
			}
		}
	}
	commandList.commands = commands;

	return commandList;
}

static TikzCommandList TikzCommandReaders::fromJson(const QString &fileName, QList<TikzCommand> *tikzCommandsList)
{
	TikzCommandList commandList;

	QFile commandsFile(fileName);
	if (!commandsFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return commandList;

	QJsonParseError error;
	QJsonDocument commandsDocument = QJsonDocument::fromJson(commandsFile.readAll(), &error);
	if (error.error != QJsonParseError::NoError)
	{
		qCritical("Parse error in TikZ commands file %s at offset %d:\n%s", qPrintable(fileName), error.offset, qPrintable(error.errorString()));
		return commandList;
	}
	if (commandsDocument.isObject())
	{
		QJsonObject sectionObject = commandsDocument.object();
		commandList = loadChildCommandsJson(sectionObject, tikzCommandsList);
	}
	return commandList;
}
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)



