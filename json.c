/*
 ============================================================================
 Name        : c.c
 Author      : Chris Last
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "json.h"


typedef struct JsonDictionary
{
	char* key;
	struct JsonKey* pItem;
	struct JsonDictionary* next;
} JsonDictionary;

/*********************************************/

static int getObject(JsonObject* object);
static int getArray(JsonArray* array);
static int getKey(JsonKey* key);
static int getValue(JsonKey* key);
//static inline void delchar(char *pIn,int n);
static void display_array(char* prefix, JsonArray* array);
static void display_object(char* prefix, JsonObject* object);
static void addToDictionary(char* key, struct JsonKey* pItem);
static void freeJsonDictionary(void);
static void deleteObject(JsonObject* o);
static void deleteArray(JsonArray* a);
static void deleteKey(JsonKey* k);

/*********************************************/

static char *pIn, *pStart, *pEnd;
int dictionary_items=0;
JsonObject root={NULL,NULL};
JsonDictionary dictionary={NULL,NULL,NULL};

static int getObject(JsonObject* object)
{
	if (*pIn == '{')
	{
		pIn++; // remove {
		while (*pIn != '}')
		{
			if (*pIn == ',') pIn++;
			if (object->this != NULL)
			{
				object->next=(JsonObject*)malloc(sizeof(JsonObject));
				object=object->next;
				object->next=NULL;
			}
			object->this=(JsonKey*)malloc(sizeof(JsonKey));
			getKey(object->this);
		}
		pIn++; // remove terminating }
	}
	else if (*pIn == '[')
	{
		object->this=(JsonKey*)malloc(sizeof(JsonKey));
		object->this->key=(char*)malloc(sizeof(char));
		*(object->this->key)='\0';
		object->this->valuetype=JSON_ARRAY;
		object->this->value.pArray = (JsonArray*)malloc(sizeof(JsonArray));
		object->this->value.pArray->this = NULL;
		object->this->value.pArray->next = NULL;
		getArray(object->this->value.pArray);
	}
	else if (*pIn == '"')
	{
		object->this=(JsonKey*)malloc(sizeof(JsonKey));
		getValue(object->this);
		object->this->key=object->this->value.string;
		object->this->valuetype=JSON_NULL;
	}
	else
		return JSON_ERROR;
	return JSON_OK;
}

static int getArray(JsonArray* array)
{
	if (*pIn == '[') pIn++; // remove [
	while (*pIn != ']')
	{
		if (*pIn == ',')pIn++;
		if (array->this != NULL)
		{
			array->next=(JsonArray*)malloc(sizeof(JsonArray));
			array=array->next;
			array->next=NULL;
		}
		array->this=(JsonObject*)malloc(sizeof(JsonObject));
		array->this->this = NULL;
		array->this->next = NULL;
		getObject(array->this);
	}
	pIn++; // remove ]
	return JSON_OK;
}

static int getKey(JsonKey* key)
{
	int length = 0;
	// Find key start
	while (*pIn != '"') pIn++; pIn++; // remove the quote
	// Get Key
	length=strcspn(pIn,"\"");
	key->key=(char*)malloc(length+1);
	strncpy(key->key, pIn, length);
	key->key[length]='\0';
	pIn+=(length+1);                  // remove the key and double quote
	while (*pIn != ':') pIn++; pIn++; // remove the key value pair separator
	// Get value
	getValue(key);
	return JSON_OK;
}

static int getValue(JsonKey* key)
{
	int length = 0;
	char* pC;

	// Find value start
	length=strcspn(pIn,"\"-0123456789{[FtfT,");
	pIn+=length;
	switch (*pIn)
	{
	case '"':
		key->valuetype = JSON_STRING;
		pC = ++pIn;
		while (*pC != '"')
		{
			if (*pC == '\\')
			{
					pC++;
					length++;
			}
			pC++;
			length++;
		}
		key->value.string = (char*)malloc(length+1);
		strncpy(key->value.string,pIn,length);
		key->value.string[length]='\0';
		pIn+=(length+1); // remove string and "
		break;
	case '{':
	{
		key->valuetype = JSON_OBJECT;
		key->value.pObject=(JsonObject*)malloc(sizeof(JsonObject));
		key->value.pObject->this = NULL;
		key->value.pObject->next = NULL;
		getObject(key->value.pObject);
		break;
	}
	case '[':
	{
		key->valuetype = JSON_ARRAY;
		key->value.pArray=(JsonArray*)malloc(sizeof(JsonArray));
		key->value.pArray->this = NULL;
		key->value.pArray->next = NULL;
		getArray(key->value.pArray);
		break;
	}
	case 'F':
	case 'f':
		key->valuetype = JSON_BOOLEAN;
		key->value.boolean=0;
		pIn+=5;
		break;
	case 'T':
	case 't':
		key->valuetype = JSON_BOOLEAN;
		key->value.boolean=1;
		pIn+=4;
		break;
	case ',':
		key->valuetype = JSON_NULL;
		break;
	default:
		pC=pIn;
		key->valuetype = JSON_INTEGER;
		while (*pC == '-' || *pC =='.' || (*pC>='0' && *pC<='9'))
		{
			if (*pC == '.')
			{
				if (key->valuetype == JSON_REAL)
					return JSON_ERROR;
				else
					key->valuetype = JSON_REAL;
			}
			pC++;
			length++;
		}
		if (key->valuetype == JSON_INTEGER)
			sscanf(pIn,"%li",&key->value.integer);
		else
			sscanf(pIn,"%lf",&key->value.real);
		pIn+=length;
		break;
	}
	return JSON_OK;
}

//static inline void delchar(char *pIn,int n) { if (n>0 && ((pIn+n)<=pEnd)) { while (*pIn){*pIn=*(pIn+n);pIn++;}pEnd-=n;} }

static void add_key(char* key)
{
	if (JsonKeys < JSON_KEYS_MAX)
	{
		size_t key_length=strlen(key);
		if (key_length <= JSON_LABEL_LENGTH_MAX)
		{
			JsonKeyList[JsonKeys]=(char *)malloc(key_length+1);
			strcpy(JsonKeyList[JsonKeys],key);
			JsonKeys++;
		}
	}
}

static void display_object(char* prefix, JsonObject* object)
{
	char label[JSON_LABEL_LENGTH_MAX+1];
	while (object)
	{
		sprintf(label,"%s:%s",prefix,object->this->key);

		switch (object->this->valuetype)
		{
		case JSON_INTEGER:
			printf ("%s=%li\n",label,object->this->value.integer);
			addToDictionary(label,object->this);
			break;
		case JSON_REAL:
			printf ("%s=%lf\n",label,object->this->value.real);
			addToDictionary(label,object->this);
			break;
		case JSON_BOOLEAN:
			if (object->this->value.boolean)
				printf ("%s=TRUE\n",label);
			else
				printf ("%s=FALSE\n",label);
			addToDictionary(label,object->this);
			break;
		case JSON_ARRAY:
			display_array(label,object->this->value.pArray);
			break;
		case JSON_OBJECT:
			display_object(label,object->this->value.pObject);
			break;
		case JSON_STRING:
			printf ("%s=%s\n",label,object->this->value.string);
			addToDictionary(label,object->this);
			break;
		case JSON_NULL:
			printf ("%s exists\n",label);
			addToDictionary(label,object->this);
			break;
		default:
			printf ("Object Error!\n");
			break;
		}
		object=object->next;
	}
}

static void display_array(char* prefix, JsonArray* array)
{
	char label[JSON_LABEL_LENGTH_MAX];
	int count=0;
	while (array)
	{
		sprintf(label,"%s[%d]",prefix,count++);
		display_object(label,array->this);
		array=array->next;
	}
}

static void addToDictionary(char* key, JsonKey* pItem)
{
	JsonDictionary* new;
	JsonDictionary* dict=&dictionary;
	int length=strlen(key);
	if (dict->key==NULL)
		new = dict;   // dictionary is empty so use the root node
	else
		new = (JsonDictionary*)malloc(sizeof(JsonDictionary));
	{
		new->key=(char*)malloc(length+1);
		strcpy (new->key,key);
		new->pItem=pItem;
		new->next=NULL;
		// If we're not using the dictionary root
		if (new != &dictionary)
		{ // then append this new key
			while (dict->next)
				dict=dict->next;
			dict->next=new;
		}
	}
	add_key(key);
}

/* return a pointer to a JSON key value */
JsonKey* JsonGet(char* key)
{
	JsonDictionary* dict=&dictionary;
	if (key)
	while (dict && dict->key)
	{
		if (!strcmp(key,dict->key))
			return dict->pItem;
		dict=dict->next;
	}
	return NULL;
}

static void deleteObject(JsonObject* o)
{
	if (o->next) deleteObject(o->next);
	if (o->this) deleteKey(o->this);
	if (o != &root) free(o);
}

static void deleteArray(JsonArray* a)
{
	if (a->next) deleteArray(a->next);
	if (a->this) deleteObject(a->this);
	free(a);
}

static void deleteKey(JsonKey* k)
{
	if (k->valuetype==JSON_STRING) free (k->value.string);
	if (k->valuetype==JSON_OBJECT) deleteObject (k->value.pObject);
	if (k->valuetype==JSON_ARRAY) deleteArray (k->value.pArray);
	if (k->key) free (k->key);
	free (k);
}

static void deleteDictionary(JsonDictionary* d)
{
	if (d->key) free (d->key);
	if (d->next) deleteDictionary(d->next);
	if (d != &dictionary) free (d);
}

static void deleteKeyList(void)
{
	int i;
	for (i=0; i<JSON_KEYS_MAX; i++)
	{
		if (JsonKeyList[i])
		{
			free(JsonKeyList[i]);
			JsonKeyList[i]=NULL;
		}
	}
	JsonKeys=0;
}

static void freeJsonDictionary(void)
{
	deleteObject(&root);
	root.next=NULL;
	root.this=NULL;
	deleteDictionary(&dictionary);
	dictionary.next=NULL;
	dictionary.key=NULL;
	deleteKeyList();
}

JsonReturnType JsonLoad(const char* string)
{
	JsonReturnType status;
	freeJsonDictionary();
	if (string && *string)
	{
		pStart=(char*)malloc(strlen(string)+1);
		pIn=pStart;
		strcpy(pIn,string);
		pIn+=(strcspn(pIn,"{["));
		pEnd = pIn+strlen(pIn);
		status = getObject(&root);
		display_object("root",&root);
		free(pStart);
	}
	else
		return JSON_ERROR;
	return status;
}
