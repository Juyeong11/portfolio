#pragma once
#include <iostream>
#include<assert.h>
#include<array>
#include<vector>
#include<map>
#include<format>
#include<queue>

#include<thread>
#include<mutex>
#include<chrono>
#include<unordered_set>
#include<atomic>
#include<concurrent_priority_queue.h> 
#include<concurrent_queue.h>

extern "C" {
#include"include/lua.h"
#include"include/lauxlib.h"
#include"include/lualib.h"
}
#pragma comment(lib, "lua54.lib")
const int BUFSIZE = 256;