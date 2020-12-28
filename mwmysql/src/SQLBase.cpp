#include "SQLException.h"
#include "SQLBase.h"

MYSQL_SQLBASE_NSAMESPACE::CSQLBase::~CSQLBase()
{

}

void MYSQL_SQLBASE_NSAMESPACE::CSQLBase::ThrowException(int nErCode, const char* szErr) const
{
    MYSQL_SQLEXCEPTION_NSAMESPACE::CSQLException e(nErCode, szErr);
    throw e;
}

void MYSQL_SQLBASE_NSAMESPACE::CSQLBase::ThrowExceptionIfNotOpen() const
{
   if(NULL == m_hConnect)
   {
      std::string strErr = "mysql_init error,no database is opened.";
      MYSQL_SQLEXCEPTION_NSAMESPACE::CSQLException e(0, strErr.c_str());
      throw e;
   }
}
