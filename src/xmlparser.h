/*******************************************************************************
 * xmlparser.h
 ******************************************************************************/

#ifndef _XMLPARSER_H_
#define _XMLPARSER_H_

#include <QString>
#include <QByteArray>

class XMLParser {
	public:
		virtual ~XMLParser();
		bool parse(const QString &s);
		bool parse(const QByteArray &s);

		// Parser callbacks
		virtual bool parseData(const QByteArray &data);
		virtual bool parseNodeOpen(const QByteArray &name);
		virtual bool parseNodeClose(const QByteArray &name);
		virtual bool parseProperty(const QByteArray &name, const QByteArray &value);
		virtual void parseError(int chr, const QString &msg);

	protected:
		QByteArray	pdata;

		int getProp(const char *str, int s, int e);
		int getProps(const char *str, int s, int e);
		bool getNode(const char *str, int s, int e);
		bool getNodes(const char *str, int len);
		bool getData(const char *str, int s, int e);
		void parseErrorInt(const char *str, int pt, const QString &msg);
		bool parseDataInt(const char *str, int pt, const char *data);
		bool parsePropertyInt(const char *str, int pt, const char *name, const char *val);
};

#endif /* ifndef _XMLPARSER_H_ */

