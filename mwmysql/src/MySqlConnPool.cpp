// MySqlConnPool.cpp: implementation of the CMySqlConnPool class and CMySqlConnPoolMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "MySqlHelper.h"
#include "MySqlConnPool.h"
#include "unistd.h"
#include <sys/prctl.h>

//MySqlConnPool
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

////���ӳ���������ȱ��:���Ӽ����ⲿ�ֻ���Ϊִ�����ݿ����,ռ����......
using namespace MySqlConnPool;

CMySqlConnPool::CMySqlConnPool()
{
	m_nMinSize = 2;
	m_nMaxSize = 5;
	m_strActiveSql = "SELECT 1";
	m_strConnAttr = "";
}

CMySqlConnPool::~CMySqlConnPool()
{
	UnInitPool();
}

//��ʼ�����ӳ�,������nMinSize������,���������Ӷ�����ɹ�ʱ����true.
bool CMySqlConnPool::InitPool(std::string& strErrMsg, std::string& strErrDesc, const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort, const char* szActiveSql, int nTimeOut, bool bUseUtf8, int nMinSize, int nMaxSize, const char* szConnAttr)
{
	bool bRet = true;
	m_strHost = szHost;m_strDataBase = szDatabase;m_strUser = szUser;m_strPwd = szPassword;m_nPort = nPort;m_nTimeOut = nTimeOut;m_bUseUtf8 = bUseUtf8;
	if (NULL != szActiveSql && '\0' != szActiveSql[0])
	{
		m_strActiveSql = szActiveSql;
	}
	if (NULL != szConnAttr && '\0' != szConnAttr[0])
	{
		m_strConnAttr = szConnAttr;
	}
	
	m_nMinSize = nMinSize;
	m_nMaxSize = nMaxSize;
	m_nMinSize<MIN_DBCONN_SIZE?m_nMinSize=MIN_DBCONN_SIZE:1;
	m_nMinSize>MAX_DBCONN_SIZE?m_nMinSize=MAX_DBCONN_SIZE:1;
	m_nMaxSize<MIN_DBCONN_SIZE?m_nMaxSize=MIN_DBCONN_SIZE:1;
	m_nMaxSize>MAX_DBCONN_SIZE?m_nMaxSize=MAX_DBCONN_SIZE:1;
	for (int i = 0; i < m_nMinSize; ++i)
	{
		DBCONN conn;
		if (ApplayNewConn(conn, strErrMsg, strErrDesc))
		{
			m_Idle.push_back(conn);
		}
		else
		{
			bRet = false;
			break;
		}
	}
	return bRet;
}

//��ȡ���ӳ���Ϣ
void CMySqlConnPool::GetConnPoolInfo(int& nMinSize, int& nMaxSize, int& nIdle, int& nBusy)
{
	SafeLock safelock(&m_csLock);
	nMinSize = m_nMinSize;
	nMaxSize = m_nMaxSize;
	nIdle    = m_Idle.size();
	nBusy	 = m_InUse.size();
}

//�������ӳش�С
void CMySqlConnPool::ResetConnPoolSize(int nMinSize, int nMaxSize)
{
	m_nMinSize = nMinSize;
	m_nMaxSize = nMaxSize;
	m_nMinSize<MIN_DBCONN_SIZE?m_nMinSize=MIN_DBCONN_SIZE:1;
	m_nMinSize>MAX_DBCONN_SIZE?m_nMinSize=MAX_DBCONN_SIZE:1;
	m_nMaxSize<MIN_DBCONN_SIZE?m_nMaxSize=MIN_DBCONN_SIZE:1;
	m_nMaxSize>MAX_DBCONN_SIZE?m_nMaxSize=MAX_DBCONN_SIZE:1;
}

//�������ӳ�,���ͷ���������
void CMySqlConnPool::UnInitPool()
{
	SafeLock safelock(&m_csLock);
	std::list<DBCONN>::iterator it = m_Idle.begin();
	for (it; it != m_Idle.end(); ++it)
	{
		delete it->pConn;
		it->pConn = NULL;
	}
	m_Idle.clear();
	std::map<DBConnection*, int>::iterator it2 = m_InUse.begin();
	for (it2; it2 != m_InUse.end(); ++it2)
	{
		delete it2->first;
	}
	m_InUse.clear();
}

//����������
bool CMySqlConnPool::ApplayNewConn(DBCONN& conn, std::string& strErrMsg, std::string& strErrDesc)
{
	bool bRet = false;
	DBConnection* pConn = NULL;
	try
	{
		pConn = conn.pConn = new DBConnection();
		if (NULL != conn.pConn)
		{
			if(conn.pConn->Open(m_strHost.c_str(), m_strDataBase.c_str(), m_strUser.c_str(), m_strPwd.c_str(), m_nPort, m_nTimeOut, m_bUseUtf8, m_strConnAttr.c_str()))
			{
				time(&conn.tLastActive);
				bRet = true;
			}
		}
	}
	catch (CSQLException e)
	{
		strErrMsg  = e.ErrorMessage();
		strErrDesc = e.Description();
	}
	catch (...)
	{
		strErrMsg = strErrDesc = "Other Error.";
	}
	if (!bRet && pConn)
	{
		delete pConn;
		pConn = NULL;
	}
	return bRet;
}


//�ӳ����л�ȡһ������.��ָ�����ȴ�ʱ��(��λ:s),��ʱ���Զ�����NULL.���ȴ�ʱ����СֵΪ5s
DBConnection* CMySqlConnPool::GetConnFromPool(int nMaxTimeWait)
{
	nMaxTimeWait < 5 ? nMaxTimeWait = 5 : 1;
	DBConnection *pCon = NULL;
	//�����ж������Ƿ��п��õ�,������ֱ�ӷ���,��û��,�鿴�Ƿ񳬹��������ֵ,��û����������һ���µķ���,��������ȴ�
	SafeLock safelock(&m_csLock);
	if (!m_Idle.empty())
	{
		std::list<DBCONN>::iterator it = m_Idle.begin();
		for (it; it != m_Idle.end();)
		{
			//2013-06-28
			//ȡ����ʱ��ÿ�ζ�active,�������ݿ��������
			if (it->pConn->IsValid()) 
			{
				time(&it->tLastActive);
				m_InUse.insert(std::make_pair(it->pConn, 0));
				pCon = it->pConn;
				m_Idle.erase(it++);
				break;
			}
			else
			{
				it->pConn->Close();
				delete it->pConn;
				m_Idle.erase(it++);
				continue;
			}
		}
	}
	//���ж���Ϊ��,��������û�г������ֵ,����������һ������
	else if (m_Idle.empty() && m_InUse.size() < m_nMaxSize)
	{
		DBCONN conn;
		std::string strErrMsg;
		std::string strErrDesc;
		if (ApplayNewConn(conn, strErrMsg, strErrDesc))
		{
			pCon = conn.pConn;
			m_InUse.insert(std::make_pair(pCon, 0));
		}
	}

	return pCon;
}

//��������
bool CMySqlConnPool::RecycleConn(DBConnection* pConn)
{
	bool bRes = true;
	if (NULL != pConn)
	{
		SafeLock safelock(&m_csLock);
		DBCONN conn;
		conn.pConn = pConn;
		time(&conn.tLastActive);
		//����ǰ��������Ƿ����,�������������,Ȼ�����ж��Ƿ�С����Сֵ,��С����������һ��
		if (m_InUse.find(pConn) != m_InUse.end())
		{
			//2013-06-28
			//ֱ�ӻ���,����active,���ٲ�������
			if (!pConn->IsValid())
			{
				//���Ӳ�������ɾ����
				//pConn->Close();
				//delete pConn;
				m_InUse.erase(pConn);
				bRes = false;
			}
			else
			{
				m_Idle.push_back(conn);
				m_InUse.erase(pConn);
			}
		}
	}

	return bRes;
}


//����ȷ��Ok�����Ӽ������LIST
void CMySqlConnPool::AddIdleList(DBCONN dbcon)
{
	SafeLock lock(&m_csLock);
	m_Idle.push_back(dbcon);
}

//��ȷ��Ϊ�Ͽ������Ӽ�������LIST
void CMySqlConnPool::AddRecyleList(DBCONN dbcon)
{
	SafeLock lock(&m_csRecyleLock);
	m_LRecyle.push_back(dbcon);
}

//һ����ȡ�����еĶϿ�������
void CMySqlConnPool::GetAllOFFlineCon(std::list<DBCONN>& listOffLine)
{
	listOffLine.clear();
	SafeLock lock(&m_csRecyleLock);
	listOffLine.swap(m_LRecyle);
}

//�Զ��ߵ����ӽ�����������
void CMySqlConnPool::CheckOFFLineDBCon()
{
	//һ����ȡ�����жϿ�������
	std::list<DBCONN> LOffLine;
	GetAllOFFlineCon(LOffLine);

	std::list<DBCONN>::iterator it ;
	for (it = LOffLine.begin();it != LOffLine.end();)
	{
		DBCONN& dbcon = *it;
		//��⵱ǰ�����ܷ�����ʹ��
		if (TestConn(dbcon.pConn))
		{
			//���뷵������
			AddIdleList(*it);

			//ɾ���������ϵ�����
			LOffLine.erase(it++);
		}
		else
		{
			//���¼ӻضϿ�����
			AddRecyleList(*it);

			//��һ�������Ӽ�⡣
			++it;
		}
	}
}


//��Ҫ��������Ƿ���Ч
bool CMySqlConnPool::IsConnValid(DBConnection* pConn)
{
	return TestConn(pConn);
}

//���������Ƿ����
bool CMySqlConnPool::TestConn(DBConnection* pConn)
{
	bool bRet = false;
	try
	{
		if (NULL != pConn)
		{
			bool bValid = pConn->IsValid();
			//��������
			if (bValid)
			{
				RecordSet rs;
				//����
				if (0 == rs.Open(*pConn,m_strActiveSql.c_str(),60,SQLSTRTYPE_NORMAL,false))
				{
					bRet = true;
				}
			}
			else
			{
				bRet = pConn->Open(m_strHost.c_str(), m_strDataBase.c_str(), m_strUser.c_str(), m_strPwd.c_str(), m_nPort, m_nTimeOut, m_bUseUtf8, m_strConnAttr.c_str());
			}
		}
	}
	catch (...)
	{
		bRet = false;
	}
	if (pConn && !bRet)
	{
		pConn->Close();
	}
	return bRet;
}

//���ʱ����е�����(����ǰ���������������ӳ����ޣ��������رտ�������)
void CMySqlConnPool::ActiveIdleConn()
{
	SafeLock safelock(&m_csLock);
	std::list<DBCONN>::iterator it = m_Idle.begin();
	for (it; it != m_Idle.end(); )
	{
		//�п��еĲ��������������������ޣ��������ر�����
		if (m_InUse.size()+m_Idle.size() > m_nMaxSize)
		{
			it->pConn->Close();
			delete it->pConn;
			m_Idle.erase(it++);
		}
		else
		{
			//�����ӽ��м���
			if (time(NULL) - it->tLastActive > MAX_DBCONN_ACTIVE_TM && 0 != TestConn(it->pConn))
			{
				delete it->pConn;
				m_Idle.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}
}

///CMySqlConnPoolMgr
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMySqlConnPoolMgr& CMySqlConnPoolMgr::GetInstance()
{
    static CMySqlConnPoolMgr instance;
    return instance;
}

CMySqlConnPoolMgr::CMySqlConnPoolMgr()
{
	m_bExit = false;
	pthread_t ntid;
	int err = pthread_create(&ntid, NULL, ThreadCheckConnPool, this);
	pthread_detach(ntid);
}

CMySqlConnPoolMgr::~CMySqlConnPoolMgr()
{
	//֪ͨ�߳��˳�
	m_bExit = true;

	while (m_nThrCnt > 0)
	{
		usleep(100*1000);
	}

	SafeLock safelock(&m_csLock);
	std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.begin();
	for (it; it != m_dbpool.end(); ++it)
	{
		it->second->UnInitPool();
		delete it->second;
		it->second = NULL;
	}
	m_dbpool.clear();
}

//�������ӳز�ָ�����ӵ�����,��ʹ��""��Ϊ��������,�򴴽�Ĭ�ϳ���,�����������ִ�Сд
bool CMySqlConnPoolMgr::CreateConnPool(std::string& strErrMsg, std::string& strErrDesc, const char* szPoolName, const char* szHost, const char* szDatabase, const char* szUser, const char* szPassword, unsigned int nPort, const char* szActiveSql, int nTimeOut, bool bUseUtf8, int nMinSize, int nMaxSize, const char* szConnAttr)
{
	bool bRet = false;
	CMySqlConnPool *pPool = new CMySqlConnPool();
	if (NULL != pPool)
	{
		if (pPool->InitPool(strErrMsg, strErrDesc, szHost, szDatabase, szUser, szPassword, nPort, szActiveSql, nTimeOut, bUseUtf8, nMinSize, nMaxSize, szConnAttr))
		{
			SafeLock safelock(&m_csLock);
			m_dbpool.insert(std::make_pair(szPoolName, pPool));
			bRet = true;
		}
		else
		{
			delete pPool;//AAAAAAAAAAAAAAAAA
			pPool = NULL;
		}
	}
	return bRet;
}


//��ָ���ĳ����л�ȡһ������.��ָ�����ȴ�ʱ��(��λ:s),��ʱ���Զ�����NULL.���ȴ�ʱ����СֵΪ5s
DBConnection * CMySqlConnPoolMgr::GetConnFromPool(const char* szPoolName, int nMaxTimeWait)
{
	DBConnection* pCon = NULL;
	CMySqlConnPool * pPool = NULL;
	{
		SafeLock safelock(&m_csLock);
		std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
		if (it != m_dbpool.end())
		{
			pPool = it->second;
		}
	}
	if (NULL != pPool)
	{
		pCon = pPool->GetConnFromPool(nMaxTimeWait);
	}
	time_t tBegin = time(NULL);
	while (NULL == pCon && ((time(NULL)-tBegin) >= nMaxTimeWait || (time(NULL)-tBegin) < 0))
	{
		{
			SafeLock safelock(&m_csLock);
			std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
			if (it != m_dbpool.end())
			{
				pPool = it->second;
			}
		}

		if (NULL != pPool)
		{
			pCon = pPool->GetConnFromPool(nMaxTimeWait);
			if (NULL != pCon) break;
		}
		usleep(1000);
	}

	return pCon;
}

//�����ӻ�����ָ��������
void CMySqlConnPoolMgr::RecycleConn(const char* szPoolName, DBConnection* pConn)
{
	CMySqlConnPool * pPool = NULL;
	{
		SafeLock safelock(&m_csLock);
		std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
		if (it != m_dbpool.end())
		{
			pPool = it->second;
		}
	}

	if (NULL != pPool)
	{
		//���ճɹ����ʹ���е���
		//��ת�Ƶ��˿�������֮��
		if(!pPool->RecycleConn(pConn))
		{
			//����ʧ�����ʹ���е�����ת��
			//����ʱ����������֮��
			DBCONN conn;
			conn.pConn = pConn;
			time(&conn.tLastActive);
			pPool->AddRecyleList(conn);
		}
	}
}

//��Ҫ��������Ƿ���Ч
bool CMySqlConnPoolMgr::IsConnValid(const char* szPoolName, DBConnection* pConn)
{
	bool bRet = false;
	CMySqlConnPool * pPool = NULL;
	{
		SafeLock safelock(&m_csLock);
		std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
		if (it != m_dbpool.end())
		{
			pPool = it->second;
		}
	}
	if (pPool)
	{
		bRet = pPool->TestConn(pConn);
		//�����ݿ����ӶϿ�,����ʱһ��ʱ��
		if (!bRet) usleep(1000); //1ms
	}
	return bRet;
}

//��ȡ���ӳ���Ϣ
void CMySqlConnPoolMgr::GetConnPoolInfo(const char* szPoolName, int& nMinSize, int& nMaxSize, int& nIdle, int& nBusy)
{
	CMySqlConnPool * pPool = NULL;
	{
		SafeLock safelock(&m_csLock);
		std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
		if (it != m_dbpool.end())
		{
			pPool = it->second;
		}
	}
	
	if (NULL != pPool)
	{
		pPool->GetConnPoolInfo(nMinSize, nMaxSize, nIdle, nBusy);
	}
}

//�������ӳش�С
void CMySqlConnPoolMgr::ResetConnPoolSize(const char* szPoolName, int nMinSize, int nMaxSize)
{
	CMySqlConnPool * pPool = NULL;
	{
		SafeLock safelock(&m_csLock);
		std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.find(szPoolName);
		if (it != m_dbpool.end())
		{
			pPool = it->second;
		}
	}

	if (NULL != pPool)
	{
		pPool->ResetConnPoolSize(nMinSize, nMaxSize);
	}
}

void CMySqlConnPoolMgr::ActiveIdleConn()
{
	std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.begin();
	for (it; it != m_dbpool.end() && !m_bExit; ++it)
	{
		if (NULL != it->second) it->second->ActiveIdleConn();
	}
}

//�ѶϿ����������³�������
void CMySqlConnPoolMgr::ReConnect()
{
	std::map<std::string, CMySqlConnPool*>::iterator it = m_dbpool.begin();
	for (it; it != m_dbpool.end() && !m_bExit; ++it)
	{
		if (NULL != it->second) it->second->CheckOFFLineDBCon();
	}
}

//���̼߳��ÿ�����ӳ��е������Ƿ񳬹�һ����ʱ��û�м��������м���
void* CMySqlConnPoolMgr::ThreadCheckConnPool(void* pParam)
{
    CMySqlConnPoolMgr *pMySqConnPoolMgr = (CMySqlConnPoolMgr *)pParam;
    if (NULL == pMySqConnPoolMgr)
    {
        return 0;
    }

	prctl(PR_SET_NAME, "MysqlConnCheck");
	
	InterlockedIncrement(&pMySqConnPoolMgr->m_nThrCnt);
	
	time_t tLastActive = time(NULL);
	time_t tLastCheckOffLineTM = time(NULL);
    while (!pMySqConnPoolMgr->m_bExit)
    {
        try
        {
			if (abs(time(NULL) - tLastActive) > MAX_DBCONN_ACTIVE_TM)
			{
				pMySqConnPoolMgr->ActiveIdleConn();
				time(&tLastActive);
			}

			if (abs(time(NULL) - tLastCheckOffLineTM) > MAX_DBCONN_RECON_TM)
			{
				pMySqConnPoolMgr->ReConnect();
				time(&tLastCheckOffLineTM);
			}
			sleep(1);	
        }
        catch (...)
        {
        }
    }

	InterlockedDecrement(&pMySqConnPoolMgr->m_nThrCnt);

    return 0;
}


//**********************************
//Description: ԭ�Ӳ���-����
// Parameter:  [IN] PLONG  Addend Ҫ��������
// Returns:    LONG  32λ�з�������
//
//************************************
long CMySqlConnPoolMgr::InterlockedIncrement(PLONG  Addend)
{
    //return __gnu_cxx::__exchange_and_add((_Atomic_word*)Addend, 1);
    __gnu_cxx::__atomic_add((_Atomic_word*)Addend,1);
    //__atomic_add((_Atomic_word*)Addend,1);
    return *Addend;
    //return InterlockedIncrement(Addend);
    //return __gnu_cxx::__exchange_and_add((_Atomic_word*)Addend, 1);
}

//**********************************
//Description: ԭ�Ӳ���-�ݼ�
// Parameter:  [IN] PLONG  Addend ��long*)
// Returns:    LONG  32λ�з�������
//************************************
long CMySqlConnPoolMgr::InterlockedDecrement(PLONG  Addend)
{
    __gnu_cxx::__atomic_add((_Atomic_word*)Addend,-1);
    //__atomic_add((_Atomic_word*)Addend,-1);
    return *Addend;
    //return InterlockedDecrement(Addend);
    //return __gnu_cxx::__exchange_and_add((_Atomic_word*)Addend, -1);
}