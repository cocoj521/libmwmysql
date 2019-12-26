// MySqlHelper.h: interface for the CMySqlHelper class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __MYSQLHELPER_H__
#define __MYSQLHELPER_H__

#include "SQLTypes.h"
#include "SQLResult.h"
#include "SQLField.h"
#include "SQLValue.h"
#include <string>
#include <list>
#include <set>

namespace MySqlHelper {

using namespace MYSQL_SQLBASE_NSAMESPACE;
using namespace MYSQL_SQLTYPES_NSAMESPACE;
using namespace MYSQL_SQLVALUE_NSAMESPACE;
using namespace MYSQL_SQLRESULT_NSAMESPACE;
using namespace MYSQL_SQLFIELD_NSAMESPACE;
using namespace MYSQL_SQLRESULT_NSAMESPACE;

typedef std::list<SQLValueArray> ROW_LIST;//��:�洢������,ÿ���ֶξ�����
typedef std::map<std::string, CSQLField> COLUMN_MAP;//��:�洢��������ֵ�����е��±��Ӧ��ϵ
typedef std::list<std::string> COLUMN_NAME_LIST;
class CMySqlRecordSet
{
public:
    CMySqlRecordSet();
    virtual ~CMySqlRecordSet();
public://���RecordSet��Ϣ��
	//�������������±��Ӧ��ϵ
	void SetColumnMap(const COLUMN_MAP& column_map);
	//���һ������
	void AddOneRow(const SQLValueArray& one_row);
	//�����ܵļ�¼������
	size_t GetRecordsNum() const;
public://�����ṩ����	
	//������¼���ĵ�һ�м�¼
	void MoveFirst();
	//������¼������һ�м�¼
	void MoveNext();
	//�Ƿ��Ѿ��ǽ�β
	bool bEOF();
	//ʹ��������ȡ��ǰ���и��е�ֵ
	bool GetFieldValueFromCurrRow(const std::string& strFieldName, SQLVALUE& value); 
    //������˳���ȡ���д洢�������б�
    bool GetFieldNameList(COLUMN_NAME_LIST& fieldNameList);
private:
	int MIN2(int a, int b);
   	void to_upper(char *str);
private:
	ROW_LIST::iterator m_itCurr;
	COLUMN_MAP m_ColumnMap;
	ROW_LIST m_RowList;
};

enum EXECUTE_RESULT
{
	EXECUTE_SUCCES=0,				//�ɹ�
	EXECUTE_DBPOOL_NOT_ENOUGH,		//���ӳز�����
	EXECUTE_DBCONN_DISCON,			//���ӶϿ�
	EXECUTE_EXESQL_ERROR,			//ִ������쳣(���������??)
	EXECUTE_OTHER_ERROR=255			//������DB�쳣(��:��������ʱ�쳣)
};
class CMySqlHelper
{
public:
    CMySqlHelper();
    virtual ~CMySqlHelper();
public:
	//�������ӳز�ָ�����ӵ�����,�����������ִ�Сд
	//���һ������szConnAttr,���������������ӵ���������,��:set names 'utf8';set names 'gbk';SET CHARACTER SET 'utf8'��
	//����ֵtrue��ʾ�ɹ�,��false��ʾʧ��,����ʧ��ԭ���strErrMsg,strErrDesc�л�ȡ
	bool CreateConnPool(std::string& strErrMsg, std::string& strErrDesc, const char* szPoolName, const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort=3306, const char* szActiveSql="SELECT 1"/*�������*/, int nTimeOut=60, bool bUseUtf8=false, int nMinSize=2, int nMaxSize=5, const char* szConnAttr="");

	//�������ӳش�С
	void ReSizeConnPoolSize(const char* szPoolName, int nMinSize, int nMaxSize);
	
    //ִ����䲢���ؽ����
    //����ֵ0��ʾ�ɹ�,��0��ʾʧ��,����ʧ��ԭ���strErrMsg,strErrDesc�л�ȡ
    int ExecuteWithRecordSetRet(std::string& strErrMsg, std::string& strErrDesc, CMySqlRecordSet& Records, const char* szPoolName, const char *szProc, int nTimeOut=60);
	
    //ִ����䵫�����ؽ����
    //����ֵ0��ʾ�ɹ�,��0��ʾʧ��,����ʧ��ԭ���strErrMsg,strErrDesc�л�ȡ
    int ExecuteWithOutRecordSetRet(std::string& strErrMsg, std::string& strErrDesc, int& nAffectedRows, const char* szPoolName, const char *szProc, int nTimeOut=60);
};
}
#endif

