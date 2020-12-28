/******************************************************************************
   File:        SQLTypes.h
   Contents:    Type declarations
******************************************************************************/
#ifndef __MYSQL_SQLTYPES_H__
#define __MYSQL_SQLTYPES_H__

#pragma warning (disable:4103)
#pragma warning (disable:4786)

#if (defined  MW_WINDOWS) || (defined  WIN32)
#include <winsock2.h>
#endif
#include "mysql.h"
#include "mysqld_error.h"
#include <map>
#include <string>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#if !((defined  MW_WINDOWS) || (defined  WIN32))

#ifndef __int64
#define __int64 long long
#endif

#ifndef INT64
#define INT64 __int64
#endif

#include <stdio.h>
#include <string.h>

#endif

#define MYSQL_SQLTYPES_NSAMESPACE           mysql_sqltypes_namespace
namespace MYSQL_SQLTYPES_NSAMESPACE {

// forward declarations to avoid including mysql headers
typedef struct st_mysql       MYSQL;
typedef struct st_mysql_res   MYSQL_RES;
typedef struct st_mysql_field MYSQL_FIELD;
typedef char **MYSQL_ROW;

// declare handle types;
// they  are aliases of types defined in mysql headers
// but a litle bit more readable for a MFC programmer
typedef MYSQL*       ConnectionPtr;
typedef MYSQL_RES*   RecordSetPtr;
typedef MYSQL_FIELD* HFIELD;
typedef MYSQL_ROW    HROW;
typedef void*        EnvironmentPtr;

#define MYSQL_SQLVALUE_NSAMESPACE          mysql_sqlvalue_namespace
#define MYSQL_SQLRESULT_NSAMESPACE         mysql_sqlresult_namespace
#define MYSQL_SQLFIELD_NSAMESPACE          mysql_sqlfield_namespace
#define MYSQL_SQLEXCEPTION_NSAMESPACE      mysql_sqlexception_namespace
#define MYSQL_SQLCONNECTION_NSAMESPACE     mysql_sqlconnection_namespace
#define MYSQL_SQLBASE_NSAMESPACE           mysql_sqlbase_namespace

namespace SQLType
{
   enum e_Type
   {                    // SQL TYPE
    //-------------------------------
      eDecimal = 0,     // DECIMAL
      eTiny,            // TINYINT
      eShort,           // SMALLINT
      eLong,            // INT
      eFloat,           // FLOAT
      eDouble,          // DOUBLE
      eTimestamp = 7,   // TIMESTAMP
      eLonglong,        // BIGINT
      eDate = 10,       // DATE
      eTime,            // TIME
      eDatetime,        // DATETIME
      eYear,            // YEAR
      eBLOB = 252,      // BLOB, TEXT, TINYBLOB, TINYTEXT, etc.
      eString = 253,    // VARCHAR, CHAR
      eUnknown = 1000
   };
};

//SQL”Ôæ‰¿‡–Õ
enum SQLSTRTYPE
{
    SQLSTRTYPE_NORMAL=0,    //SELECT INSERT DELETE UPDATE
    SQLSTRTYPE_PROC,        //CALL PROC();
    SQLSTRTYPE_UNKNOWN
};
}

#endif
