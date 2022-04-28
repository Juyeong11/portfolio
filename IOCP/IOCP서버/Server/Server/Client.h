#pragma once

enum COMP_OP { OP_RECV, OP_SEND, OP_ACCEPT, OP_NPC_MOVE, OP_PLAYER_MOVE };


class EXP_OVER {
public:
	WSAOVERLAPPED	_wsa_over;
	COMP_OP			_comp_op;
	WSABUF			_wsa_buf;
	unsigned char	_net_buf[BUFSIZE];
public:
	EXP_OVER(COMP_OP comp_op, char num_bytes, void* mess) : _comp_op(comp_op)
	{
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}

	EXP_OVER(COMP_OP comp_op) : _comp_op(comp_op) {}

	EXP_OVER()
	{
		_comp_op = OP_RECV;
	}

	~EXP_OVER()
	{
	}

	void set_exp(COMP_OP comp_op, char num_bytes, void* mess)
	{
		_comp_op = comp_op;
		ZeroMemory(&_wsa_over, sizeof(_wsa_over));
		_wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
		_wsa_buf.len = num_bytes;
		memcpy(_net_buf, mess, num_bytes);
	}
};

enum STATE { ST_FREE, ST_ACCEPT, ST_INGAME };

const int MAX_HP = 40;
class GameObject {
public:
	char name[MAX_NAME_SIZE];
	int		index;
	short	x, y;
	int		type;

	int hp;
	int maxhp;
	int level;
	int exp;
	int maxexp;
	int power;
	//volatile해줘야 한다.
	volatile STATE state;
	std::mutex state_lock;

	int		last_move_time;

	std::atomic_bool is_heal;
	std::chrono::system_clock::time_point		last_attack_time;
	std::chrono::system_clock::time_point		mlast_move_time;
	GameObject() :state(ST_FREE) {
		hp = MAX_HP;
		maxhp = MAX_HP;
		level = 1;
		exp = 0;
		maxexp = 100;

		power = level * 5;
	}
	void set_staus(int _hp, int _level, int _exp) {
		hp = _hp;
		level = _level;
		exp = _exp;
		maxhp = level* MAX_HP;
		maxexp = 100 * pow(2, _level);
		power = level * 15;
	}

	bool heal() {
		hp += maxhp * 0.1f;
		if (hp > maxhp) { 
			hp = maxhp;
			return false;
		}
		return true;
	}
};
class Npc: public GameObject {
public:
	std::atomic_int target_id;
	Npc(){
		target_id = -1;
	}
};

class PathFinder;
class movable_Npc : public Npc {
public:
	PathFinder* Astar = nullptr;
	std::atomic_bool is_move = false;

	movable_Npc(char* pp_data);
	~movable_Npc();
};
class non_movable_Npc : public Npc {
public:
	non_movable_Npc() {

	}
};


class AgroMonster : public movable_Npc
{
public:
	AgroMonster(char* pp_data) : movable_Npc(pp_data) {};
	~AgroMonster() {};
};
class PeaceMonster : public movable_Npc
{
public:
	std::mutex lua_mutex;
	lua_State* L = nullptr;
	PeaceMonster(char* pp_data) : movable_Npc(pp_data) {};
	~PeaceMonster() {};
};


const int RANGE = 7;// test를 위한 거리
class Client : public GameObject
{
public:
	std::unordered_set<int> viewlist;
	std::mutex vl;


	EXP_OVER recv_over;
	SOCKET socket; // 재사용 하기 때문에 data race -> state로 보호



	int		prev_recv_size;
public:

	Client()
	{
		x = 0;
		y = 0;
		prev_recv_size = 0;
	}

	~Client()
	{
		closesocket(socket);
	}




	void do_recv();
	void do_send(EXP_OVER* ex_over);

};

