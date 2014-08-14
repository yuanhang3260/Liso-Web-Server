#ifndef __HTTPHEADER_H_
#define __HTTPHEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "Logger.h"

using namespace std;

class HTTPHeader
{
public:
	/** constructor */
	HTTPHeader(string _key, string _value): key(_key), value(_value) {}
	//char *getValueByKey(DLL *, char *);
	
	//HTTPHeader *newENVPEntry(char *key, char *value);

	/** print key-value pair */
	void print() {
		Logger::log("header -[%s: %s]\n", key.c_str(), value.c_str());
	}

	int compare(HTTPHeader *h2) { return key.compare(h2->key); }

	string getKey() { return key; }
	string getValue() { return value; }
    
private:
    string key;
    string value;
};


#endif /* __HTTPHEADER_H_ */
