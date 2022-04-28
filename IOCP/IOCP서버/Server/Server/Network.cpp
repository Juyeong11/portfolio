#include<fstream>
#include "stdfx.h"

#include "DataBase.h"
#include "PathFinder.h"
#include "Network.h"

Network* Network::instance = nullptr;

Network* Network::GetInstance()
{
	return instance;
}

void error_display(const char* err_p, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, 0);
	std::cout << err_p << std::endl;
	std::wcout << lpMsgBuf << std::endl;
	//while (true);
	LocalFree(lpMsgBuf);
}

Network::Network() {
	//인스턴스는 한 개만!!
	assert(instance == nullptr);
	instance = this;
	{
		using namespace std;
		ifstream in{ "Map//map.bin",ios::binary };

		int i = 0;
		while (in >> map_data[i]) {
			i++;
		}
	}
	DB = new DataBase();
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i] = new Client;
		clients[i]->index = i;
	}
	for (int i = AGRO_MONSTER_ID_START; i <= AGRO_MONSTER_ID_END; ++i) {
		clients[i] = new AgroMonster(map_data);
		clients[i]->index = i;
	}
	for (int i = PEACE_MONSTER_ID_START; i <= PEACE_MONSTER_ID_END; ++i) {
		clients[i] = new PeaceMonster(map_data);
		clients[i]->index = i;
	}

	for (int i = NON_MOVABLE_MONSTER_ID_START; i <= NON_MOVABLE_MONSTER_ID_END; ++i) {
		clients[i] = new non_movable_Npc();
		clients[i]->index = i;
	}

	for (int i = 0; i < MAX_USER + MAX_NPC; ++i) {
		exp_over_pool.push(new EXP_OVER);
	}
	Initialize_NPC();
	std::cout << "생성 완료" << std::endl;
}

Network::~Network() {
	//스레드가 종료된 후 이기 때문에 락을 할 필요가 없다
//accpet상태일 때 문제가 생긴다
	for (int i = 0; i < MAX_USER; ++i)
		if (ST_INGAME == clients[i]->state)
			disconnect_client(clients[i]->index);

	for (int i = 0; i < MAX_USER + MAX_NPC; ++i) {
		delete clients[i];
	}
	for (int i = 0; i < MAX_USER + MAX_NPC; ++i) {
		EXP_OVER* ex;
		exp_over_pool.try_pop(ex);
		delete ex;
	}

	delete DB;
}
void Network::Initialize_NPC() {
	for (int i = AGRO_MONSTER_ID_START; i <= AGRO_MONSTER_ID_END; ++i) {
		sprintf_s(clients[i]->name, "Agro%d", i);
		int x = 0;
		int y = 0;
		do {
			x = rand() % WORLD_WIDTH;
			y = rand() % WORLD_HEIGHT;
		} while (map_data[y * WORLD_HEIGHT + x] == 0);
		clients[i]->x = x;
		clients[i]->y = y;
		clients[i]->index = i;
		clients[i]->state = ST_INGAME;
		clients[i]->type = 3; // Monster
	}

	for (int i = PEACE_MONSTER_ID_START; i <= PEACE_MONSTER_ID_END; ++i) {
		sprintf_s(clients[i]->name, "Peace%d", i);
		int x = 0;
		int y = 0;
		do {
			x = rand() % WORLD_WIDTH;
			y = rand() % WORLD_HEIGHT;
		} while (map_data[y * WORLD_HEIGHT + x] == 0);
		clients[i]->x = x;
		clients[i]->y = y;
		clients[i]->index = i;
		clients[i]->state = ST_INGAME;
		clients[i]->type = 3; // Monster

		/*
		루아 초기화
		*/
		/* {
				lua_State* L = reinterpret_cast<PeaceMonster*>(clients[i])->L = luaL_newstate();
				luaL_openlibs(L);
				luaL_loadfile(L, "monster.lua");
				int err = lua_pcall(L, 0, 0, 0);
				if (err) {
					std::cout << "Error:" << lua_tostring(L, -1);
					lua_pop(L, 1);
				}
				lua_getglobal(L, "set_uid");
				lua_pushnumber(L, i);//set_uid를 호출하고 스택에서 뺌
				err = lua_pcall(L, 1, 1, 0);
				if (err) {
					std::cout << "Error:" << lua_tostring(L, -1);
					lua_pop(L, 1);
				}
				lua_pop(L, 1);

				//lua_register...


				lua_register(L, "API_SendMessage", API_SendMessage);
				lua_register(L, "API_set_event", API_set_event);
				lua_register(L, "API_get_x", API_get_x);
				lua_register(L, "API_get_y", API_get_y);
			}
			*/
	}

	for (int i = NON_MOVABLE_MONSTER_ID_START + 1; i <= NON_MOVABLE_MONSTER_ID_END; ++i) {
		sprintf_s(clients[i]->name, "NPC%d", i);
		int x = 0;
		int y = 0;
		do {
			x = rand() % WORLD_HEIGHT;
			y = rand() % WORLD_WIDTH;
		} while (map_data[y * WORLD_HEIGHT + x] == 0);
		clients[i]->x = x;
		clients[i]->y = y;

		clients[i]->index = i;
		clients[i]->state = ST_INGAME;
		clients[i]->type = 2; // NPC


	}
}

void Network::send_login_ok(int c_id)
{
	sc_packet_login_ok packet;
	packet.id = c_id;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_OK;
	packet.x = clients[c_id]->x;
	packet.y = clients[c_id]->y;

	send_status(c_id, c_id);

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}

void Network::send_login_fail(int c_id)
{
	sc_packet_login_fail packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_LOGIN_FAIL;
	packet.reason = 1;

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}

void Network::send_move_object(int c_id, int mover)
{
	sc_packet_move packet;
	packet.id = mover;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_MOVE;
	packet.x = clients[mover]->x;
	packet.y = clients[mover]->y;
	packet.move_time = clients[mover]->last_move_time;

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}

void Network::send_status(int c_id, int t_id)
{
	sc_packet_status_change packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_STATUS_CHANGE;
	packet.level = clients[t_id]->level;
	packet.hp = clients[t_id]->hp;
	packet.maxhp = clients[t_id]->maxhp;
	packet.exp = clients[t_id]->exp;
	packet.id = t_id;

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}


void Network::send_put_object(int c_id, int target) {
	sc_packet_put_object packet;

	strcpy_s(packet.name, clients[target]->name);
	packet.id = target;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_PUT_OBJECT;
	packet.object_type = 0;
	packet.x = clients[target]->x;
	packet.y = clients[target]->y;

	send_status(c_id, target);

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}
void Network::send_effect(int client_id, int target_id, int effect_type, int info) 
{
	sc_packet_effect packet;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_EFFECT;
	packet.id = target_id;
	packet.effect_type = effect_type;
	packet.info = info;

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[client_id])->do_send(ex_over);
}
void Network::send_remove_object(int c_id, int victim)
{
	sc_packet_remove_object packet;
	packet.id = victim;
	packet.size = sizeof(packet);
	packet.type = SC_PACKET_REMOVE_OBJECT;

	EXP_OVER* ex_over;
	while (!exp_over_pool.try_pop(ex_over));
	ex_over->set_exp(OP_SEND, sizeof(packet), &packet);
	reinterpret_cast<Client*>(ex_over, clients[c_id])->do_send(ex_over);
}

void Network::disconnect_client(int c_id)
{
	if (c_id >= MAX_USER)
		std::cout << "disconnect_client : unexpected id range" << std::endl;
	Client& client = *reinterpret_cast<Client*>(clients[c_id]);

	client.vl.lock();
	std::unordered_set <int> my_vl = client.viewlist;
	client.vl.unlock();
	for (auto other : my_vl) {
		if (true == is_npc(other)) { 

			if (true == is_movable_monster(other)) {
				movable_Npc& target = *reinterpret_cast<movable_Npc*>(clients[other]);
				target.target_id = -1;
			}
			continue; 
		}

		Client& target = *reinterpret_cast<Client*>(clients[other]);
		if (ST_INGAME != target.state)
			continue;
		target.vl.lock();
		if (0 != target.viewlist.count(c_id)) {
			target.viewlist.erase(c_id);
			target.vl.unlock();
			send_remove_object(other, c_id);
		}
		else target.vl.unlock();
	}

	DB->update_player_data(clients[c_id]);
	clients[c_id]->state_lock.lock();
	closesocket(reinterpret_cast<Client*>(clients[c_id])->socket);
	clients[c_id]->state = ST_FREE;
	clients[c_id]->state_lock.unlock();
}

int Network::get_new_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		clients[i]->state_lock.lock();
		if (ST_FREE == clients[i]->state) {
			clients[i]->state = ST_ACCEPT;
			clients[i]->state_lock.unlock();
			return i;
		}
		clients[i]->state_lock.unlock();
	}
	std::cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}
void Network::do_timer_event_act(const timer_event& ev) {
	switch (ev.ev) {
	case EVENT_NPC_MOVE:
	{
		EXP_OVER* ex_over;// = new EXP_OVER;
		//ex_over->_comp_op = OP_NPC_MOVE;
		while (!exp_over_pool.try_pop(ex_over));
		ex_over->_comp_op = OP_NPC_MOVE;
		PostQueuedCompletionStatus(g_h_iocp, 1, ev.obj_id, &ex_over->_wsa_over);// 두번째 인자가 0이 되면 소캣 종료로 취급이 된다. 1로해주자
	}
	break;
	case EVENT_NPC_MOVE_END:
	{

		lua_State* L = reinterpret_cast<PeaceMonster*>(clients[ev.obj_id])->L;
		lua_getglobal(L, "event_player_move_end");//lua함수
		lua_pushnumber(L, ev.target_id);
		int err = lua_pcall(L, 1, 0, 0);
		if (err) {
			std::cout << "Error:" << lua_tostring(L, -1);
			lua_pop(L, 1);
		}
	}
	break;
	case EVENT_PLAYER_RESPWAN:
	{
		int client_id = ev.obj_id;


		clients[client_id]->state_lock.lock();
		clients[client_id]->state = ST_INGAME;
		clients[client_id]->state_lock.unlock();
		clients[client_id]->hp = clients[client_id]->maxhp;
		clients[client_id]->x = rand() % WORLD_WIDTH;
		clients[client_id]->y = rand() % WORLD_HEIGHT;
		clients[client_id]->exp *= 0.5f;
		Client& cl = *reinterpret_cast<Client*>(clients[client_id]);
		//다른 클라이언트에게 새로운 클라이언트가 들어옴을 알림
		for (int i = 0; i < MAX_USER; ++i)
		{
			Client* other = reinterpret_cast<Client*>(clients[i]);
			if (i == client_id) continue;
			if (ST_INGAME != other->state) {
				continue;
			}

			if (false == is_near(other->index, client_id))
				continue;

			// 새로 들어온 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			other->vl.lock();
			other->viewlist.insert(client_id);
			other->vl.unlock();

			send_put_object(other->index, client_id);
		}

		//새로 접속한 클라이언트에게 현재 객체들의 현황을 알려줌
		for (auto* other : clients) {
			//여기서 NPC도 알려줘야지
			if (other->index == client_id) continue;
			if (ST_INGAME != other->state) {
				continue;
			}

			if (false == is_near(other->index, client_id))
				continue;

			// 기존에 있던 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			cl.vl.lock();
			cl.viewlist.insert(other->index);
			cl.vl.unlock();

			send_put_object(client_id, other->index);

			if (is_movable_monster(other->index)) {

				if (false == reinterpret_cast<movable_Npc*>(clients[other->index])->is_move &&
					true == can_move(other->index, client_id)) {
					reinterpret_cast<movable_Npc*>(clients[other->index])->is_move = true;
					timer_event t;
					t.ev = EVENT_NPC_MOVE;
					t.obj_id = other->index;
					t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
					timer_queue.push(t);

					if (true == is_agro_monster(other->index)) {
						if (can_agro(other->index, client_id))
							reinterpret_cast<AgroMonster*>(clients[other->index])->target_id = client_id;
					}
				}
			}
		}
		send_put_object(client_id, client_id);
	}
	break;
	case EVENT_NPC_RESPWAN:
	{


		int npc_id = ev.obj_id;
		clients[npc_id]->hp = clients[npc_id]->maxhp;

		clients[npc_id]->state_lock.lock();
		clients[npc_id]->state = ST_INGAME;
		clients[npc_id]->state_lock.unlock();

		Client& cl = *reinterpret_cast<Client*>(clients[npc_id]);
		//다른 클라이언트에게 오브젝트가 스폰됨을 알림
		for (int i = 0; i < MAX_USER; ++i)
		{
			Client* other = reinterpret_cast<Client*>(clients[i]);
			if (i == npc_id) continue;
			if (ST_INGAME != other->state) {
				continue;
			}

			if (false == is_near(other->index, npc_id))
				continue;

			// 새로 들어온 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			other->vl.lock();
			other->viewlist.insert(npc_id);
			other->vl.unlock();

			if (is_movable_monster(npc_id)) {

				if (false == reinterpret_cast<movable_Npc*>(clients[npc_id])->is_move &&
					true == can_move(npc_id, i)) {
					reinterpret_cast<movable_Npc*>(clients[npc_id])->is_move = true;
					timer_event t;
					t.ev = EVENT_NPC_MOVE;
					t.obj_id = npc_id;
					t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
					timer_queue.push(t);

					if (true == is_agro_monster(npc_id)) {
						if (can_agro(npc_id, i))
							reinterpret_cast<AgroMonster*>(clients[npc_id])->target_id = i;
					}
				}
			}

			send_put_object(other->index, npc_id);
		}

	}
	break;
	case EVENT_NPC_ATTACK:
	{
		int a_id = ev.obj_id;
		int t_id = ev.target_id;
		if (reinterpret_cast<Npc*>(clients[a_id])->target_id == -1)break;
		if (false == can_attack(a_id, t_id)) {
			if (is_movable_monster(a_id)) {
				timer_event t;
				t.ev = EVENT_NPC_MOVE;
				t.obj_id = a_id;
				t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
				timer_queue.push(t);
			}
			break;
		}
		Client* cl = reinterpret_cast<Client*>(clients[t_id]);
		cl->hp -= clients[a_id]->power;

		if (false == cl->is_heal) {
			cl->is_heal = true;
			timer_event t;
			t.ev = EVENT_PLAYER_HEAL;
			t.obj_id = t_id;
			
			t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(5);
			timer_queue.push(t);
		}
		cl->vl.lock();
		std::unordered_set<int> c_vl{ cl->viewlist };
		cl->vl.unlock();

		if (cl->hp < 0) {

			timer_event t;
			t.ev = EVENT_PLAYER_RESPWAN;
			t.obj_id = t_id;
			t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(10);
			timer_queue.push(t);

			cl->state_lock.lock();
			cl->state = ST_ACCEPT;
			cl->state_lock.unlock();

			for (auto i : c_vl) {
				if (false == is_player(i))continue;
				Client* other_p = reinterpret_cast<Client*>(clients[i]);

				other_p->vl.lock();
				other_p->viewlist.erase(t_id);
				other_p->vl.unlock();

				send_remove_object(i, t_id);
			}
			send_remove_object(t_id, t_id);

			cl->vl.lock();
			cl->viewlist.clear();
			cl->vl.unlock();
			break;
		}
		else {

			for (auto i : c_vl) {
				if (false == is_player(i))continue;
				send_status(i, t_id);
			}
			send_status(t_id, t_id);

			timer_event t;
			t.ev = EVENT_NPC_ATTACK;
			t.obj_id = a_id;
			t.target_id = t_id;
			t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
			timer_queue.push(t);
		}

		// 몬스터 공격 이펙트
		for (auto id : c_vl) {
			if (false == is_player(id))continue;
			send_effect(id, t_id, MONSTER_ATTACK, clients[a_id]->power);
		}
		send_effect(t_id, t_id, MONSTER_ATTACK, clients[a_id]->power);
	}
	break;
	case EVENT_PLAYER_HEAL:
	{
		int client_id = ev.obj_id;
		if (clients[client_id]->heal()) {
			timer_event t;
			t.ev = EVENT_PLAYER_HEAL;
			t.obj_id = client_id;
			t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(5);
			timer_queue.push(t);
		}
		else {
			clients[client_id]->is_heal = false;
		}

		Client* cl = reinterpret_cast<Client*>(clients[client_id]);
		cl->vl.lock();
		std::unordered_set<int> c_vl{ cl->viewlist };
		cl->vl.unlock();

		for (auto i : c_vl) {
			if (false == is_player(i))continue;
			send_status(i, client_id);
		}
		send_status(client_id, client_id);
	}
	break;
	default:
		break;
	}
}

void Network::do_timer() {
	using namespace std;
	using namespace chrono;
	while (true) {

		timer_event ev;
		while (!timer_queue.empty()) {

			while (!timer_queue.try_pop(ev));

			if (ev.start_time <= system_clock::now()) {
				//이벤트 시작
				do_timer_event_act(ev);
			}
			else {
				//기껏 뺐는데 다시 넣는거 좀 비효율 적이다.
				//다시 넣지 않는 방법으로 최적화 필요
				// 이벤트 중 가장 짧은 시간은 1초 만약 남은 시간이 1초 이상이면 
				// 중간에 1초보다 짧은 이벤트가 끼어들 수 있다
				// 하지만 1초보다 짧다면?? 중간에 끼어들 이벤트가 생길 일이 없다
				// 즉 기다렸다가 해당 이벤트를 실행해도 된다는 것
				auto t = ev.start_time - system_clock::now();
				if (t < seconds(1)) {
					this_thread::sleep_for(t);
					do_timer_event_act(ev);
				}
				else
					timer_queue.push(ev);
				break;
			}
		}
		//큐가 비었거나 가장 짧은 이벤트가 1초기 때문에 1초 쉬어도됨
		this_thread::sleep_for(1000ms);
	}
}

void Network::do_npc_move(int npc_id) {

	std::unordered_set<int> old_viewlist;


	//해당 NPC는 모든 플레이어에 대한 viewlist를 만든다.
	//이것도 섹터로 나누고 혹시 NPC가 뷰리스트를 가지고 있으면 안되나? 이 부분부터 다시보자
	for (int i = 0; i < MAX_USER; ++i) {
		Client* obj = reinterpret_cast<Client*>(clients[i]);
		if (obj->state != ST_INGAME) continue;

		if (true == is_near(npc_id, i))
			old_viewlist.insert(obj->index);
	}
	//이동을 하고 주위 플레이어들에게 알려줘야 한다.
	short& x = clients[npc_id]->x;
	short& y = clients[npc_id]->y;

	int dir = rand() % 4;
	// 타겟이 있으면 쫓아감
	int target_player_id = reinterpret_cast<movable_Npc*>(clients[npc_id])->target_id;
	if (target_player_id != -1) {

		dir = reinterpret_cast<movable_Npc*>(clients[npc_id])->Astar->find_path(clients[npc_id], clients[target_player_id]);

	}
	switch (dir) {
	case 0:
		if (y > 0) {
			y--;
			if (map_data[y * WORLD_HEIGHT + x] == 0)
				y++;
		}
		break;
	case 1:
		if (y < (WORLD_HEIGHT - 1)) {
			y++;
			if (map_data[y * WORLD_HEIGHT + x] == 0)
				y--;
		}
		break;
	case 2:
		if (x > 0) {
			x--;
			if (map_data[y * WORLD_HEIGHT + x] == 0)
				x++;
		}
		break;
	case 3:
		if (x < (WORLD_WIDTH - 1)) {
			x++;
			if (map_data[y * WORLD_HEIGHT + x] == 0)
				x--;
		}
		break;
	default:
		break;
	}



	std::unordered_set<int> new_viewlist;
	std::unordered_set<int> move_list;

	//해당 NPC는 모든 플레이어에 대한 viewlist를 만든다.
	for (int i = 0; i < MAX_USER; ++i) {
		Client* obj = reinterpret_cast<Client*>(clients[i]);
		if (obj->state != ST_INGAME) continue;
		if (true == is_near(npc_id, i))
			new_viewlist.insert(i);

		if (true == can_move(npc_id, i))
			move_list.insert(i);
	}
	// new old 뷰 리스트들에는

	// 새로 시야에 들어온 플레이어
	for (auto pl : new_viewlist) {
		if (0 == old_viewlist.count(pl)) {
			Client* client = reinterpret_cast<Client*>(clients[pl]);
			client->vl.lock();
			client->viewlist.insert(npc_id);
			client->vl.unlock();

			send_put_object(pl, npc_id);
		}
		else {
			send_move_object(pl, npc_id);
		}
	}

	//시야에서 사라진 플레이어
	for (auto pl : old_viewlist) {
		if (0 == new_viewlist.count(pl)) {
			Client* client = reinterpret_cast<Client*>(clients[pl]);
			client->vl.lock();
			client->viewlist.erase(npc_id);
			client->vl.unlock();
			send_remove_object(pl, npc_id);
		}
	}

	if (target_player_id == -1 &&
		is_agro_monster(npc_id))
	{
		for (auto pl : new_viewlist) {
			if (can_agro(pl, npc_id)) {
				reinterpret_cast<AgroMonster*>(clients[npc_id])->target_id = pl;
				break;
			}
		}
	}
	if (target_player_id != -1 &&
		is_movable_monster(npc_id) &&
		can_attack(npc_id, target_player_id)) {
		timer_event t;
		t.ev = EVENT_NPC_ATTACK;
		t.obj_id = npc_id;
		t.target_id = target_player_id;
		t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
		timer_queue.push(t);


		return;
	}
	if (clients[npc_id]->hp <= 0) {

		reinterpret_cast<movable_Npc*>(clients[npc_id])->is_move = false;
		return;
	}
	if (false == move_list.empty()) {

		timer_event t;
		t.ev = EVENT_NPC_MOVE;
		t.obj_id = npc_id;
		t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
		timer_queue.push(t);
	}
	else {
		reinterpret_cast<movable_Npc*>(clients[npc_id])->is_move = false;
		//reinterpret_cast<movable_Npc*>(clients[npc_id])->target_id = -1;
	}

	
}

void Network::process_packet(int client_id, unsigned char* p)
{
	unsigned char packet_type = p[1];
	Client& cl = *reinterpret_cast<Client*>(clients[client_id]);

	switch (packet_type)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login* packet = reinterpret_cast<cs_packet_login*>(p);


		PlayerData player_data;
		player_data.name = std::wstring(packet->name, &packet->name[strnlen_s(packet->name, MAX_NAME_SIZE)]);

		if (false == DB->check_player_data(player_data)) {
			send_login_fail(client_id);
			break;
		}
		else {
			strcpy_s(cl.name, packet->name);
			cl.set_staus(MAX_HP* player_data.level, player_data.level, player_data.exp);
			cl.x = player_data.x;
			cl.y = player_data.y;
			send_login_ok(client_id);
		}
		//send_login_ok(client_id);

		cl.state_lock.lock();
		cl.state = ST_INGAME;
		cl.state_lock.unlock();

		//다른 클라이언트에게 새로운 클라이언트가 들어옴을 알림
		for (int i = 0; i < MAX_USER; ++i)
		{
			Client* other = reinterpret_cast<Client*>(clients[i]);
			if (i == client_id) continue;
			if (ST_INGAME != other->state) {
				continue;
			}

			if (false == is_near(other->index, client_id))
				continue;

			// 새로 들어온 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			other->vl.lock();
			other->viewlist.insert(client_id);
			other->vl.unlock();

			send_put_object(other->index, client_id);
		}
		send_status(client_id, client_id);

		//새로 접속한 클라이언트에게 현재 객체들의 현황을 알려줌
		for (auto* other : clients) {
			//여기서 NPC도 알려줘야지
			if (other->index == client_id) continue;
			if (ST_INGAME != other->state) {
				continue;
			}

			if (false == is_near(other->index, client_id))
				continue;

			// 기존에 있던 클라이언트가 가까이 있다면 뷰 리스트에 넣고 put packet을 보낸다.
			cl.vl.lock();
			cl.viewlist.insert(other->index);
			cl.vl.unlock();

			send_put_object(client_id, other->index);

			if (is_movable_monster(other->index)) {

				if (false == reinterpret_cast<movable_Npc*>(clients[other->index])->is_move &&
					true == can_move(other->index, client_id)) {
					reinterpret_cast<movable_Npc*>(clients[other->index])->is_move = true;
					timer_event t;
					t.ev = EVENT_NPC_MOVE;
					t.obj_id = other->index;
					t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
					timer_queue.push(t);

					if (true == is_agro_monster(other->index)) {
						if (can_agro(other->index, client_id))
							reinterpret_cast<AgroMonster*>(clients[other->index])->target_id = client_id;
					}
				}
			}
		}



	}
	break;
	case CS_PACKET_MOVE:
	{
		cs_packet_move* packet = reinterpret_cast<cs_packet_move*>(p);

		if (cl.mlast_move_time + std::chrono::seconds(1) > std::chrono::system_clock::now()) break;

		cl.mlast_move_time = std::chrono::system_clock::now();

		cl.last_move_time = packet->move_time;
		short& x = cl.x;
		short& y = cl.y;
		switch (packet->direction) {
		case 0:
			if (y > 0) {
				y--;
				if (map_data[y * WORLD_HEIGHT + x] == 0)
					y++;
			}
			break;
		case 1:
			if (y < (WORLD_HEIGHT - 1)) {
				y++;
				if (map_data[y * WORLD_HEIGHT + x] == 0)
					y--;
			}
			break;
		case 2:
			if (x > 0) {
				x--;
				if (map_data[y * WORLD_HEIGHT + x] == 0)
					x++;
			}
			break;
		case 3:
			if (x < (WORLD_WIDTH - 1)) {
				x++;
				if (map_data[y * WORLD_HEIGHT + x] == 0)
					x--;
			}
			break;
		default:
			std::cout << "Invalid move in client " << client_id << std::endl;
			exit(-1);
		}


		send_move_object(cl.index, cl.index);
		// 이동한 클라이언트에 대한 nearlist 생성
		// 꼭 unordered_set이여야 할까?
		// 얼마나 추가될지 모르고, 데이터는 id이기 때문에 중복없음이 보장되있다. id로 구분안하는 경우가 있나?
		// 섹터를 나누어 근처에 있는지 검색해 속도를 높이자
		std::unordered_set<int> nearlist;
		//큰 부하
		for (auto* other : clients) {
			if (other->index == client_id)
				continue;
			if (ST_INGAME != other->state)
				continue;
			if (false == is_near(client_id, other->index))
				continue;
			if (true == is_peace_monster(other->index)) {
				//activate_player_move_event(other->index, cl.index);
			}

			nearlist.insert(other->index);
		}


		//lock시간을 줄이기 위해 자료를 복사해서 사용
		cl.vl.lock();
		std::unordered_set<int> my_vl{ cl.viewlist };
		cl.vl.unlock();


		// 움직임으로써 시야에 들어온 플레이어 확인 및 추가
		for (int other : nearlist) {
			// cl의 뷰리스트에 없으면
			if (0 == my_vl.count(other)) {
				// cl의 뷰리스트에 추가하고
				cl.vl.lock();
				cl.viewlist.insert(other);
				cl.vl.unlock();
				// 보였으니 그리라고 패킷을 보낸다.
				send_put_object(cl.index, other);

				//npc는 send를 안한다.
				//npc는 뷰리스트가 없고 자신을 볼 수 있는 플레이어가 있다면 isActive변수를 통해 움직인다.
				//플레이어에게 보인 NPC의 움직임 이벤트를 시작한다.

				//부하
				if (is_movable_monster(other)) {

					if (false == reinterpret_cast<movable_Npc*>(clients[other])->is_move)
					{
						reinterpret_cast<movable_Npc*>(clients[other])->is_move = true;
						timer_event t;
						t.ev = EVENT_NPC_MOVE;
						t.obj_id = other;
						t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
						timer_queue.push(t);
					}
					continue;
				}
				if (true == is_npc(other))continue;


				Client* otherPlayer = reinterpret_cast<Client*>(clients[other]);
				// 나한테 보이면 상대에게도 보인다는 뜻이니
				// 상대 뷰리스트도 확인한다.
				otherPlayer->vl.lock();

				// 상대 뷰리스트에 없으면
				if (0 == otherPlayer->viewlist.count(cl.index)) {
					// 뷰리스트에 추가하고 cl을 그리라고 전송
					otherPlayer->viewlist.insert(cl.index);
					otherPlayer->vl.unlock();
					send_put_object(other, cl.index);
				}
				// 상대 뷰리스트에 있으면 이동 패킷 전송
				else {
					otherPlayer->vl.unlock();
					send_move_object(other, cl.index);
				}

			}
			//계속 시야에 존재하는 플레이어 처리
			else {

				if (true == is_npc(other)) continue;
				Client* otherPlayer = reinterpret_cast<Client*>(clients[other]);
				otherPlayer->vl.lock();
				//상대방에 뷰리스트에 내가 있는지 확인
				if (0 != otherPlayer->viewlist.count(cl.index))
				{
					//있으면 이동
					otherPlayer->vl.unlock();

					send_move_object(other, cl.index);
				}
				else {
					//없으면 삽입
					otherPlayer->viewlist.insert(cl.index);
					otherPlayer->vl.unlock();

					send_put_object(other, cl.index);
				}
			}
		}


		// 움직임으로써 시야에서 빠진 플레이어 확인 및 제거
		for (int other : my_vl) {
			// nearlist에 없으면
			if (0 == nearlist.count(other)) {
				// 나한테서 지우고
				cl.vl.lock();
				cl.viewlist.erase(other);
				cl.vl.unlock();
				send_remove_object(cl.index, other);

				//npc는 view리스트를 가지고 있지 않다.
				if (true == is_npc(other)) {
					continue;
				}
				Client* otherPlayer = reinterpret_cast<Client*>(clients[other]);

				// 상대방도 나를 지운다.
				otherPlayer->vl.lock();
				//있다면 지움
				if (0 != otherPlayer->viewlist.count(cl.index)) {

					otherPlayer->viewlist.erase(cl.index);
					otherPlayer->vl.unlock();

					send_remove_object(other, cl.index);
				}
				else otherPlayer->vl.unlock();
			}

		}

	}
	break;
	case CS_PACKET_ATTACK:
	{
		cs_packet_attack* packet = reinterpret_cast<cs_packet_attack*>(p);
		if (cl.last_attack_time + std::chrono::seconds(1) > std::chrono::system_clock::now()) break;

		cl.last_attack_time = std::chrono::system_clock::now();

		// 때릴 몬스터 찾고 가장 가까운 적이 아니라 
		//int min_length = 0xffffff;
		//int min_id = -1;
		//for (auto i : cl.viewlist) {
		//	if (false == is_npc(i))continue;
		//	int length = abs(cl.x - clients[i]->x) + abs(cl.y - clients[i]->y);
		//	if (length < min_length) { min_length = length; min_id = i; }
		//}
		//if (min_id == -1) break;
		//
		//상하 좌우를 때려야됨
		cl.vl.lock();
		std::unordered_set cp_vl{ cl.viewlist };
		cl.vl.unlock();
		for (auto i : cp_vl) {
			if (false == is_npc(i))continue;
			if (false == can_attack(i, cl.index)) continue;
			// 때리고
			clients[i]->hp -= cl.power;
			
			// 죽었으면 지우고
			if (clients[i]->hp <= 0) {
				reinterpret_cast<Npc*>(clients[i])->target_id = -1;
				reinterpret_cast<movable_Npc*>(clients[i])->is_move = false;
				clients[i]->state_lock.lock();
				clients[i]->state = ST_FREE;
				clients[i]->state_lock.unlock();
				//어그로 로밍 몬스터는 2배의 경험치,
				if (is_movable_monster(i)) {
					int e = clients[i]->level * clients[i]->level * 80;
					cl.exp += e;
					
				}
				else if (is_non_movable_monster(i)) {
					int e = clients[i]->level * clients[i]->level * 40;
					cl.exp += e;
				
				}

				if (cl.exp > cl.maxexp) {
					cl.level++;
					cl.exp = 0;
					cl.hp = cl.level*MAX_HP;
					cl.maxhp = cl.level * MAX_HP;
					cl.maxexp = 100 * pow(2, cl.level);
					cl.power = cl.level * 15;

					
					// 레벨업 이펙트
					for (auto id : cp_vl) {
						if (false == is_player(id))continue;
						send_effect(id, client_id, LEVEL_UP, cl.level);
						send_status(id, client_id);
					}
					send_effect(client_id, client_id, LEVEL_UP, cl.level);
				}

				// 30초 뒤 리스폰
				timer_event t;
				t.ev = EVENT_NPC_RESPWAN;
				t.obj_id = i;
				t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(30);
				timer_queue.push(t);


				for (auto id : cp_vl) {
					if (false == is_player(id))continue;
					reinterpret_cast<Client*>(clients[id])->vl.lock();
					reinterpret_cast<Client*>(clients[id])->viewlist.erase(i);
					reinterpret_cast<Client*>(clients[id])->vl.unlock();
					send_remove_object(id, i); 
				}
				send_remove_object(client_id, i);
				send_status(client_id, client_id);
				cl.vl.lock();
				cl.viewlist.erase(i);
				cl.vl.unlock();
			}
			else {

				if (is_non_movable_monster(i) &&
					reinterpret_cast<Npc*>(clients[i])->target_id == -1) {
					timer_event t;
					t.ev = EVENT_NPC_ATTACK;
					t.obj_id = i;
					t.target_id = client_id;
					t.start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
					timer_queue.push(t);
				}
				reinterpret_cast<Npc*>(clients[i])->target_id = cl.index;

				for (auto id : cp_vl) {
					if (false == is_player(id))continue;
					send_status(id, i);
				}
				send_status(client_id, i);
			}

			
		}

		// 공격 이펙트
		for (auto id : cp_vl) {
			if (false == is_player(id))continue;
			send_effect(id, client_id, PLAYER_ATTACK, cl.power);
		}
		send_effect(client_id, client_id, PLAYER_ATTACK, cl.power);

	}
	break;
	case CS_PACKET_TELEPORT:
	{
		int x;
		int y;
		do {
			x = rand() % WORLD_HEIGHT;
			y = rand() % WORLD_WIDTH;
		} while (map_data[y * WORLD_HEIGHT + x] == 0);
		cl.x = x;
		cl.y = y;
		send_move_object(client_id, client_id);
	}
	break;
	case CS_PACKET_CHAT:
	{
		cs_packet_chat* packet = reinterpret_cast<cs_packet_chat*>(p);
		std::cout << packet->message << std::endl;

		Client* client = reinterpret_cast<Client*>(clients[client_id]);
		client->vl.lock();
		std::unordered_set<int> c_vl{ client->viewlist };
		client->vl.unlock();
		for (int i : c_vl) {
			if (false == is_player(i)) continue;
			send_chat_packet(i, client_id, packet->message);
		}
		send_chat_packet(client_id, client_id, packet->message);
	}
	break;
	default:
		std::cout << "이상한 패킷 수신\n";
		break;
	}
}

void Network::worker()
{
	while (true) {
		DWORD num_byte;
		LONG64 iocp_key;
		WSAOVERLAPPED* p_over;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &num_byte, (PULONG_PTR)&iocp_key, &p_over, INFINITE);

		int client_id = static_cast<int>(iocp_key);
		EXP_OVER* exp_over = reinterpret_cast<EXP_OVER*>(p_over);

		if (FALSE == ret) {

			error_display("GQCS", WSAGetLastError());

			disconnect_client(client_id);
			if (exp_over->_comp_op == OP_SEND)
				exp_over_pool.push(exp_over);
			continue;
		}

		switch (exp_over->_comp_op)
		{
		case OP_RECV:
		{
			if (num_byte == 0) {
				disconnect_client(client_id);
				continue;
			}
			//하나의 소켓에 대해 Recv호출은 언제나 하나 -> EXP_OVER(버퍼, WSAOVERLAPPED) 재사용 가능
			//패킷이 중간에 잘려진 채로 도착할 수 있다. -> 버퍼에 놔두었다가 다음에 온 데이터와 결합 -> 이전에 받은 크기를 기억해 그 위치부터 받기 시작하자
			//패킷이 여러 개 한번에 도착할 수 있다.	 -> 첫 번째가 사이즈이니 잘라서 처리하자
			Client& cl = *reinterpret_cast<Client*>(clients[client_id]);

			int remain_data = cl.prev_recv_size + num_byte;
			unsigned char* packet_start = exp_over->_net_buf;
			int packet_size = packet_start[0];

			while (packet_size <= remain_data) {
				process_packet(client_id, packet_start);
				remain_data -= packet_size;
				packet_start += packet_size;
				if (remain_data > 0)
					packet_size = packet_start[0];
			}


			cl.prev_recv_size = remain_data;
			if (remain_data) {
				memcpy_s(&exp_over->_net_buf, remain_data, packet_start, remain_data);
			}

			cl.do_recv();
		}
		break;
		case OP_SEND:
		{
			if (num_byte != exp_over->_wsa_buf.len) {
				std::cout << num_byte << " 송신버퍼 가득 참\n";
				std::cout << "클라이언트 연결 끊음\n";
				disconnect_client(client_id);
			}
			exp_over_pool.push(exp_over);
		}
		break;
		case OP_ACCEPT:
		{
			std::cout << "Accept Completed.\n";
			SOCKET c_socket = *(reinterpret_cast<SOCKET*>(exp_over->_net_buf)); // 확장 overlapped구조체에 넣어 두었던 소캣을 꺼낸다
			int new_id = get_new_id();
			if (-1 == new_id) continue;

			Client& cl = *(reinterpret_cast<Client*>(clients[new_id]));
			cl.x = 0;
			cl.y = 0;
			cl.index = new_id;
			cl.prev_recv_size = 0;
			cl.recv_over._comp_op = OP_RECV;
			cl.state = ST_ACCEPT;
			cl.recv_over._wsa_buf.buf = reinterpret_cast<char*>(cl.recv_over._net_buf);
			cl.recv_over._wsa_buf.len = sizeof(cl.recv_over._net_buf);
			ZeroMemory(&cl.recv_over._wsa_over, sizeof(cl.recv_over._wsa_over));
			cl.socket = c_socket;

			CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), g_h_iocp, new_id, 0);

			cl.do_recv();

			// exp_over 재활용
			ZeroMemory(&exp_over->_wsa_over, sizeof(exp_over->_wsa_over));
			c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
			//char* 버퍼를 socket*로 바꿔 소켓을 가르킬 수 있도록 한다. 소켓도 포인터인디?
			*(reinterpret_cast<SOCKET*>(exp_over->_net_buf)) = c_socket;

			AcceptEx(g_s_socket, c_socket, exp_over->_net_buf + sizeof(SOCKET), 0, sizeof(SOCKADDR_IN) + 16,
				sizeof(SOCKADDR_IN) + 16, NULL, &exp_over->_wsa_over);
		}
		break;
		case OP_NPC_MOVE:
			do_npc_move(client_id);
			exp_over_pool.push(exp_over);
			break;
		case OP_PLAYER_MOVE:
		{
			reinterpret_cast<PeaceMonster*>(clients[client_id])->lua_mutex.lock();
			lua_State* L = reinterpret_cast<PeaceMonster*>(clients[client_id])->L;
			lua_getglobal(L, "event_player_move");//lua함수
			int a = *reinterpret_cast<int*>(exp_over->_net_buf);
			lua_pushnumber(L, a);
			int err = lua_pcall(L, 1, 0, 0);
			if (err) {
				std::cout << "Error:" << lua_tostring(L, -1);
				lua_pop(L, 1);
			}
			reinterpret_cast<PeaceMonster*>(clients[client_id])->lua_mutex.unlock();
			exp_over_pool.push(exp_over);
		}
		break;

		default:
			break;
		}
	}
}