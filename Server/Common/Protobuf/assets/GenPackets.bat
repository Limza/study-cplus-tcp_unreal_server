@echo off
cd /d %~dp0
protoc.exe -I=./ --cpp_out=./ ./Enum.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto

GetPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GetPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_

IF ERRORLEVEL 1 PAUSE 

COPY /Y Enum.pb.h "..\..\..\GameServer\Enum.pb.h"
COPY /Y Enum.pb.cc "..\..\..\GameServer\Enum.pb.cc"
COPY /Y Struct.pb.h "..\..\..\GameServer\Struct.pb.h"
COPY /Y Struct.pb.cc "..\..\..\GameServer\Struct.pb.cc"
COPY /Y Protocol.pb.h "..\..\..\GameServer\Protocol.pb.h"
COPY /Y Protocol.pb.cc "..\..\..\GameServer\Protocol.pb.cc"
COPY /Y ClientPacketHandler.h "..\..\..\GameServer\ClientPacketHandler.h"

COPY /Y Enum.pb.h "..\..\..\DummyClient\Enum.pb.h"
COPY /Y Enum.pb.cc "..\..\..\DummyClient\Enum.pb.cc"
COPY /Y Struct.pb.h "..\..\..\DummyClient\Struct.pb.h"
COPY /Y Struct.pb.cc "..\..\..\DummyClient\Struct.pb.cc"
COPY /Y Protocol.pb.h "..\..\..\DummyClient\Protocol.pb.h"
COPY /Y Protocol.pb.cc "..\..\..\DummyClient\Protocol.pb.cc"
COPY /Y ServerPacketHandler.h "..\..\..\DummyClient\ServerPacketHandler.h"

DEL /F /Q *.pb.h
DEL /F /Q *.pb.cc
DEL /F /Q *.h

PAUSE