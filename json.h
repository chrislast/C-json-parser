/*
 * json.h
 *
 *  Created on: 23 Mar 2016
 *      Author: clast
 */

#ifndef JSON_H_
#define JSON_H_


#define JSON_KEYS_MAX 10000
#define JSON_LABEL_LENGTH_MAX 256

enum JsonValueType;
enum JsonReturnType;
union JsonValue;
struct JsonArray;
struct JsonObject;

typedef enum JsonValueType
{
	JSON_ARRAY=1,
	JSON_OBJECT,
	JSON_STRING,
	JSON_INTEGER,
	JSON_REAL,
	JSON_BOOLEAN,
	JSON_NULL
} JsonValueType;


typedef union JsonValue
{
	struct JsonArray* pArray;
	struct JsonObject* pObject;
	char* string;
	long integer;
	double real;
	int boolean;
} JsonValue;

// A value is a single object, array or data item
typedef struct JsonKey
{
	char* key;
	enum JsonValueType valuetype;
	union JsonValue value;
} JsonKey;

// An object is a list of key value pairs
typedef struct JsonObject
{
	struct JsonKey* this;
	struct JsonObject* next;
} JsonObject;

// An array is a list of objects
typedef struct JsonArray
{
	struct JsonObject* this;
	struct JsonArray* next;
} JsonArray;

typedef enum JsonReturnType
{
	JSON_OK=0,
	JSON_EMPTY,
	JSON_ERROR
} JsonReturnType;


JsonKey* JsonGet(char* key);
JsonReturnType JsonLoad(const char* string);

int JsonKeys;
char *JsonKeyList[JSON_KEYS_MAX];

#endif /* JSON_H_ */
