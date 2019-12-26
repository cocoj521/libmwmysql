#ifndef __MYSQL_SQLRESULT_H__
#define __MYSQL_SQLRESULT_H__

#include "SQLBase.h"
#include "SQLValue.h"
#include "SQLField.h"

namespace MYSQL_SQLRESULT_NSAMESPACE {
//using namespace MYSQL_SQLTYPES_NSAMESPACE;
class CSQLResult : public MYSQL_SQLBASE_NSAMESPACE::CSQLBase
{
public:
   CSQLResult(MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr hConnect);
   void Init(MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr hConnect,MYSQL_SQLTYPES_NSAMESPACE::EnvironmentPtr pEnv);
   CSQLResult();
   virtual ~CSQLResult();
protected:
   MYSQL_SQLTYPES_NSAMESPACE::RecordSetPtr     m_hRes;
   MYSQL_SQLTYPES_NSAMESPACE::HFIELD   m_hFields;
   MYSQL_SQLFIELD_NSAMESPACE::SQLFieldArray m_SQLFIELD;
   int m_nAffectedRowCnt;
public:
   bool IsEmpty() {return NULL==m_hRes;};
   int  GetItemIndex(const char* szField);
   int  GetAffectedRowCnt() const;
   //nType 0:表示szSql是普通SQL语句(select,insert,update,delete) 1:表示szSql是存储过程
   //nNeedRetSet表示szSql是否需要返回记录集
   int  Query(const char* szSql, MYSQL_SQLTYPES_NSAMESPACE::SQLSTRTYPE nType=MYSQL_SQLTYPES_NSAMESPACE::SQLSTRTYPE_NORMAL, bool bNeedRetSet=true);
   bool Fetch(MYSQL_SQLVALUE_NSAMESPACE::SQLValueArray& arrValues, bool bFetchBLOB = true);
   void EndFetch();
   //返回列名与列下标的对应关系
   const MYSQL_SQLFIELD_NSAMESPACE::SQLFieldArray& GetColumnMap();
protected:
   unsigned int GetFieldsCount() const;
   void GetFields();
   void ThrowExceptionIfNoResultSet() const;

   int MIN2(int a, int b)
   {
       return a>b ? b : a;
   }

   void to_upper(char *str)
   {
       int  i = 0;
       while(str[i] != '\0')
       {
           if((str[i] >= 'a') && (str[i] <= 'z'))
           {
               str[i] -= 32;
           }
           ++i;
       }
   }
};
}

#endif
