/*****************************************************************************
 * faviconfetch.cpp
 * May 21, 2015
 *
 *****************************************************************************/

#include <QDebug>
#include <QUuid>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QIcon>
#include <QVariant>
#include <QStringList>
//#include <QWebPage>
//#include <QWebFrame>
//#include <QWebElement>

#include "faviconfetch.h"
#include "xml.h"

#define NETREQ_ATTR(d)          (QNetworkRequest::Attribute)(QNetworkRequest::User + (d))
#define ATTR_UUID               NETREQ_ATTR(1)
#define ATTR_URL                NETREQ_ATTR(2)
#define ATTR_STATE              NETREQ_ATTR(3)
#define ATTR_IMAGE_LIST         NETREQ_ATTR(4)
#define STATE_GET_HTML          1
#define STATE_GET_IMAGE         2

#define HEADER_USER_AGENT               "User-Agent"
#define HEADER_USER_AGENT_VALUE         "bookmark-sync/1.0"

FaviconFetch::FaviconFetch(QObject *p) :
        QObject(p)
{
        _net = new QNetworkAccessManager(this);
        connect(_net, SIGNAL(finished(QNetworkReply *)), this, SLOT(accessFinished(QNetworkReply *)));
}

void FaviconFetch::fetch(const QUuid &uuid, const QUrl &url) {
        QNetworkRequest r(url);
        r.setAttribute(ATTR_UUID, uuid.toString());
        r.setAttribute(ATTR_URL, url);
        r.setAttribute(ATTR_STATE, STATE_GET_HTML);
        r.setRawHeader(HEADER_USER_AGENT, HEADER_USER_AGENT_VALUE);
        _net->get(r);
        return;
}

void FaviconFetch::requestNextImage(const QUuid &uuid, const QUrl &url, QStringList imageList) {
        if(imageList.isEmpty()) {
                qDebug() << "NO FAVICON" << url;
                emit fetchDone(uuid, url, QIcon());
                return;
        }
        QUrl imageUrl = imageList.takeFirst();
        QNetworkRequest req(imageUrl);
        req.setAttribute(ATTR_UUID, uuid.toString());
        req.setAttribute(ATTR_URL, url);
        req.setAttribute(ATTR_STATE, STATE_GET_IMAGE);
        req.setAttribute(ATTR_IMAGE_LIST, imageList);
        req.setRawHeader(HEADER_USER_AGENT, HEADER_USER_AGENT_VALUE);
        _net->get(req);
        return;
}

void FaviconFetch::accessFinished(QNetworkReply *reply) {
        QNetworkRequest r = reply->request();
        QUuid uuid = QUuid::fromString(r.attribute(ATTR_UUID).toString());
        QUrl url = r.attribute(ATTR_URL).toUrl();
        int state = r.attribute(ATTR_STATE).toInt();
        QStringList imageList = r.attribute(ATTR_IMAGE_LIST).toStringList();

        if(reply->error() != QNetworkReply::NoError) {
                qDebug() << "GET ERROR" << reply->error() << url << reply->url();
                if(state == STATE_GET_HTML) {
                        imageList += reply->url().resolved(QUrl("/favicon.ico")).toString();
                        imageList += url.resolved(QUrl("/favicon.ico")).toString();
                        imageList.removeDuplicates();
                }
                requestNextImage(uuid, url, imageList);
                return;
        }
        QString redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        if(!redirect.isEmpty()) {
                QUrl redirectUrl = reply->url().resolved(redirect);
                qDebug() << "REDIRECT" << url << reply->url() << redirectUrl;
                QNetworkRequest req(redirectUrl);
                req.setAttribute(ATTR_UUID, uuid.toString());
                req.setAttribute(ATTR_URL, url);
                req.setAttribute(ATTR_STATE, state);
                req.setAttribute(ATTR_IMAGE_LIST, imageList);
                req.setRawHeader(HEADER_USER_AGENT, HEADER_USER_AGENT_VALUE);
                _net->get(req);
                return;
        }
	/*
        if(state == STATE_GET_HTML) {
                QWebPage page;
                page.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
                page.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
                page.settings()->setAttribute(QWebSettings::JavaEnabled, false);
                page.settings()->setAttribute(QWebSettings::PluginsEnabled, false);
                QWebFrame *frame = page.mainFrame();
                frame->setHtml(reply->readAll());

                QList<QWebElement> links;
                links += frame->documentElement().findAll("head > link[rel=\"icon\" type=\"image/png\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"icon\" type=\"image/ico\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"icon\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"shortcut icon\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"apple-touch-icon-precomposed\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"android-touch-icon\"]").toList();
                links += frame->documentElement().findAll("head > link[rel=\"apple-touch-icon\"]").toList();

                QStringList imageList;
                for(int i = 0; i < links.size(); i++) {
                        QString href = links.at(i).attribute("href");
                        imageList += reply->url().resolved(href).toString();
                }
                imageList += reply->url().resolved(QString("/favicon.ico")).toString();
                imageList += url.resolved(QString("/favicon.ico")).toString();
                imageList.removeDuplicates();
                qDebug() << "LIST" << imageList;
                requestNextImage(uuid, url, imageList);
                return;

        } else 
	*/
	if(state == STATE_GET_IMAGE) {
                QPixmap pixmap;
                if(pixmap.loadFromData(reply->readAll())) {
                        qDebug() << "SUCCESS" << url;
                        emit fetchDone(uuid, url, QIcon(pixmap));
                } else requestNextImage(uuid, url, imageList);
        }
        return;
}

