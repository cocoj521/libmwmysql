set(mwmysql_SRCS
	LockBase.cpp 
	DerivedLock.cpp 
	SafeLock.cpp 
	SQLBase.cpp 
	SQLConnection.cpp 
	SQLException.cpp 
	SQLField.cpp 
	SQLResult.cpp 
	SQLValue.cpp 
	MySqlHelper.cpp 
	MySqlConnPool.cpp 
	MySqlOperator.cpp
)

if(NOT CMAKE_BUILD_DYNAMIC_LIB)
	add_library(mwmysql ${mwmysql_SRCS})
else()
	add_library(mwmysql SHARED ${mwmysql_SRCS})
endif()

target_link_libraries(mwmysql mysqlclient pthread rt)

install(TARGETS mwmysql DESTINATION lib)

set(HEADERS
	MySqlHelper.h
	SQLBase.h
	SQLConnection.h
	SQLException.h
	SQLField.h
	SQLResult.h
	SQLTypes.h
	SQLValue.h
)

install(FILES ${HEADERS} DESTINATION include)
