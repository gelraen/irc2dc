INCLUDES = 
METASOURCES = AUTO
bin_PROGRAMS = irc2dc
irc2dc_SOURCES = config.cpp connection.cpp dcclient.cpp dcconfig.cpp \
	dcconnection.cpp ircconfig.cpp ircconnection.cpp main.cpp translator.cpp
noinst_HEADERS = config.h connection.h dcclient.h dcconfig.h dcconnection.h \
	ircclient.h ircconfig.h ircconnection.h translator.h
noinst_PROGRAMS = dctest irctest
dctest_SOURCES = connection.cpp dcclient.cpp dcconfig.cpp dcconnection.cpp \
	dctest.cpp
irctest_SOURCES = connection.cpp ircclient.cpp ircconfig.cpp ircconnection.cpp \
	irctest.cpp
