/*****************************************************************************
 * xmlparser.cpp
 *
 *****************************************************************************/


#include <QDebug>
#include "xmlparser.h"
#include <stdlib.h>

enum {
	PROP_NAME_GET,
	PROP_EQUAL_WAIT,
	PROP_VALUE_WAIT,
	PROP_VALUE_GET,
	PROP_DONE
};

static inline void findCharLocation(const char *str, int pt, int &line, int &lchar) {
	int c, l = 1, lc = 0;
	for(c = 0; str[c] != 0 && c < pt; c++) {
		if(str[c] == '\n') {
			l++; // We just moved up a line
			lc = 0;
		}
		lc++;
	}
	line = l;
	lchar = lc;
	return;
}

static QByteArray unescapeize(const char *str, bool &error) {
	// Do a quick check to see if there is anything to desanitize
	int len = 0;
	int icc = 0;
	for(len = 0; str[len] != 0; len++) if(str[len] == '&') icc++;
	if(!icc) {
		error = false;
		return str;
	}

	// Ok- desanitize
        QByteArray buf;
        buf.reserve(len);
	for(int i = 0; i < len; i++) {
		if(str[i] == '&') {
			// scan for a semicolon
			int fs = 0;
			int ab = 0;
			char atom[10];
			for(int n = i + 1; (n < i + 8) && (n < len) && (!fs); n++) {
				char c = str[n];
				if(c == ';') {
					fs = n;
					atom[ab] = 0;
				} else {
					atom[ab] = c;
				}
				ab++;
			}
			// Error
			if(!fs) {
				error = true;
				return "expected a semicolon";
			}

			// Now try and figure out what code we are talking about
			if(!strcmp(atom, "amp")) {
				buf += '&';

			} else if(!strcmp(atom, "apos")) {
				buf += '\'';

			} else if(!strcmp(atom, "quot")) {
				buf += '\"';

			} else if(!strcmp(atom, "lt")) {
				buf += '<';

			} else if(!strcmp(atom, "gt")) {
				buf += '>';

			} else {
				error = true;
				return QByteArray("unknown escape code '") + QByteArray(atom) + QByteArray("'");
			}

			// Now skip over this
			i = fs;
		} else {
			buf += str[i];
		}
	}
	error = false;
	return buf;
}

// Virtual fuctions 

bool XMLParser::parseNodeOpen(const QByteArray &) {
	return true;
}

bool XMLParser::parseNodeClose(const QByteArray &) {
	return true;
}

bool XMLParser::parseProperty(const QByteArray &, const QByteArray &) {
	return true;
}

bool XMLParser::parseData(const QByteArray &) {
	return true;
}

void XMLParser::parseError(int pt, const QString &msg) {
        qDebug() << QString("PARSE ERROR [%1]: %2\n%3\n").arg(pt).arg(msg).arg(QString(pdata));
	return;
}

void XMLParser::parseErrorInt(const char *str, int pt, const QString &msg) {
	int line, lchar;
	findCharLocation(str, pt, line, lchar);
    QString err = msg + QString(" at line %1, char %2").arg(line).arg(lchar);
	parseError(pt, err);
	return;
}

bool XMLParser::parseDataInt(const char *str, int pt, const char *data) {
	bool error;
	QByteArray dval = unescapeize(data, error);
	if(error) {
		parseErrorInt(str, pt, "data parse error");
		error = false;
	} else {
		error = parseData(dval);
	}
	return error;
}

bool XMLParser::parsePropertyInt(const char *str, int pt, const char *name, const char *val) {
	bool error;
	QByteArray dval = unescapeize(val, error);
	if(error) {
		parseErrorInt(str, pt, "property parse error");
		error = false;
	} else {
		error = parseProperty(name, dval);
	}
	return error;
}

// Inline processing functions 
static inline bool isWhitespace(char c) {
	switch(c) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			return true;
	}
	return false;
}

static inline bool isCommentChar(char c) {
	switch(c) {
		case '?':
		case '!':
			return true;
	}
	return false;
}
	
static inline bool isOpenNode(char c) {
	return c == '<';
}

static inline bool isCloseNode(char c) {
	return c == '>';
}

static inline bool isNestUp(char c) {
	return c == '/';
}

static inline bool isEquals(char c) {
	return c == '=';
}

static inline bool isQuote(char c) {
	switch(c) {
		case '\'':
		case '\"':
			return true;
	}
	return false;
}

static inline bool isLegalNameChar(char c) {
	// Yea- I know this is a bit ugly - but it has to be fast.
	switch(c) {
		case '_':
		case '-':
		case ':':
		case '.':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			return true;
	}
	return false;
}

inline int XMLParser::getProp(const char *str, int s, int e) {
	char c;
	int state = PROP_NAME_GET;
        QByteArray name;
        name.reserve(e - s);
	QByteArray val;
        val.reserve(e - s);
	char quoteChar = '"';

	// Ignore any whitespace at the front
	while(s < e && state != PROP_DONE) {
		c = str[s];
		switch(state) {
			case PROP_NAME_GET:
				if(isLegalNameChar(c)) {
					name += c;
				} else if(isWhitespace(c)) {
					state = PROP_EQUAL_WAIT;
				} else if(isEquals(c)) {
					state = PROP_VALUE_WAIT;
				} else {
					parseErrorInt(str, s, "illegal character in property name");
					return -1;
				}
				break;
			case PROP_EQUAL_WAIT:
				if(isEquals(c)) {
					state = PROP_VALUE_WAIT;
				} else if(!isWhitespace(c)) {
					parseErrorInt(str, s, "illegal character between property name and =");
					return -1;
				}
				break;
			case PROP_VALUE_WAIT:
				if(isQuote(c)) {
					quoteChar = c;
					state = PROP_VALUE_GET;
				} else if(!isWhitespace(c)) {
					parseErrorInt(str, s, "illegal character between property = and value");
					return -1;
				}
				break;
			case PROP_VALUE_GET:
				// We should only match the same quote char used to start
				if(c == quoteChar) {
					state = PROP_DONE;
				} else {
					val += c;
				}
				break;
		}
		s++;
	}
	if(state == PROP_DONE) {
		if(name.isEmpty()) {
			parseErrorInt(str, s, "no property name");
			return -1;
		}
		if(!parsePropertyInt(str, s, name, val)) return -1;
	} else {
		parseErrorInt(str, s, "mangled property");
	}
	return s;
}
					
inline int XMLParser::getProps(const char *str, int s, int e) {
	char c;
	while(s < e) {
		c = str[s];
		if(isNestUp(c)) return s;
		if(isLegalNameChar(c)) { 
			s = getProp(str, s, e);
			if(s == -1) return -1;
			continue;
		}
		if(!isWhitespace(c)) {
			parseErrorInt(str, s, "illegal character while searching for node properties");
			return -1;
		}
		s++;
	}
	return s;
}

inline bool XMLParser::getNode(const char *str, int s, int e) {
        QByteArray name;
        name.reserve(e - s);
	char c;
	
	// Check to see if this is a close node
	if(isNestUp(str[s])) {
		s++;
		while(s < e) {
			c = str[s];
			if(!isLegalNameChar(c)) {
				parseErrorInt(str, s, "illegal character in close node");
				return false;
			}
			name += c;
			s++;
		}
		if(name.isEmpty()) {
			parseErrorInt(str, s, "close node has no name");
			return false;
		}
		if(!parseNodeClose(name)) return false;
		return true;
	}
	
	// Ok, get the name
	while(s < e) {
		c = str[s];
		if(isWhitespace(c)) {
			if(name.isEmpty()) { 
				parseErrorInt(str, s, "whitespace before start of node name");
				return false;
			} else {
				break;
			}
		}
			
		if(isNestUp(c)) break;
		
		if(!isLegalNameChar(c)) {
			parseErrorInt(str, s, "illegal character in node name");
			return false;
		}
                name += c;
		s++;
	}
	if(name.isEmpty()) {
		parseErrorInt(str, s, "node does not have a name");
		return false;
	}
	if(!parseNodeOpen(name)) return false;

	// Get the node properties
	s = getProps(str, s, e);
	if(s == -1) return false;
	
	// Now check to see if the node closes it's self
	while(s < e) {
		c = str[s];
		if(isNestUp(c)) {
			if(!parseNodeClose(name)) return false;
			break;
		}
		s++;
	}
	
	return true;
}

inline bool XMLParser::getData(const char *str, int s, int e) {
        QByteArray data;
        data.reserve(e - s);
	char c;
	bool go = false;
	while(s < e) {
		c = str[s];
		if(!isWhitespace(c)) {
			go = true;
			break;
		}
		s++;
	}
	if(!go) return true;
	while(s < e) {
                data += str[s];
		s++;
	}
	return parseDataInt(str, s, data);
}

inline bool XMLParser::getNodes(const char *str, int len) {
	char c;
	int loc = 0;
	int pc = -1, s = -1;
	bool inQuote = false;
	char quoteType = 0;
	bool inComment = false;

	while(loc < len) {
		c = str[loc];
		loc++;
		if(isQuote(c)) {
			if(inComment) continue;

			if(inQuote && (quoteType == c)) {
				inQuote = false;
			} else if(!inQuote) {
				inQuote = true;
				quoteType = c;
			}

		} else if(isOpenNode(c)) {

			// Ignore if we are already in a comment of quote
			if(inQuote || inComment) continue;
			
			// Check to see if this node is a comment
			char nc = (loc < len) ? str[loc] : 0;
			if(isCommentChar(nc)) {
				inComment = true;
				continue;
			}

			// Record the start of the node
			s = loc;

			// Check for any data between the last close node and this open node.
			if(pc != -1 && pc != (loc - 1)) 
				if(!getData(str, pc, loc - 1)) return false;

		} else if(isCloseNode(c)) {
			// Ignore if we are in a quote
			if(inQuote) continue;

			// Check for a close in the comments
			if(inComment) {
				char s1 = str[loc - 2];
				char s2 = str[loc - 3];
				if((s1 == '-' && s2 == '-') || s1 == '?') inComment = false;
				continue;
			}

			// Make sure we have a start position
			if(s == -1) {
				parseErrorInt(str, loc, "node close found before open");
				return false;
			}

			if(!getNode(str, s, loc - 1)) return false;
			s = -1;
			pc = loc;
		}
		
	}

	if(s != -1) {
		parseErrorInt(str, loc, "reached the end of data and node did not close");
		return false;
	}
	return true;
}

bool XMLParser::parse(const QString &s) {
	return parse(s.toLatin1());
}

bool XMLParser::parse(const QByteArray &s) {
	if(s.isNull()) return false;
	pdata = s;
	const char *str = s.data();
	int len = s.length();
	if(!len) return false;
	return getNodes(str, len);;
}

XMLParser::~XMLParser() {

}




