/***************************************************************************
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

#ifndef TIKZCOMMANDMANAGER_H
#define TIKZCOMMANDMANAGER_H

#include <QString>
#include <QPair>
#include <QHash>
#include <QExplicitlySharedDataPointer>
#include <QMetaMethod>
#include <QMultiMap>
#include <QMutex>
#include <QThread>
#include <QPointer>
#include "tikzcommand.h"

class TikzCommandManager;
//class TikzCommandManager::PrivateData;

class TikzCommandManagerRequest;
class TikzCommandManagerResult;


/// Manager for loading and filtering (one day?) the TikzCommandFile content.
/// @remarks This class is thread-safe data-shared. Can be copied without care.
class TikzCommandManager
{
public:
    /// This class will hold  request parameters for TikzCommandManager
    class RequestParams {
        public:
            /// @remarks: to be reimplemented when the class will grow.
            bool operator<(const RequestParams& other) const {return false;}
    };

    typedef TikzCommandManagerRequest Request;
    typedef TikzCommandManagerResult Result;

    class TikzCommandFile {
        // friend class TikzCommandManager;
    public:
        TikzCommandFile(){}
        TikzCommandFile(
                const QString& xmlFilePath,
                        #if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        const QString& jsonFilePath,
                        #endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        QString fileKeyPrefix
                        #if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
                        ,const bool useJson
                        #endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
      );
        TikzCommandFileContent load() const;
        bool operator <(const TikzCommandFile &other) const{ return this->m_fileKey < other.m_fileKey;}

        /// Create an instance from baseFilePath (for example, ":/tikzcommands")
        static TikzCommandFile fromBasePathName(const QString &basePathName, const QString &fileKeyPrefix);      

    private:
        QString m_fileKey;
        QString m_xmlFilePath;
        #if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QString m_jsonFilePath;
        #endif
        QString m_Warning;
        #if  QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        bool m_useJson;
        #endif

    };

    class PrivateData : public QSharedData, private QThread {
        //friend class TikzCommandManager;


        void fetch();
        /// Push request
        /// Warning: the caller losses the ownership of the request
    public:
        virtual ~PrivateData()
        {
            qDebug("A instance of TikzCommandManager::PrivateData is been destroyed. Have the TikzCommand files been updated?");
        }

        PrivateData();
        void pushRequest (Request* request);

    protected:
        virtual void run();
    private:
        typedef QList<QPair<TikzCommandManager::TikzCommandFile, TikzCommandFileContent> > files_t;
        files_t m_files;
        bool m_isMapReadey;
        typedef QMultiMap<RequestParams, Request*> requests_t;
        requests_t m_pendingRequests;
        //mutable QThread m_fetchingThread;
        mutable QMutex m_requestMutex;

        TikzCommandFileContent combine(RequestParams& params) const;
    };



    /// Start fetching files in separate threads
    static void startFetching();


private:
    typedef PrivateData private_data_t;
    QExplicitlySharedDataPointer<private_data_t> m_data;

    static TikzCommandManager m_global;

    TikzCommandManager(PrivateData* d);
    TikzCommandManager(const QExplicitlySharedDataPointer<PrivateData>& d);
public:
    ///
    TikzCommandManager();
    /// Global instance of the manager
    /// @remarks Warning: this instance changes when the manager is reload (for example, an update in the TikzCommand files).
    /// Clients should keep the instance of TikzCommandManager from TikzCommandManager::Result to keep the same set, and to preserve the associated data in memory.
    static TikzCommandManager global();


    /// Push request
    /// Warning: the caller losses the ownership of the request
    void pushRequest(Request* request);

    /// Enumerate all tikz command files
    static QVector<TikzCommandFile> enumTikzCommandFiles(bool forceRefresh=false);
};

/// This class will hold a result of a TikzCommandManager::Request
class TikzCommandManagerResult{
public:
    TikzCommandManagerResult();
    TikzCommandManagerResult(const TikzCommandManager& manager, const TikzCommandFileContent& tikzCommands);
    TikzCommandManager manager;
    TikzCommandFileContent tikzCommands;
};
Q_DECLARE_METATYPE(TikzCommandManagerResult)


/// This class will hold a request for TikzCommandManager
/// @remarks Error: Meta object features not supported for nested classes
class TikzCommandManagerRequest /*: public QObject*/ {
    //Q_OBJECT

    friend class TikzCommandManager::PrivateData;
private:
    TikzCommandManager::RequestParams m_params;
    const QPointer<QObject> m_destReceiver;
    //const char* m_slotName;
    QMetaMethod m_destMethod;
    void emitFinished(const TikzCommandManagerResult& res) ;
public:
    //_TikzCommandManagerRequest(const TikzCommandManager::RequestParams& params, QObject *parent = 0);
    TikzCommandManagerRequest(const TikzCommandManager::RequestParams& params, QObject* receiver, const char* slotName);
    const TikzCommandManager::RequestParams& params() const {return this->m_params;}
/*Q_SIGNALS:
    void finished(const TikzCommandManager::Result&);*/
};

#endif // TIKZCOMMANDMANAGER_H
