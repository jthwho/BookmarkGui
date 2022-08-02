/*****************************************************************************
 * xml.cpp
 * July 31, 2014
 *
 *****************************************************************************/

#include <QDebug>
#include <QFile>
#include "xml.h"
#include "xmlparser.h"

class XMLTreeParser : public XMLParser {
	private:
		int level;
		XMLNode curnode;

	public:
		bool parseData(const QByteArray &d) {
			curnode.setData(d);
			return true;
		}
		
		bool parseNodeOpen(const QByteArray &n) {
			QString name = n;
			if(name.contains(':')) name = name.section(':', 1, 1);
			if(!level) {
				curnode.setName(name);
				level = 1;
			} else {
				curnode = curnode.addChild(name);
				level++;
			}
			return true;
		}
		
		bool parseNodeClose(const QByteArray &n) {
			QString name = n;
			if(name.contains(':')) name = name.section(':', 1, 1);
			if(!level) return false;
			if(name != curnode.name()) return false;
			curnode = curnode.parent();
			level--;
			return true;
		}
		
		bool parseProperty(const QByteArray &n, const QByteArray &v) {
			QString name = n;
			if(name.contains(':')) name = name.section(':', 1, 1);
			curnode.propSet(name, v);
			return true;
		}
		
		XML parseString(const QString &d) {
			XML xml;
			level = 0;
			curnode = xml.root();
			if(!parse(d)) {
				qWarning() << "parse error";
				return XML(); // was an error in the parsing
			}
			if(level) {
				qWarning() << "level error " << level;
				return XML(); // All nodes did not close
			}
			return xml;
		}
};

inline static bool isQuote(char v) {
	switch(v) {
		case '\'': // '
		case '\"':
			return true;
	}
	return false;
}

inline static bool isIllegal(char v) {
	switch(v) {
		case '&':
		case '\'': // '
		case '\"':
		case '<':
		case '>':
			return true;
	}
	return false;
}

inline static QString escapeize(const QString &str) {
	// Do a quick check to see if there is anything to sanitize
	int len = str.length();
	int icc = 0;
	for(int i = 0; i < len; i++) if(isIllegal(str[i].toAscii())) icc ++;
	if(!icc) return str;

	// Ok- sanitize!
	QByteArray buf;
        buf.reserve(len + icc * 7);
	for(int i = 0; i < len; i++) {
		char c = str[i].toAscii();

		switch(c) {
			case '&':  buf += "&amp;"; break;
			case '\'': buf += "&apos;"; break;
			case '\"': buf += "&quot;"; break;
			case '<':  buf += "&lt;"; break;
			case '>':  buf += "&gt;"; break;
			default:   buf += c; break;
		}
	}
	return buf;
}



///////////////////////////////////////////////////////////////////////////////
// XMLNodeData

XMLNodeData::XMLNodeData() : count(0), parent(NULL), next(NULL), prev(NULL), 
				 first(NULL), last(NULL) { 
				 
}

XMLNodeData::XMLNodeData(const QString &n) : count(0), parent(NULL), next(NULL), prev(NULL), 
				 first(NULL), last(NULL), name(n) { 
				 				 
}

XMLNodeData::~XMLNodeData() {

}


inline XMLNodeData *XMLNodeData::findChildNode(const XMLNodeData *p, int c) {
	XMLNodeData *i = p->first;
	int n = 0;
	while(i) {
		if(c == n) return i;
		i = i->next;
		n++;
	}
	return NULL;
}

inline void XMLNodeData::addChildNodeToLast(XMLNodeData *p, XMLNodeData *i) {
	i->parent = p;
	p->count++;
	if(p->last) {
		p->last->next = i;
		i->prev = p->last;
		p->last = i;
		i->next = NULL;
	} else {
		// First item
		p->first = i;
		p->last = i;
		i->prev = NULL;
		i->next = NULL;
	}
	return;
}

inline void XMLNodeData::addChildNodeToFirst(XMLNodeData *p, XMLNodeData *i) {
	i->parent = p;
	p->count++;
	if(p->first) {
		p->first->prev = i;
		i->next = p->first;
		p->first = i;
		i->prev = NULL;
	} else {	
		// First child to be added
		p->first = i;
		p->last = i;
		i->prev = NULL;
		i->next = NULL;
	}
	return;
}

inline void XMLNodeData::addChildNode(XMLNodeData *p, XMLNodeData *i, int loc) {
	// If the location was set to -1, add to the back of the list
	if(loc < 0) return addChildNodeToLast(p, i);
	// If the location was set to 0, add to the front of the list
	if(!loc) return addChildNodeToFirst(p, i);
	// Now try to find the loc item
	XMLNodeData *bi = findChildNode(p, loc - 1);
	// Check to see if we just need to add and item to the end
	if(!bi) return addChildNodeToLast(p, i);
	if(bi == p->last) return addChildNodeToLast(p, i);
	// Now we can be assured that the item has and item before and after it.
	i->prev = bi;
	i->next = bi->next;
	bi->next->prev = i;
	bi->next = i;
	p->count++;
	return;
}

XMLNodeData *XMLNodeData::copyNode(const XMLNodeData *i) {
	XMLNodeData *c, *n, *ni = new XMLNodeData(i->name);
	// Set the data
	ni->data = i->data;
	ni->props = i->props;
	ni->next = NULL;
	ni->prev = NULL;
	ni->parent = NULL;
	// Recurse through it's children
	n = i->first;
	int ct = 0;
	while(n) {
		c = copyNode(n);
		addChildNodeToLast(ni, c);
		n = n->next;
		ct++;
	}
	ni->count = ct;
	return ni;
}


XMLNodeData *XMLNodeData::copy() const {
	return copyNode(this);
}

XMLNodeData *XMLNodeData::findChild(int c) const {
	return findChildNode(this, c);
}

XMLNodeData *XMLNodeData::findChild(const QString &d, int c) const {
	XMLNodeData *i = first;
	int n = 0;
	while(i) {
		if(i->name == d) {
			if(n == c) 
				return i;
			else
				n++;
		}
		i = i->next;
	}
	return NULL;
}

XMLNodeData *XMLNodeData::addChild(const QString &d, int c) {
	XMLNodeData *i = new XMLNodeData(d);
	addChildNode(this, i, c);
	return i;
}

XMLNodeData *XMLNodeData::addChild(const XMLNodeData &d, int c) {
	XMLNodeData *i = d.copy();
	addChildNode(this, i, c);
	return i;
}


void XMLNodeData::remove() {
	/* Unreference the children */
	removeChildren();

	/* Remove thyself from thy parent's list */
	if(parent) {
		parent->count--;
		if(parent->first == this) parent->first = next;
		if(parent->last == this) parent->last = prev;
	}
	parent = NULL;
	if(next) next->prev = prev;
	if(prev) prev->next = next;
	next = NULL;
	prev = NULL;
	delete this;
	return;
}


void XMLNodeData::removeChildren() {
	XMLNodeData *nt, *it = first;
	while(it) {
		nt = it->next;
		it->remove();
		count--;
		it = nt;
	}
	first = NULL;
	last = NULL;
}




///////////////////////////////////////////////////////////////////////////////
// XMLNode

XMLPropList XMLNode::null_props;

XMLNode::~XMLNode() { 

}

XMLNode::XMLNode() : d(NULL) {

}

XMLNode::XMLNode(XMLNodeData *_d) : d(_d) { 

}

XMLNode::XMLNode(const XMLNode &_d) : d(_d.d) {

}

bool XMLNode::toFile(const QString &fname) const {
	QString val = toString();
	if(val.isNull()) return false;
	QFile file(fname);
	if(!file.open(QIODevice::WriteOnly)) return false;
	file.write(val.toAscii());
	return true;
}

QString XMLNode::toString(int level) const {
	if(d == NULL) return QString();
	QString str;
	QString pad = QString().fill('\t', level);
	if(!level) str += "<?xml version=\"1.0\"?>\n";
	str += pad + '<' + d->name;
	for(XMLPropList::const_iterator i = d->props.constBegin(); i != d->props.constEnd(); ++i) {
		str += ' ' + i.key() + "=\"" + escapeize(i.value()) + "\"";
	}
	if(hasChildren() || hasData()) {
		if(hasChildren()) {
			str += ">\n";
			XMLNode n = firstChild();
			level++;
			while(n.isValid()) {
				str += n.toString(level) + '\n';
				n.gotoNext();
			}
		} else {
			str += '>';
		}
		if(hasData()) str += escapeize(d->data);
		if(hasChildren()) str += pad;
		str += "</" + d->name + '>';
	} else {
		str += "/>";
	}
	return str;
}

void XMLNode::dump() const {
	qDebug() << toString();
	return;
}

XMLNode &XMLNode::operator=(XMLNodeData *_d) { 
	d = _d; 
	return *this; 
}

XMLNode &XMLNode::operator=(const XMLNode &_d) { 
	d = _d.d; 
	return *this; 
}

void XMLNode::remove() {
	if(d == NULL) return;
	// We don't want to be able to remove a root node!
	if(d->parent == NULL) return;
	d->remove();
	d = NULL;
}

bool XMLNode::gotoNext() {
	if(d == NULL) return false;
	d = d->next;
	return (d != NULL);
}

bool XMLNode::gotoPrev() {
	if(d == NULL) return false;
	d = d->prev;
	return (d != NULL);
}

bool XMLNode::gotoParent() {
	if(d == NULL) return false;
	d = d->parent;
	return (d != NULL);
}

bool XMLNode::gotoFirstChild() {
	if(d == NULL) return false;
	d = d->first;
	return (d != NULL);
}

bool XMLNode::gotoLastChild() {
	if(d == NULL) return false;
	d = d->last;
	return (d != NULL);
}

XMLNode XMLNode::addChild(const XMLNode &val, int loc) const {
	if(d == NULL) return XMLNode();
	if(val.d == NULL) return XMLNode();
	return XMLNode(d->addChild(*val.d, loc));
};

XMLNode XMLNode::addChild(const QString &val, int loc) const {
	if(d == NULL) return XMLNode();
	return XMLNode(d->addChild(val, loc));
}

XMLNode XMLNode::child(const QString &val, int n) const {
	if(d == NULL) return XMLNode();
	return XMLNode(d->findChild(val, n));
}

XMLNode XMLNode::child(int n) const {
	if(d == NULL) return XMLNode();
	return XMLNode(d->findChild(n));
}

int XMLNode::count() const {
	if(d == NULL) return 0;
	return d->count;
}

void XMLNode::removeChild(const QString &val, int n) const {
	if(d == NULL) return;
	XMLNodeData *i = d->findChild(val, n);
	if(i) i->remove();
	return;
}

void XMLNode::removeChild(int n) const {
	if(d == NULL) return;
	XMLNodeData *i = d->findChild(n);
	if(i) i->remove();
	return;
}

QString XMLNode::name() const {
	if(d == NULL) return QString();
	return d->name;
}

void XMLNode::setName(const QString &val) {
	if(d == NULL) return;
	d->name = val;
}

QString XMLNode::data() const {
	if(d == NULL) return QString();
	return d->data;
}

void XMLNode::setData(const QString &val) {
	if(d == NULL) return;
	d->data = val;
}

void XMLNode::propSet(const QString &key, const QString &val) {
	if(d == NULL) return;
	d->props[key] = val;
	return;
}

XMLPropList &XMLNode::props() const {
	if(d == NULL) return null_props;
	return d->props;
}

QString XMLNode::propGet(const QString &key) const {
	if(d == NULL) return QString();
	return d->props[key];
}

bool XMLNode::isValid() const {
	return (d != NULL);
}

bool XMLNode::isOrphan() const {
	if(d == NULL) return false;
	if(d->parent == NULL && 	// No parent
			d->next == NULL && 	// No siblings
			d->prev == NULL &&	
			d->first == NULL)	// No children
		return true;
	return false;
}

bool XMLNode::isRoot() const {
	if(d == NULL) return false;
	return (d->parent == NULL);
}

bool XMLNode::hasChildren() const {
	if(d == NULL) return false;
	return (d->count != 0);
}

bool XMLNode::hasData() const {
	if(d == NULL) return false;
	return (!d->data.isEmpty());
}

bool XMLNode::hasProps() const {
	if(d == NULL) return false;
	return (d->props.count() != 0);
}

XMLNode XMLNode::next() const {
	if(d == NULL) return XMLNode(NULL);
	return XMLNode(d->next);
}

XMLNode XMLNode::prev() const {
	if(d == NULL) return XMLNode(NULL);
	return XMLNode(d->prev);
}

XMLNode XMLNode::parent() const {
	if(d == NULL) return XMLNode(NULL);
	return XMLNode(d->parent);
}

XMLNode XMLNode::firstChild() const {
	if(d == NULL) return XMLNode(NULL);
	return XMLNode(d->first);
}

XMLNode XMLNode::lastChild() const {
	if(d == NULL) return XMLNode(NULL);
	return XMLNode(d->last);
}


///////////////////////////////////////////////////////////////////////////////
// XML 


// Static Functions 

XML XML::fileToXML(const QString &fname) {
	QFile file(fname);
	if(!file.open(QIODevice::ReadOnly)) return XML();
	QString str = file.readAll();
	return stringToXML(str);
}

XML XML::stringToXML(const QString &s) {
	if(s.isNull())
		return XML();
	XMLTreeParser parser;
	return parser.parseString(s);
}



XML::~XML() { 
	data->remove(); 
}
		
XML::XML() { 
	data = new XMLNodeData(); 
}

XML::XML(const QString &rname) { 
	data = new XMLNodeData(rname); 
}

XML::XML(const XML &t) { 
	data = t.data->copy(); 
}

XML::XML(const XMLNode &n) { 
	if(n.d) {
		data = n.d->copy();
	} else {
		data = new XMLNodeData();
	}
}

XML &XML::operator=(const XML &t) {
	data->remove();
	data = t.data->copy();
	return *this;
}

XML &XML::operator=(XMLNode &n) {
	data->remove();
	if(n.d) {
		data = n.d->copy();
	} else {
		data = new XMLNodeData();
	}
	return *this;
}

XML::operator XMLNode() {
	return XMLNode(data);
}

void XML::clear() {
	data->remove();
	data = new XMLNodeData();
}

XMLNode XML::root() const {
	return XMLNode(data);
}

bool XML::isValid() const {
	return !data->name.isEmpty();
}




