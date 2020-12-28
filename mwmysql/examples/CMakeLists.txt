set(tests_SRCS
	test.cpp
  )

#include_directories(${PROJECT_SOURCE_DIR}/mwmysql/examples/include)
#link_directories(${PROJECT_SOURCE_DIR}/mwmysql/examples/lib)

add_executable(mwmysql_test test.cpp)
target_link_libraries(mwmysql_test
    mwmysql
    pthread
    z
    rt
    dl
)
