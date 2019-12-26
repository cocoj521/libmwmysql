CC 	= g++
CFLAGS  += -DLINUX -DSQL_WCHART_CONVERT -DLIB_EXPORTS -ldl

ARCH = _x86_

#version type(debug or release)
ifeq ($(VERTYPE),DEBUG)
CFLAGS  += -Wall -g -DDEBUG
endif
ifeq ($(VERTYPE),RELEASE)
CFLAGS  +=
endif

#
ifeq ($(OLEVEL),0)
CFLAGS  += 
endif
ifeq ($(OLEVEL),1)
CFLAGS  += -O1
endif
ifeq ($(OLEVEL),2)
CFLAGS  += -O2
endif
ifeq ($(OLEVEL),3)
CFLAGS  += -O3
endif


ifeq ($(GCCTYPE),OLD)
CC = g++343
endif
ifeq ($(GCCTYPE),NEW)
CC = g++
endif
ifeq ($(GCCTYPE),G64M32)
CC = g++
CFLAGS  += -D_686_M32_
endif
ifeq ($(GCCTYPE),X86_64)
CC = g++
CFLAGS  += -D_X64_ -fPIC -lpthread
ARCH = _x64_
endif

INCLUDE += -I/usr/include/mysql
LIB += -L/usr/lib64/mysql -lmysqlclient

ROBJS =
LOBJS = main.o LockBase.o DerivedLock.o SafeLock.o SQLBase.o SQLConnection.o SQLException.o SQLField.o SQLResult.o SQLValue.o MySqlHelper.o MySqlConnPool.o MySqlOperator.o
OBJS = $(ROBJS) $(LOBJS)
RSRCS =
LSRCS = main.cpp LockBase.cpp DerivedLock.cpp SafeLock.cpp SQLBase.cpp SQLConnection.cpp SQLException.cpp SQLField.cpp SQLResult.cpp SQLValue.cpp MySqlHelper.cpp MySqlConnPool.cpp MySqlOperator.cpp
SRCS =  $(RSRCS) $(LSRCS)

TARGET += ./testmysqlhelper

$(TARGET): $(OBJS)
	$(CC) $(INCLUDE) $(CFLAGS) -o $@ $(OBJS) $(LIB)
.cpp.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c $<

clean:
	rm -f $(LOBJS)
	rm -f ./testmysqlhelper
