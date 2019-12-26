#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include "SQLConnection.h"
#include "SQLException.h"
#include "SQLResult.h"
#include "SQLValue.h"
#include "MySqlHelper.h"

using namespace MySqlHelper;
using namespace std;
//#include <unistd.h>
/*
int main()
{

CSQLConnection sqlconn;
sqlconn.Open("192.169.1.130", "smsaccount", "root", "521521", 3306);
while (1)
{
try
{
CSQLResult rs(sqlconn);
//rs.Query("insert into testtb (testdata) values ('testdata')", false);
rs.Query("CALL GetLoginUserInfo_B('WBS00A')", true);
//SQLValueArray values;
//while (rs.Fetch(values))
{
//printf("get values...\r\n");
}
}
catch (CSQLException e)
{
printf("ErrorMsg:%s\r\nDescription:%s\r\n", e.ErrorMessage(), e.Description());
}
catch (...)
{
printf("other error...\r\n");
}
sleep(1);
}
while(1)
{

#if (defined  MW_WINDOWS) || (defined  WIN32)
Sleep(1);
#else
sleep(1);
#endif
}
return 0;
}
*/


int main()
{
    string sql = "SELECT * from tz_user where phone='13510924061'";
    CMySqlHelper mysqlhelper;
    std::string strErrMsg;
    std::string strErrDesc;
    if (mysqlhelper.CreateConnPool(strErrMsg, strErrDesc, "testpool", "192.169.0.163", "tzw_app", "root", "123456", 3306, "SELECT 1", 60, true, 5, 50))
    {
        CMySqlRecordSet records;
        if (0 == mysqlhelper.ExecuteWithRecordSetRet(strErrMsg, strErrDesc, records, "testpool", sql.c_str()))
        {
            //std::map<std::string, CSQLField> COLUMN_MAP;//列:存储列名与列值在行中的下标对应关系
            /*
            COLUMN_NAME_LIST fieldNameList;
            if (records.GetFieldNameList(fieldNameList))
            {
                for (COLUMN_NAME_LIST::iterator item = fieldNameList.begin(); item != fieldNameList.end(); item++)
                {
                    printf("column name:%s\n", item->c_str());
                }
            } */

            records.MoveFirst();
            while (!records.bEOF())
            {
                SQLVALUE value;
                //取该字段的值,有才取,没有时不要取
                if (records.GetFieldValueFromCurrRow("NAME", value))
                {
                    string name = value.GetData();
                    printf("名字(胡建玲)NAME=%s\n", name.c_str());
                }
                if (records.GetFieldValueFromCurrRow("ACCOUNT", value))
                {
                    printf("帐号ACCOUNT=%s\n", (const char*)value);
                }
                if (records.GetFieldValueFromCurrRow("PHONE", value))
                {
                    printf("手机号码PHONE=%s\n", (const char*)value);
                }
                records.MoveNext();
            }
        }
        //失败原因打印
        else
        {
            printf("%s--%s\n", strErrMsg.c_str(), strErrDesc.c_str());
        }
    }
    //失败原因打印
    else
    {
        printf("%s--%s\n", strErrMsg.c_str(), strErrDesc.c_str());
    }

    while (true)
    {
        sleep(1);
    }

    return 0;
}

