

#ifndef _CJSON_H
#define _CJSON_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "palloc.h"
/* json_st Types: */
#define json_False 0
#define json_True 1
#define json_NULL 2
#define json_Number 3
#define json_String 4
#define json_Array 5
#define json_Object 6
	
#define json_IsReference 256

/* The json_st structure: */
typedef struct json_st {
	struct json_st *next,*prev;	/* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
	struct json_st *child;		/* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

	int type;					/* The type of the item, as above. */

	char *valuestring;			/* The item's string, if type==json_String */
	int valueint;				/* The item's number, if type==json_Number */
	double valuedouble;			/* The item's number, if type==json_Number */

	char *string;				/* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} json_st;

typedef struct json_Hooks {
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
} json_Hooks;

/* Supply malloc, realloc and free functions to json_st */
extern void json_initHooks(pool_t);

extern void printf_num();

/* Supply a block of JSON, and this returns a json_st object you can interrogate. Call json_Delete when finished. */
extern json_st *json_parse(const char *value);
/* Render a json_st entity to text for transfer/storage. Free the char* when finished. */
extern char  *json_print(json_st *item);
/* Render a json_st entity to text for transfer/storage without any formatting. Free the char* when finished. */
extern char  *json_printUnformatted(json_st *item);
/* Delete a json_st entity and all subentities. */
extern void   json_delete(json_st *c);

/* Returns the number of items in an array (or object). */
extern int	  json_getArraySize(json_st *array);
/* Retrieve item number "item" from array "array". Returns NULL if unsuccessful. */
extern json_st *json_getArrayItem(json_st *array,int item);
/* Get item "string" from object. Case insensitive. */
extern json_st *json_getObjectItem(json_st *object,const char *string);

/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when json_Parse() returns 0. 0 when json_Parse() succeeds. */
extern const char *json_getErrorPtr();
	
/* These calls create a json_st item of the appropriate type. */
extern json_st *json_createNull();
extern json_st *json_createTrue();
extern json_st *json_createFalse();
extern json_st *json_createBool(int b);
extern json_st *json_createNumber(double num);
extern json_st *json_createString(const char *string);
extern json_st *json_createArray();
extern json_st *json_createObject();

/* These utilities create an Array of count items. */
extern json_st *json_createIntArray(int *numbers,int count);
extern json_st *json_createFloatArray(float *numbers,int count);
extern json_st *json_createDoubleArray(double *numbers,int count);
extern json_st *json_createStringArray(const char **strings,int count);

/* Append item to the specified array/object. */
extern void json_addItemToArray(json_st *array, json_st *item);
extern void	json_addItemToObject(json_st *object,const char *string,json_st *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing json_st to a new json_st, but don't want to corrupt your existing json_st. */
extern void json_addItemReferenceToArray(json_st *array, json_st *item);
extern void	json_addItemReferenceToObject(json_st *object,const char *string,json_st *item);

/* Remove/Detatch items from Arrays/Objects. */
extern json_st *json_detachItemFromArray(json_st *array,int which);
extern void   json_deleteItemFromArray(json_st *array,int which);
extern json_st *json_detachItemFromObject(json_st *object,const char *string);
extern void   json_deleteItemFromObject(json_st *object,const char *string);
	
/* Update array items. */
extern void json_replaceItemInArray(json_st *array,int which,json_st *newitem);
extern void json_replaceItemInObject(json_st *object,const char *string,json_st *newitem);

#define json_addNullToObject(object,name)	json_addItemToObject(object, name, json_createNull())
#define json_addTrueToObject(object,name)	json_addItemToObject(object, name, json_createTrue())
#define json_addFalseToObject(object,name)		json_addItemToObject(object, name, json_createFalse())
#define json_addNumberToObject(object,name,n)	json_addItemToObject(object, name, json_createNumber(n))
#define json_addStringToObject(object,name,s)	json_addItemToObject(object, name, json_createString(s))

#ifdef __cplusplus
}
#endif

#endif
