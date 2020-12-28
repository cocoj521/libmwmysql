#include "SQLField.h"

MYSQL_SQLFIELD_NSAMESPACE::CSQLField::CSQLField(const std::string& strFieldName, int nIndex)
{
   m_strName = strFieldName;
   m_nIndex  = nIndex;
}

void MYSQL_SQLFIELD_NSAMESPACE::CSQLField::Copy(const MYSQL_SQLFIELD_NSAMESPACE::CSQLField& rField)
{
   m_strName = rField.m_strName;
   m_nIndex  = rField.m_nIndex;
}

MYSQL_SQLFIELD_NSAMESPACE::CSQLField::CSQLField()
{

}

MYSQL_SQLFIELD_NSAMESPACE::CSQLField::CSQLField(const MYSQL_SQLFIELD_NSAMESPACE::CSQLField& rField)
{
    Copy(rField);
}

MYSQL_SQLFIELD_NSAMESPACE::CSQLField& MYSQL_SQLFIELD_NSAMESPACE::CSQLField::operator=(const MYSQL_SQLFIELD_NSAMESPACE::CSQLField& rField)
{
    if(this != &rField)
    {
        Copy(rField);
    }
    return *this;
}
