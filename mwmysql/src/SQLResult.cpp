#include "SQLException.h"
#include "SQLResult.h"
#include "SQLValue.h"
#include "SQLField.h"

MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::CSQLResult(MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr hConnect)
   : MYSQL_SQLBASE_NSAMESPACE::CSQLBase(hConnect),
     m_hRes(NULL),
     m_nAffectedRowCnt(0),
     m_hFields(NULL)
{

}

MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::CSQLResult():m_hRes(NULL),m_nAffectedRowCnt(0),m_hFields(NULL)
{
}


void MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::Init(MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr hConnect,MYSQL_SQLTYPES_NSAMESPACE::EnvironmentPtr pEnv)
{
    m_hConnect = hConnect;
}

MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::~CSQLResult()
{
   EndFetch(); // free the result set
}

int  MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::GetAffectedRowCnt() const
{
	return 	m_nAffectedRowCnt;
}

int MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::Query(const char* szSql, MYSQL_SQLTYPES_NSAMESPACE::SQLSTRTYPE nType, bool bNeedRetSet)
{
   int nRet = -1;
   ThrowExceptionIfNotOpen();

	 m_nAffectedRowCnt = 0;
	 
   // execute the query
   if(0 != mysql_query(m_hConnect, szSql))
   {
      // still error so throw thw exception
      ThrowException(mysql_errno(m_hConnect), mysql_error(m_hConnect));
   }
   else
   {
   		 //get affected rows
       m_nAffectedRowCnt = mysql_affected_rows(m_hConnect);
   }
   if (bNeedRetSet)
   {
       // initiates a result set retrieval
       // NOTE: mysql_use_result but not actually read the result set into
       // the client; mysql_fetch_row will be called Fetch function to retrieve
       // each row from result set

       m_hRes = mysql_store_result(m_hConnect);
       if(NULL == m_hRes)
       {
           ThrowException(mysql_errno(m_hConnect), mysql_error(m_hConnect));
       }

       do
       {
       }while(!mysql_next_result(m_hConnect));

       //get fields from result set
       m_hFields = mysql_fetch_fields(m_hRes);
       if(NULL == m_hFields)
       {
           ThrowException(mysql_errno(m_hConnect), mysql_error(m_hConnect));
       }
       //get fields to array
       GetFields();
       nRet = 0;
   }
   else
   {
        do
        {
            m_hRes = mysql_store_result(m_hConnect);
            EndFetch();
        }while(!mysql_next_result(m_hConnect));
        nRet = 0;
   }
   return nRet;
}

bool MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::Fetch(MYSQL_SQLVALUE_NSAMESPACE::SQLValueArray& arrValues, bool bFetchBLOB)
{
   arrValues.clear();
   bool bRet = false;
   ThrowExceptionIfNotOpen();
   //ThrowExceptionIfNoResultSet();
   if (NULL != m_hRes)
   {
       try
       {
           MYSQL_SQLTYPES_NSAMESPACE::HROW row = mysql_fetch_row(m_hRes);
           if(NULL != row)
           {
               //get fields num
               unsigned int nFieldsCount = mysql_num_fields(m_hRes);
               //get every field length
               unsigned long* lengths    = mysql_fetch_lengths(m_hRes);
               for(unsigned int nIndex = 0; nIndex < nFieldsCount; ++nIndex)
               {
                  MYSQL_SQLTYPES_NSAMESPACE::SQLType::e_Type eType   = static_cast<MYSQL_SQLTYPES_NSAMESPACE::SQLType::e_Type>(m_hFields[nIndex].type);
                  char* pData             = reinterpret_cast<char*>(row[nIndex]);
                  unsigned long nLength    = lengths[nIndex];
                  if((MYSQL_SQLTYPES_NSAMESPACE::SQLType::eBLOB == eType) && (!bFetchBLOB))
                  {
                     pData   = NULL;
                     nLength = 0;
                  }
                  MYSQL_SQLVALUE_NSAMESPACE::CSQLValue value(pData, nLength, eType);
                  //arrValues.SetAt(nIndex, value);
                  arrValues.insert(std::make_pair(nIndex, value));
               }
               bRet = true;
           }
           else
           {
               if(0 != mysql_errno(m_hConnect))//error
               {
                  ThrowException(mysql_errno(m_hConnect), mysql_error(m_hConnect));
               }
               else//no rows
               {
                  EndFetch();
               }
           }
       }
       catch (...)
       {
           bRet = false;
       }
   }
   return bRet;
}

void MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::EndFetch()
{
   if(NULL != m_hRes)
   {
      // frees the memory allocated for the result set by mysql_use_result
      mysql_free_result(m_hRes);
      m_hRes = NULL;
   }
}

unsigned int MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::GetFieldsCount() const
{
   ThrowExceptionIfNotOpen();
   ThrowExceptionIfNoResultSet();

   return mysql_num_fields(m_hRes);
}

void MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::GetFields()
{
   m_SQLFIELD.clear();
   ThrowExceptionIfNotOpen();
   ThrowExceptionIfNoResultSet();
   const unsigned int nFieldsCount = mysql_num_fields(m_hRes);
   // fill arrray
   for(unsigned int nIndex = 0; nIndex < nFieldsCount; ++nIndex)
   {
      to_upper(m_hFields[nIndex].name);
      MYSQL_SQLFIELD_NSAMESPACE::CSQLField field(m_hFields[nIndex].name, nIndex);
      m_SQLFIELD.insert(std::make_pair(field.GetName(), field));
   }
}

void MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::ThrowExceptionIfNoResultSet() const
{
    if(NULL == m_hRes)
    {
        std::string strErr = "No result set,Please call Query before Fetch.";
        ThrowException(0, strErr.c_str());
    }
}

int MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::GetItemIndex(const char* szField)
{
    char szBuf[64+1] = {0};
    memcpy(szBuf, szField, MIN2((int)(sizeof(szBuf)-1), (int)(strlen(szField))));
    to_upper(szBuf);
    int nIndex = m_SQLFIELD[szBuf].GetIndex();
    if (nIndex < 0)
    {
        std::string strErr;
        strErr = "\"";
        strErr += szBuf;
        strErr += "\"";
        strErr += " is not in this result set.";
        ThrowException(0, strErr.c_str());
    }
    return nIndex;
}

//返回列名与列下标的对应关系
const MYSQL_SQLFIELD_NSAMESPACE::SQLFieldArray& MYSQL_SQLRESULT_NSAMESPACE::CSQLResult::GetColumnMap()
{
	return m_SQLFIELD;
}

