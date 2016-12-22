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

#include "tikzcommandmanager.h"
#include "tikzcommandreaders.h"



#include <QStringBuilder>
#include <QFileInfo>
#include <QVector>

static const QLatin1String tcfeXml = QLatin1String(".xml");
static const QLatin1String tcfeJson = QLatin1String(".json") ;




static QString removeExtension(const QString& fileName, const QString& extWithDot)
{
	if (fileName.endsWith(extWithDot, Qt::CaseInsensitive))
	{
		QString c(fileName);
		c.chop(extWithDot.length());
		return c;
	}
	return fileName;
}



static TikzCommandManager::TikzCommandFile SetBuiltInTikzCommandFile(
		//TikzCommandManager::TikzCommandFile& target,
		// QVector<>& targetList,
		QString baseFilePath,
		bool isJson)
{

	const QLatin1String fileKeyPrefix("built-in");
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QString jsonPath = baseFilePath.append(QLatin1String(".json"));
	TikzCommandManager::TikzCommandFile tcf(
				QString::null,
				jsonPath,
				fileKeyPrefix,
				true);
#else
	QString xmlPath = baseFilePath.append(QLatin1String(".xml"));
	TikzCommandManager::TikzCommandFile tcf(
				 //QString::null,
				xmlPath,
				fileKeyPrefix);
	Q_UNUSED(isJson);
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

	return tcf;
}


TikzCommandManager TikzCommandManager::m_global (
		QExplicitlySharedDataPointer<TikzCommandManager::PrivateData>(new TikzCommandManager::PrivateData()));
TikzCommandManager::TikzCommandManager(TikzCommandManager::PrivateData *d):
	m_data(d)
{}
TikzCommandManager::TikzCommandManager(const QExplicitlySharedDataPointer<PrivateData> &d):
	m_data(d)
{}

TikzCommandManager(d);

TikzCommandManager::TikzCommandManager() :
	m_data(TikzCommandManager::m_global.m_data)
{

}

TikzCommandManager TikzCommandManager::global()
{
	return m_global;
}

void TikzCommandManager::pushRequest(TikzCommandManager::Request *request)
{
	m_data->pushRequest(request);
}

QVector<TikzCommandManager::TikzCommandFile> TikzCommandManager::enumTikzCommandFiles(bool forceRefresh)
{
	static QVector<TikzCommandFile> files;

	if(files.count() == 0 || forceRefresh)
	{
		files.clear();

		files << SetBuiltInTikzCommandFile(QString::fromLatin1(":/tikzcommands"), true);
		files << SetBuiltInTikzCommandFile(QString::fromLatin1(":/tikz.external"), true);
	}
	return files;
}




TikzCommandManager::TikzCommandFile::TikzCommandFile(
		const QString &xmlFilePath,
		#if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		const QString &jsonFilePath,
		#endif
		QString fileKeyPrefix
		#if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		, const bool useJson
		#endif
		)
{
	this->m_xmlFilePath = xmlFilePath;
#if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	file.m_jsonFilePath = jsonFilePath;
	const QString filePath (useJson ? jsonFilePath : xmlFilePath);
	this->m_useJson = useJson;
#else
	const QString& filePath = xmlFilePath ;
	const bool useJson = false;
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

//	QFileInfo info (filePath);

	this->m_fileKey =
			fileKeyPrefix % QChar::fromLatin1(':') %
			removeExtension(
				QFileInfo(filePath).fileName(),
				useJson ? tcfeJson : tcfeXml
				);

}


TikzCommandFileContent TikzCommandManager::TikzCommandFile::load() const
{
	#if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	return  m_isJson ? : ;
	#else
	return TikzCommandReaders::fromXml(this->m_xmlFilePath);
	#endif
}

TikzCommandManager::TikzCommandFile TikzCommandManager::TikzCommandFile::fromBasePathName(
		const QString &basePathName, const QString &fileKeyPrefix)
{
	QString xmlp (basePathName);
	xmlp.append	(tcfeXml);
#if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QString jsonp (basePathName);
	jsonp.append	(tcfeJson);

	QFileInfo fix(xmlp);
	QFileInfo fij(xmlj);

	bool ex = fix.exists();
	bool ej = fix.exists();

	if (ex && ej) // both files exist
	{
		bool useJson;
		//bool warning;

		QDateTime dtx = fix.lastModified();
		QDateTime dtj = fij.lastModified();

		useJson =  dtj >= dtx;

		TikzCommandManager::TikzCommandFile tcf(xmlp, xmlj, fileKeyPrefix, useJson);
		if (!useJson)
		{
			tcf.m_Warning = tr("The JSON file is older than XML one. The XML file has been read instead (This takes a little more time).");
		}
		return tcf;
	}
	else if (ex) // only xml file exist
	{
		TikzCommandManager::TikzCommandFile tcf(xmlp, QString::null, fileKeyPrefix, false);
	}
	else if(ej) // only json file exist
	{
		TikzCommandManager::TikzCommandFile tcf(QString::null, xmlj, fileKeyPrefix, true);
	}
	else
	{
		qWarning("The file %s has been declared, but it does not exist.", xmlp.toLatin1().data());
		return TikzCommandManager::TikzCommandFile();
	}

#else
	return TikzCommandManager::TikzCommandFile(xmlp, fileKeyPrefix);
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

}



void TikzCommandManager::PrivateData::fetch()
{

	Q_ASSERT(this->m_files.size() == 0);

	// Get alls files
	if( m_files.empty()){
		QVector<TikzCommandManager::TikzCommandFile> files(
					TikzCommandManager::enumTikzCommandFiles(false));

		Q_FOREACH(const TikzCommandManager::TikzCommandFile& file, files)
		{
			this->m_files.append(files_t::value_type(
						file,
						file.load()
						));
		}
	}

	RequestParams p;
	Q_FOREVER
	{
		QMutexLocker lock(&m_requestMutex);

		if (m_pendingRequests.empty())
			return;
		else
		{
			p =  m_pendingRequests.constBegin().key();
		}
		lock.unlock();


		Result res(TikzCommandManager(this),  combine(p) );



		lock.relock();
		QList<requests_t::mapped_type> l = m_pendingRequests.values(p);
		m_pendingRequests.remove(p);
		lock.unlock();

		for (int i = 0 ; i < l.size(); ++i) {
			Request* pi = l[i];
			pi->emitFinished(res);
			delete pi;
		}
	}



}

TikzCommandManager::PrivateData::PrivateData():
	m_requestMutex()
{
}

void TikzCommandManager::PrivateData::pushRequest(TikzCommandManager::Request *request)
{
	QMutexLocker lock(&m_requestMutex);
	//request->setParent(0);
	m_pendingRequests.insertMulti(request->params(), request);
	if(m_pendingRequests.size()==1)
		QThread::start(QThread::LowPriority);
}

void TikzCommandManager::PrivateData::run()
{
	fetch();
}


/// Combine two TikzCommandList
/// @todo Improve the combinaison to allow merging between submenus.
static void combineSubTikzCommandList( TikzCommandList* dest, const TikzCommandList* src )
{
   dest->commands.append(src->commands);
   dest->children.append(src->children);
}
static void increaseTikzCommandIndex( TikzCommandList& tcl, int offset, int startAtChildren = 0 , int startAtCommand = 0 )
{
	typedef QList<TikzCommandList>::iterator N;
	typedef QList<TikzCommand>::iterator L;
	for ( N i = tcl.children.begin()+startAtChildren; i != tcl.children.end(); ++i)
	{
		increaseTikzCommandIndex(*i, offset);
	}
	for ( L i = tcl.commands.begin()+startAtCommand; i != tcl.commands.end(); ++i)
	{
		  i->number += offset;
	}
}


TikzCommandFileContent TikzCommandManager::PrivateData::combine(TikzCommandManager::RequestParams &params) const
{
	if(Q_UNLIKELY(this->m_files.empty()))
		return TikzCommandFileContent();


	TikzCommandFileContent base = (*m_files.constBegin()).second;

	for (files_t::ConstIterator i = ++m_files.constBegin()
		 ; i != m_files.constEnd(); ++i) {

		int childrenOldSize = base.commandList()->children.size();
		int commandOldSize = base.commandList()->commands.size();
		int flatCommandOldSize = base.flatCommands()->size();
		base.flatCommands()->append(*i->second.flatCommands());
		combineSubTikzCommandList(base.commandList(), i->second.commandList());
		increaseTikzCommandIndex(*base.commandList(), flatCommandOldSize, childrenOldSize,  commandOldSize);
	}

	return base;
}


void TikzCommandManagerRequest::emitFinished(const TikzCommandManagerResult &res)
{
	//Q_EMIT finished(res);
	m_destMethod.invoke(
				m_destReceiver.data(),
				Qt::DirectConnection,
				Q_ARG(const TikzCommandManager::Result &, res));
}

TikzCommandManagerRequest::TikzCommandManagerRequest(const TikzCommandManager::RequestParams &params,
		QObject *receiver,
		const char *slotName) :
	//QObject(parent),
	m_params(params),
	m_destReceiver(receiver)
{
   const QMetaObject* mm (receiver->metaObject());
   int i = mm->indexOfSlot(slotName);
   m_destMethod = mm->method(i);
}

TikzCommandManagerResult::TikzCommandManagerResult() :
	manager(),
	tikzCommands()
{}

TikzCommandManagerResult::TikzCommandManagerResult(const TikzCommandManager &manager, const TikzCommandFileContent &tikzCommands):
	manager(manager),
	tikzCommands(tikzCommands)
{}
