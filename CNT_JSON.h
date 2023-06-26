#ifndef CNT_JSON__h
#define CNT_JSON__h

#include "cJSON.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <exception>
using namespace std;

#define INVALID_ID -9999

/*
    C++11 smart ptr wrappers

   Usage:

    create cJSON object with unique ptr:
      cJSONPtr = foo(cJSON_CreateObject())

    create cJSON object with shared ptr:
      cJSONSharedPtr = foo2(cJSON_CreateObject())

 */
// struct cJSON_Deleter {
//   void operator()(cJSON*  ptr) { if (ptr) cJSON_Delete(ptr); }
// };
 
// // shared ptr:
// template<class T, class D = std::default_delete<T>>
// struct shared_ptr_with_deleter : public std::shared_ptr<T>
// {
//   explicit shared_ptr_with_deleter(T* t = nullptr)
//       : std::shared_ptr<T>(t, D()) {_t = t;}
 
//   void reset(T* t = nullptr) {
//     std::shared_ptr<T>::reset(t, D());
//   }

//   T* get() { return _t;}

//   T* _t;
// };
// // unique ptr:
// typedef std::unique_ptr<cJSON, cJSON_Deleter> cJSONPtr;
// typedef shared_ptr_with_deleter<cJSONPtr,  cJSON_Deleter> cJSONSharedPtr;


void delete_cJSON(cJSON* obj);

// some typedefs for using cJSON for RPC calls, ie Responses, results, etc...
typedef cJSON* RPCObj;
typedef cJSON* RPC_RespObj;
typedef cJSON* RPC_Params;
typedef cJSON* RPC_Result;

typedef map<string, string> ARG;

// Synonyms for using cJSON objects as output or input/output when
// wrapped in Swig.
// - cJSON*: is only input, 
// - cJSON_out*: doesn't consume a parameter and it's returned in the output
// - cJSON_inout*: consumes the parameter, and returns the modification on the output
typedef cJSON cJSON_out;
typedef cJSON cJSON_inout;      /* Not implemented yet (maybe unnecessary) */

enum ResponseType_t
{
   RES_COMPLETE = 0, RES_UPDATE, RES_ERROR
};

RPCObj createRPCObj(string method, int id);
RPCObj createRPCObj(string method, int id, map<string, string> Args);

void recycleRPCObj(RPCObj RPC_Obj, string method, int id);

RPC_RespObj createRPC_RespObj(RPCObj RPC_Obj);
RPC_RespObj createRPC_RespObj(int id);

void addParam(RPCObj RPC_Obj, string key, int param);
void addParam(RPCObj RPC_Obj, string key, double param);
void addParam(RPCObj RPC_Obj, string key, string param);
void addParam(RPCObj RPC_Obj, string key, bool param);

bool addParamsMap(RPCObj RPC_Obj, map<string, string> Args);
bool addParamsMap(RPCObj RPC_Obj, map<string, double> Args);

bool doesAttributeExist(cJSON* JSONObj, string AttributeKey, bool recurse = false);

int getAttributeDefault_Int(cJSON* JSONObj, string AttributeKey, int _default);
bool getAttributeValue_Int(cJSON* JSONObj, string AttributeKey, int &valueint);
bool getAttributeValue_IntArray(cJSON* JSONObj, string AttributeKey, int* valueint, unsigned int nValues);
bool setAttributeValue_Int(cJSON* JSONObj, string AttributeKey, int valueint);
bool incrAttributeValue_Int(cJSON* JSONObj, string AttributeKey, int incr = 1);

bool getAttributeDefault_Bool(cJSON* JSONObj, string AttributeKey, bool _default);
bool getAttributeValue_Bool(cJSON* JSONObj, string AttributeKey, bool &valuebool);
bool setAttributeValue_Bool(cJSON* JSONObj, string AttributeKey, bool valuebool);

double getAttributeDefault_Double(cJSON* JSONObj, string AttributeKey, double _default);
bool getAttributeValue_Double(cJSON* JSONObj, string AttributeKey, double &valuedouble);
bool setAttributeValue_Double(cJSON* JSONObj, string AttributeKey, double valuedouble);
bool incrAttributeValue_Double(cJSON* JSONObj, string AttributeKey, double incr = 1.0);

bool getAttributeValue_String(cJSON* JSONObj, string AttributeKey, string &stringval);
bool setAttributeValue_String(cJSON* JSONObj, string AttributeKey, string stringval);

bool MapToJSON_Str_Double(map<string, double> map_double, cJSON **JSONObj);

bool MapToJSON_Int_Double(map<int, double> map_double, cJSON **JSONObj);

bool JSONToMap_Str_Double(cJSON *JSONObj, map<string, double> *map_double);
bool JSONToMap_Str_Str(cJSON *JSONObj, map<string, string> *map_str_str);
bool JSONToMap_Int_Double(cJSON *JSONObj, map<int, double> *map_double);

bool writeJSONToFile(cJSON *object, string filename);
bool readJSONFromFile(cJSON **object, string filename);

bool mergeJSONObjects(cJSON *targetJSONObj, cJSON *sourceJSONObj, bool overwrite = true);
bool diffJSONObjects(cJSON *JSONObj1, cJSON *JSONObj2, cJSON *JSONObjDiff);

bool copyItem(cJSON *targetJSONObj, const char *targetKey, cJSON *sourceJSONObj,
      const char *sourceKey);

void printJSON(cJSON *object);
void printJSONUnformatted(cJSON *object);

string JSON2Str(cJSON *object);

class cJSONException: public std::exception
{
 public:
   cJSONException(cJSON* obj, string msg);
   ~cJSONException() throw () { };
   virtual const char* what() const throw();
 private:
   cJSON* m_obj;
   string m_msg;
};

cJSON* getKey(cJSON* obj, string key);
cJSON* getItem(cJSON* obj, string key, int index);
string asString(cJSON* obj);
double asDouble(cJSON* obj);
int asInt(cJSON* obj);

#endif
