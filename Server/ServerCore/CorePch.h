#pragma once

#include "Types.h"
#include "CoreMacro.h"
#include "CoreTls.h"
#include "CoreGlobal.h"
#include "Container.h"

#include <windows.h>
#include <iostream>
#include <cassert>
#include <chrono>

#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Lock.h"
#include "ObjectPool.h"
#include "TypeCast.h"
#include "Memory.h"
#include "SendBuffer.h"
#include "Session.h"
#include "LockQueue.h"