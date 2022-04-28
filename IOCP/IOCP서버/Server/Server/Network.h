#pragma once

#include <WS2tcpip.h>
#include<MSWSock.h>
#include <windows.h> 
#include <sqlext.h>
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "MSWSock.LIB")

#include"protocol.h"
#include"Client.h"

const int MAX_NON_MOVABLE_MONSTER = 150'000;
const int MAX_AGRO_MONSTER = 25'000;
const int MAX_PEACE_MONSTER = 25'000;

constexpr int AGRO_MONSTER_ID_START = MAX_USER;
constexpr int AGRO_MONSTER_ID_END = MAX_USER + MAX_AGRO_MONSTER - 1;

constexpr int PEACE_MONSTER_ID_START = AGRO_MONSTER_ID_END + 1;
constexpr int PEACE_MONSTER_ID_END = PEACE_MONSTER_ID_START + MAX_PEACE_MONSTER - 1;

constexpr int NON_MOVABLE_MONSTER_ID_START = PEACE_MONSTER_ID_END + 1;
constexpr int NON_MOVABLE_MONSTER_ID_END = NON_MOVABLE_MONSTER_ID_START + MAX_NON_MOVABLE_MONSTER - 1;

enum EFFECT_TYPE { PLAYER_ATTACK, MONSTER_ATTACK, LEVEL_UP };

void error_display(const char* err_p, int err_no);

enum EVENT_TYPE { EVENT_NPC_MOVE, EVENT_NPC_MOVE_END, EVENT_NPC_RESPWAN, EVENT_NPC_ATTACK, EVENT_PLAYER_RESPWAN, EVENT_PLAYER_HEAL};
struct timer_event {
	int obj_id;
	std::chrono::system_clock::time_point start_time;
	EVENT_TYPE ev;
	int target_id;

	constexpr bool operator <(const timer_event& left)const
	{
		return (start_time > left.start_time);
	}
};


class DataBase;
const int THREAD_NUM = 6;

class Network
{
private:
	static Network* instance;

	char map_data[WORLD_HEIGHT * WORLD_WIDTH];
public:
	static Network* GetInstance();

	DataBase* DB;

	HANDLE g_h_iocp;
	SOCKET g_s_socket;
	Network();
	~Network();


	void start_accept() {
		SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

		*(reinterpret_cast<SOCKET*>(accept_ex._net_buf)) = c_socket;

		ZeroMemory(&accept_ex._wsa_over, sizeof(accept_ex._wsa_over));
		accept_ex._comp_op = OP_ACCEPT;

		AcceptEx(g_s_socket, c_socket, accept_ex._net_buf + sizeof(SOCKET), 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, NULL, &accept_ex._wsa_over);
	}

	void send_login_ok(int client_id);
	void send_login_fail(int client_id);
	void send_move_object(int client_id, int mover_id);
	void send_put_object(int client_id, int target_id);
	void send_remove_object(int client_id, int victim_id);
	void send_status(int client_id, int target_id);
	void send_effect(int client_id, int target_id, int effect_type, int info);
	void send_chat_packet(int user_id, int my_id, char* mess)
	{
		sc_packet_chat packet;
		strcpy_s(packet.message, mess);
		packet.id = my_id;
		packet.size = sizeof(packet);
		packet.type = SC_PACKET_CHAT;

		EXP_OVER* ex_over;
		while (!exp_over_pool.try_pop(ex_over));
		ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
		reinterpret_cast<Client*>(ex_over, clients[user_id])->do_send(ex_over);
	}
	void disconnect_client(int client_id);

	bool is_near(int a, int b)
	{
		if (RANGE < abs(clients[a]->x - clients[b]->x)) return false;
		if (RANGE < abs(clients[a]->y - clients[b]->y)) return false;

		return true;
	}
	bool can_move(int a, int b)
	{
		if (10 < abs(clients[a]->x - clients[b]->x)) return false;
		if (10 < abs(clients[a]->y - clients[b]->y)) return false;

		return true;
	}
	bool can_attack(int a, int b)
	{
		int x = abs(clients[a]->x - clients[b]->x);
		int y = abs(clients[a]->y - clients[b]->y);
		if (1 < x + y) return false;

		return true;
	}
	bool can_agro(int a, int b)
	{
		if (6 < abs(clients[a]->x - clients[b]->x)) return false;
		if (6 < abs(clients[a]->y - clients[b]->y)) return false;

		return true;
	}
	bool is_npc(int id)
	{
		return (id >= NPC_ID_START) && (id <= NPC_ID_END);
	}
	bool is_player(int id)
	{
		return (id >= 0) && (id < MAX_USER);
	}

	bool is_non_movable_monster(int id)
	{
		return (id >= NON_MOVABLE_MONSTER_ID_START) && (id <= NON_MOVABLE_MONSTER_ID_END);
	}
	bool is_movable_monster(int id)
	{
		return (id >= AGRO_MONSTER_ID_START) && (id <= PEACE_MONSTER_ID_END);
	}

	bool is_agro_monster(int id)
	{
		return (id >= AGRO_MONSTER_ID_START) && (id <= AGRO_MONSTER_ID_END);
	}
	bool is_peace_monster(int id)
	{
		return (id >= PEACE_MONSTER_ID_START) && (id <= PEACE_MONSTER_ID_END);
	}

	int get_new_id();

	void Initialize_NPC();

	void activate_player_move_event(int target, int player_id) {
		EXP_OVER* ex;
		while (!exp_over_pool.try_pop(ex));
		ex->_comp_op = OP_PLAYER_MOVE;
		memcpy(ex->_net_buf, &player_id, sizeof(int));
		PostQueuedCompletionStatus(g_h_iocp, 1, target, &ex->_wsa_over);
	}

	static int API_SendMessage(lua_State* L) {
		int my_id = (int)lua_tointeger(L, -3);
		int user_id = (int)lua_tointeger(L, -2);
		char* mess = (char*)lua_tostring(L, -1);

		lua_pop(L, 4);

		Network* n = Network::GetInstance();
		Client* cl = reinterpret_cast<Client*>(n->clients[user_id]);

		cl->vl.lock();
		std::unordered_set<int> my_vl{ cl->viewlist };
		cl->vl.unlock();

		for (auto i : my_vl)
		{
			if (n->is_npc(i)) continue;
			n->send_chat_packet(i, my_id, mess);
		}
		n->send_chat_packet(user_id, my_id, mess);
		return 0;
	}
	static int API_set_event(lua_State* L) {
		int time = (int)lua_tointeger(L, -1);
		int my_id = (int)lua_tointeger(L, -3);
		int plyer_id = (int)lua_tointeger(L, -2);

		lua_pop(L, 4);

		timer_event t;
		//t.ev = EVENT_NPC_MOVE;
		t.obj_id = my_id;
		t.target_id = plyer_id;
		//t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
		//Network::GetInstance()->timer_queue.push(t);
		//t.start_time = t.start_time + std::chrono::seconds(1);
		//Network::GetInstance()->timer_queue.push(t);
		//t.ev = EVENT_NPC_MOVE_END;
		//t.start_time = t.start_time + std::chrono::seconds(1);
		//Network::GetInstance()->timer_queue.push(t);

		t.ev = EVENT_NPC_MOVE_END;
		t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(time);
		Network::GetInstance()->timer_queue.push(t);
		return 0;
	}

	static int API_get_x(lua_State* L) {
		int user_id = (int)lua_tointeger(L, -1);
		lua_pop(L, 2);
		int x = Network::GetInstance()->clients[user_id]->x;
		lua_pushnumber(L, x);
		return 1;
	}

	static int API_get_y(lua_State* L) {
		int user_id = (int)lua_tointeger(L, -1);
		lua_pop(L, 2);
		int y = Network::GetInstance()->clients[user_id]->y;
		lua_pushnumber(L, y);
		return 1;
	}

	void do_npc_move(int npc_id);
	void do_timer();
	void do_timer_event_act(const timer_event& ev);
	void process_packet(int client_id, unsigned char* p);

	void worker();
private:
	concurrency::concurrent_priority_queue<timer_event> timer_queue;
	concurrency::concurrent_queue<EXP_OVER*> exp_over_pool;
	std::array<GameObject*, MAX_USER + MAX_NPC> clients;//»Ï..
	
	EXP_OVER accept_ex;
};

