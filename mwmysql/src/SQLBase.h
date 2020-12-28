#ifndef __MYSQL_SQLBASE_H__
#define __MYSQL_SQLBASE_H__

#include "SQLTypes.h"
#include "SQLValue.h"
#include "SQLField.h"

namespace MYSQL_SQLBASE_NSAMESPACE {
//using namespace MYSQL_SQLTYPES_NSAMESPACE;
class CSQLBase
{
public:
   CSQLBase(MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr hConnect) : m_hConnect(hConnect)
   {
   };
   CSQLBase()
   {
        m_hConnect = NULL;
   };
   virtual ~CSQLBase();
protected:
   MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr m_hConnect;
protected:
   void ThrowException(int nErCode, const char* szErr) const;
   void ThrowExceptionIfNotOpen() const;
};
}

#endif
