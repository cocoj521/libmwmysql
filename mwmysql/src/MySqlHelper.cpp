
// MySqlHelper.h: interface for the CAccessMysql class.
//
//////////////////////////////////////////////////////////////////////

#include "MySqlHelper.h"
#include "MySqlConnPool.h"
#include "MySqlOperator.h"

using namespace MySqlHelper; 
using namespace MySqlConnPool;
using namespace MySqlOperator;

//////////////////////////////////////////////////////////////////////////////////

CMySqlRecordSet::CMySqlRecordSet()
{

}

CMySqlRecordSet::~CMySqlRecordSet()
{

}

int CMySqlRecordSet::MIN2(int a, int b)
{
	return a>b ? b : a;
}

void CMySqlRecordSet::to_upper(char *str)
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

//������¼���ĵ�һ�м�¼
void CMySqlRecordSet::MoveFirst()
{
	m_itCurr = m_RowList.begin();
}

//������¼������һ�м�¼
void CMySqlRecordSet::MoveNext()
{
	if (m_itCurr != m_RowList.end())
	{
		++m_itCurr;
	}
}

//�Ƿ��Ѿ��ǽ�β
bool CMySqlRecordSet::bEOF()
{
	return (m_itCurr == m_RowList.end());
}

size_t CMySqlRecordSet::GetRecordsNum() const
{
	return m_RowList.size();
}

//�������������±��Ӧ��ϵ
void CMySqlRecordSet::SetColumnMap(const COLUMN_MAP& column_map)
{
	m_ColumnMap = column_map;
}

//���һ������
void CMySqlRecordSet::AddOneRow(const SQLValueArray& one_row)
{
	m_RowList.push_back(one_row);
}

//ʹ��������ȡ��ǰ���и��е�ֵ
bool CMySqlRecordSet::GetFieldValueFromCurrRow(const std::string& strFieldName, SQLVALUE& value)
{
	bool bRet = false;

	char szBuf[64+1] = {0};
    memcpy(szBuf, strFieldName.c_str(), MIN2((int)(sizeof(szBuf)-1), (int)(strFieldName.length())));
    to_upper(szBuf);
	COLUMN_MAP::iterator it = m_ColumnMap.find(std::string(szBuf));
	if (it != m_ColumnMap.end())
	{
		int nIndex = it->second.GetIndex();
		SQLValueArray::iterator itValue = (*m_itCurr).find(nIndex);
		if (itValue != (*m_itCurr).end())
		{
			value.value = itValue->second;
			bRet = true;
		}
	}
	
	return bRet;
}

// ����������
// Defining a lambda function to compare two pairs. It will compare two pairs using second field
bool compFunctor(std::pair<std::string, CSQLField> elem1, std::pair<std::string, CSQLField> elem2)
{
    return elem1.second.GetIndex() < elem2.second.GetIndex();
}

//������˳���ȡ���д洢�������б�
bool CMySqlRecordSet::GetFieldNameList(COLUMN_NAME_LIST& fieldNameList)
{
    bool bRet = false;
    if (!m_ColumnMap.empty())
    {
        // Declaring the type of Predicate that accepts 2 pairs and return a bool
        typedef bool(*Comparator)(std::pair<std::string, CSQLField>, std::pair<std::string, CSQLField>);

        // Declaring a set that will store the pairs using above comparision logic
        std::set<std::pair<std::string, CSQLField>, Comparator> setOfAccouts(
            m_ColumnMap.begin(), m_ColumnMap.end(), compFunctor);

        // Iterate over a set using range base for loop
        // It will display the items in sorted order of values
        std::set<std::pair<std::string, CSQLField>, Comparator>::iterator element;
        for (element = setOfAccouts.begin(); element != setOfAccouts.end(); element++)
        {
            //����
            fieldNameList.push_back(element->second.GetName());
        }
        return true;
    }
    return bRet;
}


//////////////////////////////////////////////////////////////////////////////////
CMySqlHelper::CMySqlHelper()
{
}

CMySqlHelper::~CMySqlHelper()
{
    
}

//�������ӳز�ָ�����ӵ�����,�����������ִ�Сд
bool CMySqlHelper::CreateConnPool(std::string& strErrMsg, std::string& strErrDesc, const char* szPoolName, const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort, const char* szActiveSql, int nTimeOut, bool bUseUtf8,int nMinSize, int nMaxSize, const char* szConnAttr)
{
	return CMySqlConnPoolMgr::GetInstance().CreateConnPool(strErrMsg, strErrDesc, szPoolName,szHost,szDatabase,szUser,szPassword,nPort,szActiveSql,nTimeOut,bUseUtf8,nMinSize,nMaxSize,szConnAttr);
}

//�������ӳش�С
void CMySqlHelper::ReSizeConnPoolSize(const char* szPoolName, int nMinSize, int nMaxSize)
{
	return CMySqlConnPoolMgr::GetInstance().ResetConnPoolSize(szPoolName, nMinSize, nMaxSize);
}

//ִ����䲢���ؽ����
int CMySqlHelper::ExecuteWithRecordSetRet(std::string& strErrMsg, std::string& strErrDesc, CMySqlRecordSet& Records, const char* szPoolName, const char *szSQL, int nTimeOut)
{
	int nRet = EXECUTE_DBCONN_DISCON;
	
	SafeConnPool connpool;

    try
    {
		DBConnection* pConn = connpool.GetConnEx(szPoolName);
		if (pConn)
		{
	        if (pConn->IsValid())
	        {
	        	RecordSet rs;
	            if (0 == rs.Open(*pConn, szSQL, nTimeOut, SQLSTRTYPE_NORMAL, true))
	            {
					//�������������±�ӳ��
					Records.SetColumnMap(rs.GetColumnMap());
					
					//..............����,����
					while (!rs.EndOfFile)
					{
						Records.AddOneRow(rs.GetThisRowValue());	
						rs.MoveNext();
					}

					nRet = EXECUTE_SUCCES;
					strErrMsg = "SUCCESS";
					strErrDesc = "EXECUTE SUCCESS";
		        }
	        }
	        else
	        {
	            nRet = EXECUTE_DBCONN_DISCON;
				strErrMsg = "ERROR";
				strErrDesc = "Link has gone away.";
				
				connpool.IsConnValid();
	        }
		}
		else
		{
			nRet = EXECUTE_DBPOOL_NOT_ENOUGH;
			strErrMsg = "WARN";
			strErrDesc = "ConnPool is not enough or no available links.";
		}
    }
    catch (CSQLException e)
    {
        nRet = EXECUTE_EXESQL_ERROR;
		strErrMsg = e.ErrorMessage();
		strErrDesc = e.Description();
		
		connpool.IsConnValid();
    }
    catch (...)
    {
        nRet = EXECUTE_OTHER_ERROR;

        strErrMsg  = "ERROR";
		strErrDesc = "Other error.";
    }
	return nRet;
}

//ִ����䵫�����ؽ����
int CMySqlHelper::ExecuteWithOutRecordSetRet(std::string& strErrMsg, std::string& strErrDesc, int& nAffectedRows, const char* szPoolName, const char *szSQL, int nTimeOut)
{
	int nRet = EXECUTE_DBCONN_DISCON;

	SafeConnPool connpool;

    try
    {
		DBConnection* pConn = connpool.GetConnEx(szPoolName);
		if (pConn)
		{
	        if (pConn->IsValid())
	        {
	        	RecordSet rs;
	            if (0 == rs.Open(*pConn, szSQL, nTimeOut, SQLSTRTYPE_NORMAL, false))	
	            {
	            	//��ȡӰ������
	            	nAffectedRows = rs.GetAffectedRowCnt();
					
	            	nRet = EXECUTE_SUCCES;
					strErrMsg = "SUCCESS";
					strErrDesc = "EXECUTE SUCCESS";
	            }
	        }
	        else
	        {
	            nRet = EXECUTE_DBCONN_DISCON;
				strErrMsg = "ERROR";
				strErrDesc = "Link has gone away.";
				
				connpool.IsConnValid();
	        }
		}
		else 
		{
			nRet = EXECUTE_DBPOOL_NOT_ENOUGH;
			strErrMsg = "WARN";
			strErrDesc = "ConnPool is not enough or no available links.";
		}
    }
    catch (CSQLException e)
    {
        nRet = EXECUTE_EXESQL_ERROR;
		strErrMsg = e.ErrorMessage();
		strErrDesc = e.Description();

		connpool.IsConnValid();
    }
    catch (...)
    {
        nRet = EXECUTE_OTHER_ERROR;

        strErrMsg  = "ERROR";
		strErrDesc = "Other error.";
    }
    return nRet;
}

