HEADERS       = server.h ../Database/database.h
SOURCES       = server.cpp main.cpp ../Database/database.cpp
QT           += network

TEMPLATE = app
TARGET = Server

# install
LIBS += -L../Database -lDatabase 
