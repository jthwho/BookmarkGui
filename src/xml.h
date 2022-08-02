/*******************************************************************************
 * xml.h
 ******************************************************************************/

#ifndef _XML_H_
#define _XML_H_

#include <QString>
#include <QHash>

typedef QHash<QString, QString>		XMLPropList;

class XMLNodeData {
	public:
		int			count;
		XMLNodeData 		*parent;	// Parent Node
		XMLNodeData		*next;		// Next Sibling Node
		XMLNodeData		*prev;		// Next Sibling Node
		XMLNodeData		*first;		// First Child Node
		XMLNodeData		*last;		// Last Child Node
		QString			name;
		QString			data;
		XMLPropList		props;

		XMLNodeData();
		XMLNodeData(const QString &n);
		
		static void addChildNodeToLast(XMLNodeData *p, XMLNodeData *i);
		static void addChildNodeToFirst(XMLNodeData *p, XMLNodeData *i);
		static void addChildNode(XMLNodeData *p, XMLNodeData *i, int loc);
		static XMLNodeData *findChildNode(const XMLNodeData *i, int loc);
		static XMLNodeData *copyNode(const XMLNodeData *i);
			
		XMLNodeData *copy() const;
		XMLNodeData *findChild(int c) const;
		XMLNodeData *findChild(const QString &d, int c) const;
		XMLNodeData *addChild(const QString &d, int c);
		XMLNodeData *addChild(const XMLNodeData &d, int c);
		void removeChild(XMLNodeData *i);
			
		void remove(); 
		void removeChildren(); 
		
	protected:
		~XMLNodeData();
};


class XMLNode {
	friend class XML;
	
	private:
		static XMLPropList null_props;

	protected: 
		XMLNodeData *d;
		
	public:
		~XMLNode();
		XMLNode();
		XMLNode(XMLNodeData *d);
		XMLNode(const XMLNode &d);
		
		XMLNode &operator=(XMLNodeData *d);
		XMLNode &operator=(const XMLNode &d);
		
		void remove();
		bool gotoNext();
		bool gotoPrev();
		bool gotoParent();
		bool gotoFirstChild();
		bool gotoLastChild();
		
		// Convert from this node on to a string
		QString toString(int level = 0) const;

		// Save the node to a file
		bool toFile(const QString &fname) const;

		// Dump the node to the log
		void dump() const;

		XMLNode addChild(const XMLNode &d, int loc = -1) const;
		XMLNode addChild(const QString &d, int loc = -1) const;
		XMLNode child(const QString &d, int n = 0) const;
		XMLNode child(int n) const;
		int count() const;
		void removeChild(const QString &d, int n = 0) const;
		void removeChild(int n) const;
		QString name() const;
		void setName(const QString &d);
		QString data() const;
		void setData(const QString &d);
		void propSet(const QString &key, const QString &val);
		XMLPropList &props() const;
		QString propGet(const QString &key) const;
		bool isValid() const;
		bool isOrphan() const;
		bool isRoot() const;
		bool hasChildren() const;
		bool hasData() const;
		bool hasProps() const;
		XMLNode next() const;
		XMLNode prev() const;
		XMLNode parent() const;
		XMLNode firstChild() const;
		XMLNode lastChild() const;
};


	
class XML {
	protected:
		XMLNodeData *data;
		
	public:
		static XML fileToXML(const QString &fname);
		static XML stringToXML(const QString &str);

		~XML();
		
		XML(); 
		XML(const QString &rname); 
		XML(const XML &t); 
		XML(const XMLNode &n); 
		XML &operator=(const XML &t);
		XML &operator=(XMLNode &n);
		
		operator XMLNode();
		void clear();
		XMLNode root() const;
		bool isValid() const;
		
};
		
#endif /* ifndef _XML_H_ */


