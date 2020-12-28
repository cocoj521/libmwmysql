//////////////////////////////////////////////////////////////////////
#ifndef __MYSQLOPERATOR_H__
#define __MYSQLOPERATOR_H__

#include "SQLTypes.h"
#include "SQLConnection.h"
#include "SQLException.h"
#include "SQLResult.h"
#include "SQLField.h"
#include "SQLValue.h"
#include <string>
#include <list>

namespace MySqlOperator
{
using namespace MYSQL_SQLBASE_NSAMESPACE;
using namespace MYSQL_SQLTYPES_NSAMESPACE;
using namespace MYSQL_SQLCONNECTION_NSAMESPACE;
using namespace MYSQL_SQLVALUE_NSAMESPACE;
using namespace MYSQL_SQLRESULT_NSAMESPACE;
using namespace MYSQL_SQLEXCEPTION_NSAMESPACE;
using namespace MYSQL_SQLFIELD_NSAMESPACE;
using namespace MYSQL_SQLRESULT_NSAMESPACE;

class DBConnection
{
public:
    DBConnection();
    virtual ~DBConnection();
public:
    ConnectionPtr GetConnection();
    EnvironmentPtr GetEnvironment() const;
    bool IsValid() const;
    bool Open(const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort=3306, unsigned int nTimeOut=60, bool bUseUtf8=true, const char* szConnAttr="");
    bool Close();
private:
    bool bValid;
    CSQLConnection sqlconn;
};

//�����
class RecordSet
{
public:
    RecordSet();
    virtual ~RecordSet();
public:
    //ִ�к������Ҫ��¼����ָ�뽫ֱ��������¼���ĵ�һ����¼��movefirst�������������κβ���
    //ע�⣺��¼���Ĳ����ǵ���ģ�ֻ��ǰ��������������ز�������ʹ����movenext����ʹ��movefirst����Ч��,û��һ����
    int Open(DBConnection& dbconn, const char *szSql, int nTimeOut=60, SQLSTRTYPE nType=SQLSTRTYPE_NORMAL, bool bNeedRecordSet=true);
	
    void GetValue(const char* szItemName, SQLVALUE& value);
	
    //�����κβ���,open����������ɴ˹���
    void MoveFirst();
	
    //movenext�󣬽��޷���movefirst
    void MoveNext();
	
    void Close();

	//��ȡ���м�¼������
	const SQLValueArray& GetThisRowValue();

	//��ȡ���������±�Ķ�Ӧ��ϵ
	const SQLFieldArray& GetColumnMap();

	//��ȡӰ������
	int	GetAffectedRowCnt() const;
private:
    //�����Ƶ�ͷ�����ж��Ƿ��м�¼��
    bool TryMoveFirst();
public:
    bool EndOfFile;
private:
    SQLValueArray values;
    CSQLResult rs;
};
}
#endif