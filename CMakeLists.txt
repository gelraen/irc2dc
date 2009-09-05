cmake_minimum_required(VERSION 2.6)

include(CheckStructHasMember)

project(irc2dc C CXX)

set(CONFPATH etc CACHE STRING "Path to config file (relative to PREFIX)")
set(CONFFILE irc2dc.conf CACHE STRING "Name of config file")

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_SOURCE_DIR}/config.h)

add_executable(irc2dc
		src/config.cpp
		src/config.h
		src/connection.cpp
		src/connection.h
		src/dcclient.cpp
		src/dcclient.h
		src/dcconfig.cpp
		src/dcconfig.h
		src/dcconnection.cpp
		src/dcconnection.h
		src/defs.cpp
		src/defs.h
		src/ircclient.cpp
		src/ircclient.h
		src/ircconfig.cpp
		src/ircconfig.h
		src/ircconnection.cpp
		src/ircconnection.h
		src/main.cpp
		src/translator.cpp
		src/translator.h)

install(TARGETS irc2dc RUNTIME DESTINATION bin)
install(FILES src/irc2dc.conf.dist DESTINATION ${CONFPATH} RENAME ${CONFFILE}.dist)

add_definitions(-DCONFFILE="${CMAKE_INSTALL_PREFIX}/${CONFPATH}/${CONFFILE}")

check_struct_has_member("struct sockaddr_in" sin_len netinet/in.h HAVE_SIN_LEN)

if (HAVE_SIN_LEN)
	add_definitions(-DHAVE_SIN_LEN)
endif(HAVE_SIN_LEN)

if (WITH_IPV6)
	add_definitions(-DWITH_INET6)
	check_struct_has_member("struct sockaddr_in6" sin6_len netinet/in.h HAVE_SIN6_LEN)
	if (HAVE_SIN6_LEN)
		add_definitions(-DHAVE_SIN6_LEN)
	endif(HAVE_SIN6_LEN)
endif(WITH_IPV6)

