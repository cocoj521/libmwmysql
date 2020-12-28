#ifndef __MYSQL_SQLFIELD_H__
#define __MYSQL_SQLFIELD_H__

#include "SQLTypes.h"

namespace MYSQL_SQLFIELD_NSAMESPACE {
class CSQLField
{
public:
    CSQLField();
    CSQLField(const std::string& strFieldName, int nIndex);
    CSQLField(const CSQLField& rField);
protected:
    std::string m_strName;
    int  m_nIndex;
public:
    CSQLField& operator=(const CSQLField& rField);
    inline int  GetIndex() const
    {
        return m_nIndex;
    }
    inline std::string GetName() const
    {
        return m_strName;
    }
protected:
    void Copy(const CSQLField& rField);
};
typedef std::map<std::string, CSQLField> SQLFieldArray;
}

#endif
