#ifndef __MYSQL_SQLCONNECTION_H__
#define __MYSQL_SQLCONNECTION_H__

#include "SQLBase.h"

namespace MYSQL_SQLCONNECTION_NSAMESPACE {
//using namespace MYSQL_SQLTYPES_NSAMESPACE;
class CSQLConnection : public MYSQL_SQLBASE_NSAMESPACE::CSQLBase
{
public:
    CSQLConnection();
    virtual ~CSQLConnection();
public:
    bool Open(const char* szHost,
            const char* szDatabase,
            const char* szUser,
            const char* szPassword,
            unsigned int nPort = 3306,
            unsigned int nTimeOut=60,
            bool bUseUtf8=false,
            const char* szConnAttr="");

    void Close();

    bool IsOpen();

    operator MYSQL_SQLTYPES_NSAMESPACE::ConnectionPtr() const
    {
        return m_hConnect;
    }
    MYSQL_SQLTYPES_NSAMESPACE::EnvironmentPtr GetEnvironment() const
    {
        return NULL;
    }
};
}

#endif
