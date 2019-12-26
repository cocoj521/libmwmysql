#include "MySqlOperator.h"

using namespace MySqlOperator;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DBConnection::DBConnection()
{
    bValid = false;
}
DBConnection::~DBConnection()
{
	Close();
}
ConnectionPtr DBConnection::GetConnection()
{
    return (ConnectionPtr)sqlconn;
}
EnvironmentPtr DBConnection::GetEnvironment() const
{
    return sqlconn.GetEnvironment();
}
bool DBConnection::IsValid() const
{
    return bValid;
}
bool DBConnection::Open(const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort, unsigned int nTimeOut, bool bUseUtf8, const char* szConnAttr)
{
    return (bValid = sqlconn.Open(szHost,szDatabase,szUser,szPassword,nPort,nTimeOut,bUseUtf8,szConnAttr));
}
bool DBConnection::Close()
{
    bValid = false;
    sqlconn.Close();
    return bValid;
}

///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
RecordSet::RecordSet()
{
};
RecordSet::~RecordSet()
{
	/*rs.EndFetch();*/
};
//ִ�к������Ҫ��¼����ָ�뽫ֱ��������¼���ĵ�һ����¼��movefirst�������������κβ���
//ע�⣺��¼���Ĳ����ǵ���ģ�ֻ��ǰ��������������ز�������ʹ����movenext����ʹ��movefirst����Ч��,û��һ����
int RecordSet::Open(DBConnection& dbconn, const char *szSql, int nTimeOut, SQLSTRTYPE nType, bool bNeedRecordSet)
{
    rs.Init(dbconn.GetConnection(), dbconn.GetEnvironment());
    int nRet = rs.Query(szSql, nType, bNeedRecordSet);
    EndOfFile = bNeedRecordSet&&TryMoveFirst();
    return nRet;
}
void RecordSet::GetValue(const char* szItemName, SQLVALUE& value)
{
    int nIndex = rs.GetItemIndex(szItemName);
    value.value = values[nIndex];
}
//�����κβ���,open����������ɴ˹���
void RecordSet::MoveFirst()
{
    //EndOfFile = !rs.Fetch(values);
}
//movenext�󣬽��޷���movefirst
void RecordSet::MoveNext()
{
    EndOfFile = !rs.Fetch(values);
}
void RecordSet::Close()
{
    rs.EndFetch();
    EndOfFile = true;
}
//�����Ƶ�ͷ�����ж��Ƿ��м�¼��
bool RecordSet::TryMoveFirst()
{
    bool bRet = false;
    if (!rs.IsEmpty())
    {
        bRet = !rs.Fetch(values);
    }
    return bRet;
}

//��ȡ���м�¼������
const SQLValueArray& RecordSet::GetThisRowValue()
{
	return values;
}

//��ȡ���������±�Ķ�Ӧ��ϵ
const SQLFieldArray& RecordSet::GetColumnMap()
{
	return rs.GetColumnMap();
}

//��ȡӰ������
int RecordSet::GetAffectedRowCnt() const
{
	return rs.GetAffectedRowCnt();
}
