#ifndef __HTTPHEADER_H_
#define __HTTPHEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

class HTTPHeader
{
public:
	/** constructor */
	HTTPHeader(string _key, string _value): key(_key), value(_value)
	//char *getValueByKey(DLL *, char *);
	
	//HTTPHeader *newENVPEntry(char *key, char *value);

	/** print key-value pair */
	void print(void *) {
		logger("header -[%s: %s]\n", key._str(), value.c_str());
	}

	int compare(HTTPHeader *h2);

	string getKey() { return key; }
	string getValue() { return Value; }
    
private:
    string key;
    string value;
};


#endif /* __HTTPHEADER_H_ */
