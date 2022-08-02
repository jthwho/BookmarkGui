/*****************************************************************************
 * faviconfetch.h
 * May 21, 2015
 *
 *****************************************************************************/

#ifndef _FAVICONFETCH_H_
#define _FAVICONFETCH_H_

#include <QObject>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkReply;
class QUuid;
class QUrl;
class QIcon;

class FaviconFetch : public QObject {
        Q_OBJECT
        public:
                FaviconFetch(QObject *parent = NULL);

        public slots:
                void fetch(const QUuid &uuid, const QUrl &url);

        signals:
                void fetchDone(const QUuid &uuid, const QUrl &url, const QIcon &icon);

        private slots:
                void accessFinished(QNetworkReply *reply);
                void requestNextImage(const QUuid &uuid, const QUrl &url, QStringList imageList);

        private:
                QNetworkAccessManager           *_net;
};

#endif /* ifndef _FAVICONFETCH_H_ */


