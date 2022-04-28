#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include<algorithm>

#include<queue>
#include<vector>
#include<cstdlib>

using namespace std;

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

#include "..\..\Server\Server\protocol.h"

sf::TcpSocket socket;


constexpr auto BUF_SIZE = 256;
constexpr auto SCREEN_WIDTH = 20;
constexpr auto SCREEN_HEIGHT = 20;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;
//constexpr auto BUF_SIZE = MAX_BUFFER;


const int MAX_NON_MOVABLE_MONSTER = 150'000;
const int MAX_AGRO_MONSTER = 25'000;
const int MAX_PEACE_MONSTER = 25'000;

constexpr int AGRO_MONSTER_ID_START = MAX_USER;
constexpr int AGRO_MONSTER_ID_END = MAX_USER + MAX_AGRO_MONSTER - 1;

constexpr int PEACE_MONSTER_ID_START = AGRO_MONSTER_ID_END + 1;
constexpr int PEACE_MONSTER_ID_END = PEACE_MONSTER_ID_START + MAX_PEACE_MONSTER - 1;

constexpr int NON_MOVABLE_MONSTER_ID_START = PEACE_MONSTER_ID_END + 1;
constexpr int NON_MOVABLE_MONSTER_ID_END = NON_MOVABLE_MONSTER_ID_START + MAX_NON_MOVABLE_MONSTER - 1;

int g_myid;
int g_x_origin;
int g_y_origin;

sf::RenderWindow* g_window;
sf::Font g_font;

const int sprite_width = 64;

const int red_spider_frame = 8;
const int player_frame = 3;
const int Head_hunt_frame = 3;
const int fly_frame = 2;


enum type { PLAYER, RED_SPIDER, FLY, HEAD_HUNT };

class UI {
	int hp;
	int maxhp;
	int level;
	int exp;
	int maxExp;
	vector<sf::Text> mess;
	chrono::system_clock::time_point m_mess_end_time;

	sf::Text m_level;

	sf::RectangleShape m_Exp;
	sf::RectangleShape m_Exp_background;
	sf::RectangleShape m_hp;
	sf::RectangleShape m_hp_background;

	const int ui_hp_length = 160;
	const int ui_exp_length = 100;

public:
	UI() {

	}
	void set_ui(int _hp, int _maxhp, int _level, int _exp) {
		hp = _hp;
		maxhp = _maxhp;
		level = _level;
		exp = _exp;
		maxExp = 100 * pow(2, level);

		m_Exp_background.setSize(sf::Vector2f(float(ui_exp_length), 5.f));
		m_Exp.setSize(sf::Vector2f(exp / (float)maxExp * float(ui_exp_length), 5.f));

		m_hp_background.setSize(sf::Vector2f(float(ui_hp_length), 16.f));
		m_hp.setSize(sf::Vector2f(hp / (float)maxhp * float(ui_hp_length), 16.f));

		m_Exp_background.setPosition(10, 76);
		m_hp_background.setPosition(10, 60);
		m_Exp.setPosition(10, 76);
		m_hp.setPosition(10, 60);

		m_level.setPosition(10, 90);
		m_level.setFont(g_font);
		m_level.setString(std::to_string(_level) + "Level");
		m_level.setFillColor(sf::Color(200, 255, 120));
		m_level.setStyle(sf::Text::Bold);
	}
	void draw() {

		g_window->draw(m_Exp_background);
		m_Exp.setFillColor(sf::Color(0, exp / (float)maxExp * 255, 0, 255));

		g_window->draw(m_Exp);


		g_window->draw(m_hp_background);
		m_hp.setFillColor(sf::Color(hp / (float)maxhp * 255, 0, 0, 255));

		g_window->draw(m_hp);
		g_window->draw(m_level);

		if (m_mess_end_time > chrono::system_clock::now()) {

			int y_pos = 0;
			for (auto m : mess) {
				y_pos++;
				m.setPosition(10, 100 + 20*y_pos);
				g_window->draw(m);
			}
			
		}
		else { mess.clear(); }
	}

	void set_mess(const char* str) {
		sf::Text chat;
		chat.setFont(g_font);
		wchar_t wmess[MAX_CHAT_SIZE];
		size_t mess_size;
		mbstowcs_s(&mess_size, wmess, MAX_CHAT_SIZE, str, MAX_CHAT_SIZE);
		chat.setString(wmess);
		chat.setFillColor(sf::Color(255, 255, 255));
		chat.setStyle(sf::Text::Bold);
		chat.setCharacterSize(20);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
		if (mess.size() > 10) {
			mess.erase(mess.begin());
		}
		mess.push_back(chat);
	}
};

const int hp_rect_size = 40;

class OBJECT {
public:
	bool m_showing;
	sf::Sprite m_sprite;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;

	sf::RectangleShape m_hp;
	sf::RectangleShape m_hp_background;
	float frame_time;
public:
	sf::Text m_name;
	int m_x, m_y;
	int maxhp = 40;
	int hp = maxhp;
	int level;

	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		set_name("NONAME");
		m_mess_end_time = chrono::system_clock::now();
		frame_time = 0;
		m_hp_background.setSize(sf::Vector2f(float(hp_rect_size), 5.f));
		m_hp.setSize(sf::Vector2f(float(hp_rect_size), 5.f));
		level = 1;
	}
	OBJECT() {
		m_showing = false;
	}
	virtual ~OBJECT() {

	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}
	void set_hp(int h, int mh) {
		hp = h;
		maxhp = mh;
		m_hp.setSize(sf::Vector2f(hp / float(maxhp) * hp_rect_size, 5.f));
	}
	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	virtual void animate(float elsaped) {};
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_x_origin) * 65.0f + 8;
		float ry = (m_y - g_y_origin) * 65.0f + 8;
		m_sprite.setPosition(rx, ry);
		g_window->draw(m_sprite);
		if (m_mess_end_time < chrono::system_clock::now()) {
			m_name.setPosition(rx - 10, ry - 20);
			g_window->draw(m_name);
		}
		else {
			m_chat.setPosition(rx - 10, ry - 20);
			g_window->draw(m_chat);
		}
		m_hp_background.setPosition(rx + 10, ry + 60);
		g_window->draw(m_hp_background);
		m_hp.setFillColor(sf::Color(hp / (float)maxhp * 255, 0, 0, 255));
		m_hp.setPosition(rx + 10, ry + 60);
		g_window->draw(m_hp);

	}

	void set_name(const char str[]) {
		m_name.setFont(g_font);
		m_name.setString(str);
		m_name.setFillColor(sf::Color(255, 255, 0));
		m_name.setStyle(sf::Text::Bold);
	}

	void set_chat(const char str[]) {
		m_chat.setFont(g_font);
		wchar_t mess[MAX_CHAT_SIZE];
		size_t mess_size;
		mbstowcs_s(&mess_size, mess, MAX_CHAT_SIZE, str, MAX_CHAT_SIZE);
		m_chat.setString(mess);
		m_chat.setFillColor(sf::Color(255, 255, 255));
		m_chat.setStyle(sf::Text::Bold);
		m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
	}
};

class Player : public OBJECT {
public:
	Player(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT(t, x, y, x2, y2) {};
	Player() {
	}
	virtual void animate(float elsaped)
	{
		if (false == m_showing) return;
		frame_time += elsaped * 10.f;
		int frame = frame_time;

		m_sprite.setTextureRect(sf::IntRect(sprite_width * (frame % player_frame), 0, sprite_width, sprite_width));
	}
};

class Fly : public OBJECT {
public:
	Fly(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT(t, x, y, x2, y2) {};
	virtual void animate(float elsaped)
	{
		if (false == m_showing) return;
		frame_time += elsaped * 10.f;
		int frame = frame_time;
		m_sprite.setTextureRect(sf::IntRect(sprite_width * (frame % fly_frame), 0, sprite_width, sprite_width));
	}
};

class Spider : public OBJECT {
public:
	Spider(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT(t, x, y, x2, y2) {};
	virtual void animate(float elsaped)
	{
		if (false == m_showing) return;
		frame_time += elsaped * 10.f;
		int frame = frame_time;

		m_sprite.setTextureRect(sf::IntRect(sprite_width * (frame % red_spider_frame), 0, sprite_width, sprite_width));
	}
};

class HeadHunt : public OBJECT {
public:
	HeadHunt(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT(t, x, y, x2, y2) {};
	virtual void animate(float elsaped)
	{
		if (false == m_showing) return;
		frame_time += elsaped * 10.f;
		int frame = frame_time;
		m_sprite.setTextureRect(sf::IntRect(sprite_width * (frame % Head_hunt_frame), 0, sprite_width, sprite_width));
	}
};


enum EFFECT_TYPE { PLAYER_ATTACK, MONSTER_ATTACK, LEVEL_UP };
class Effect {
public:
	int width, height;
	int u, v;
	int x_frame, y_frame;
	int cur_frame;
	int total_frame;
	float frame_time;

	int pos_x, pos_y;
	int type;
	sf::Sprite m_sprite;

	virtual bool show(float elapsed) = 0;
};
class Effect_Player_Attack : public Effect {
public:
	Effect_Player_Attack(sf::Texture& t, int _u, int _v, int _width, int _height, int _x_frame, int _y_frame)
	{
		frame_time = 0;

		width = _width;
		height = _height;
		x_frame = _x_frame;
		y_frame = _y_frame;
		u = _u;
		v = _v;
		total_frame = x_frame * y_frame;

		type = PLAYER_ATTACK;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(u, v, width, height));
		m_sprite.setScale(0.3f, 0.3f);
	}

	virtual bool show(float elapsed) {
		frame_time += elapsed * 20.f;
		int frame = frame_time;
		u = frame % x_frame;
		v = frame / x_frame;
		m_sprite.setPosition(pos_x, pos_y);
		m_sprite.setTextureRect(sf::IntRect(u* width, v* height, width, height));
		g_window->draw(m_sprite);
		if (u + v >= total_frame - 1) {
			return false;
		}
		return true;
	}
};
class Effect_Monster_Attack : public Effect {
public:
	Effect_Monster_Attack(sf::Texture& t, int _u, int _v, int _width, int _height, int _x_frame, int _y_frame)
	{
		frame_time = 0;

		width = _width;
		height = _height;
		x_frame = _x_frame;
		y_frame = _y_frame;
		u = _u;
		v = _v;
		total_frame = x_frame * y_frame;
		type = MONSTER_ATTACK;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(u, v, width, height));
	}

	virtual bool show(float elapsed) {
		frame_time += elapsed * 10.f;
		int frame = frame_time;
		u = frame % x_frame;
		v = frame / x_frame;
		m_sprite.setPosition(pos_x, pos_y);
		m_sprite.setTextureRect(sf::IntRect(u * width, v * height, width, height));
		g_window->draw(m_sprite);
		if (u + v >= total_frame - 1) {
			return false;
		}
		return true;
	}
};
class Effect_Level_Up : public Effect {
public:
	Effect_Level_Up(sf::Texture& t, int _u, int _v, int _width, int _height, int _x_frame, int _y_frame)
	{
		frame_time = 0;

		width = _width;
		height = _height;
		x_frame = _x_frame;
		y_frame = _y_frame;
		u = _u;
		v = _v;
		total_frame = x_frame * y_frame;
		type = LEVEL_UP;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(u, v, width, height));
		m_sprite.setScale(2.0f, 2.0f);
		m_sprite.setColor(sf::Color::Yellow);
	}

	virtual bool show(float elapsed) {
		frame_time += elapsed * 5.f;
		int frame = frame_time;
		u = frame % x_frame;
		v = frame / x_frame;
		m_sprite.setPosition(pos_x, pos_y);
		m_sprite.setTextureRect(sf::IntRect(u * width, v * height, width, height));
		g_window->draw(m_sprite);
		if (u + v >= total_frame - 1) {
			return false;
		}
		return true;
	}
};
UI g_ui;

Player avatar;
OBJECT* players[MAX_USER + MAX_NPC];// 최적화해주자

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* pieces;

sf::Texture* player;
sf::Texture* other_player;

sf::Texture* red_spider;
sf::Texture* fly;
sf::Texture* head_hunt;

sf::Texture* effect_attack;
sf::Texture* effect_monster_attack; // 
sf::Texture* effect_level_up;

const int MAX_EFFECT = 100;
const int PLAYER_ATTACK_EFFECT = 40;
const int MONSTER_ATTACK_EFFECT = 50;
const int LEVEL_UP_EFFECT = 10;

queue<Effect*> player_attack_effect;
queue<Effect*> monster_attack_effect;
queue<Effect*> level_up_effect;

vector<Effect*> play_effect;

char map_data[WORLD_WIDTH][WORLD_HEIGHT];
void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;

	player = new sf::Texture;
	other_player = new sf::Texture;

	red_spider = new sf::Texture;
	fly = new sf::Texture;
	head_hunt = new sf::Texture;

	effect_attack = new sf::Texture;
	effect_monster_attack = new sf::Texture;
	effect_level_up = new sf::Texture;

	if (false == g_font.loadFromFile("Room NO.703.ttf")) {
		cout << "Font Loading Error!\n";
		while (true);
	}
	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");

	player->loadFromFile("image//player.png");
	other_player->loadFromFile("image//other_player.png");

	red_spider->loadFromFile("image//red_spider.png");
	fly->loadFromFile("image//Fly.png");
	head_hunt->loadFromFile("image//Head_hunt.png");

	effect_attack->loadFromFile("image//Attack.png");
	effect_monster_attack->loadFromFile("image//hit.png");
	effect_level_up->loadFromFile("image//level_up.png");

	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = Player{ *player, 0, 0, sprite_width, sprite_width };

	for (int i = 0; i < PLAYER_ATTACK_EFFECT; ++i) {
		player_attack_effect.emplace(new Effect_Player_Attack(*effect_attack, 0, 0, 192, 192, 5, 3));
	}
	for (int i = 0; i < MONSTER_ATTACK_EFFECT; ++i) {
		monster_attack_effect.emplace(new Effect_Monster_Attack(*effect_monster_attack, 0, 0, 60, 90, 5, 3));
	}
	for (int i = 0; i < LEVEL_UP_EFFECT; ++i) {
		level_up_effect.emplace(new Effect_Level_Up(*effect_level_up, 0, 0, 70, 70, 5, 3));
	}


	for (int i = 0; i < MAX_USER; ++i) {
		players[i] = new Player{ *other_player, 0, 0, sprite_width, sprite_width };
	}
	for (int i = AGRO_MONSTER_ID_START; i <= AGRO_MONSTER_ID_END; ++i) {
		players[i] = new Spider{ *red_spider, 0, 0, sprite_width, sprite_width };
	}
	for (int i = PEACE_MONSTER_ID_START; i <= PEACE_MONSTER_ID_END; ++i) {
		players[i] = new Fly{ *fly, 0, 0, sprite_width, sprite_width };
	}

	for (int i = NON_MOVABLE_MONSTER_ID_START; i <= NON_MOVABLE_MONSTER_ID_END; ++i) {
		players[i] = new HeadHunt{ *head_hunt, 0, 0, sprite_width, sprite_width };
	}
	{
		using namespace std;
		ifstream in{ "Map//map.bin",ios::binary };

		int i = 0;
		while (in >> map_data[i / WORLD_HEIGHT][i % WORLD_WIDTH]) {
			i++;
		}
		//cout << i;
	}

}

void client_finish()

{
	for (auto* i : players) {
		delete i;
	}
	delete board;
	delete pieces;

	delete player;

	delete red_spider;
	delete fly;
	delete head_hunt;
}

void send_login_packet(string& name);
void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok* packet = reinterpret_cast<sc_packet_login_ok*>(ptr);
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		g_x_origin = packet->x - SCREEN_WIDTH / 2;
		g_y_origin = packet->y - SCREEN_WIDTH / 2;

		avatar.move(packet->x, packet->y);
		avatar.show();
	}
	break;
	case SC_PACKET_LOGIN_FAIL:
	{
		sc_packet_login_fail* packet = reinterpret_cast<sc_packet_login_fail*>(ptr);
		packet->reason;
		string name;
		cout << "로그인에 실패했습니다.\n해당 아이디를 이미 사용 중입니다.\n아이디 입력 : ";
		cin >> name;
		send_login_packet(name);
		avatar.set_name(name.c_str());
	}
	break;
	case SC_PACKET_PUT_OBJECT:
	{
		sc_packet_put_object* my_packet = reinterpret_cast<sc_packet_put_object*>(ptr);
		int id = my_packet->id;

		//players[id].set_name(my_packet->name);
		if (id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			avatar.show();
		}
		else if (id < MAX_USER) {
			players[id]->set_name(my_packet->name);
			players[id]->move(my_packet->x, my_packet->y);
			players[id]->show();
		}
		else { // NPC
			players[id]->set_name(my_packet->name);
			players[id]->move(my_packet->x, my_packet->y);
			players[id]->show();
		}
		break;
	}
	case SC_PACKET_MOVE:
	{
		sc_packet_move* my_packet = reinterpret_cast<sc_packet_move*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_x_origin = my_packet->x - SCREEN_WIDTH / 2;
			g_y_origin = my_packet->y - SCREEN_WIDTH / 2;
		}
		else if (other_id < MAX_USER) {
			players[other_id]->move(my_packet->x, my_packet->y);
		}
		else {
			players[other_id]->move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_PACKET_REMOVE_OBJECT:
	{
		sc_packet_remove_object* my_packet = reinterpret_cast<sc_packet_remove_object*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else if (other_id < MAX_USER) {
			players[other_id]->hide();
		}
		else {
			players[other_id]->hide();
		}
		break;
	}
	case SC_PACKET_CHAT:
	{
		sc_packet_chat* my_packet = reinterpret_cast<sc_packet_chat*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.set_chat(my_packet->message);
		}
		else if (other_id < MAX_USER) {
			players[other_id]->set_chat(my_packet->message);
		}
		else {
			players[other_id]->set_chat(my_packet->message);
		}
	}
	break;
	case SC_PACKET_STATUS_CHANGE:
	{
		sc_packet_status_change* my_packet = reinterpret_cast<sc_packet_status_change*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			g_ui.set_ui(my_packet->hp, my_packet->maxhp, my_packet->level, my_packet->exp);
			avatar.level = (my_packet->level);
			avatar.set_hp(my_packet->hp, my_packet->maxhp);


		}
		else {
			players[other_id]->level = (my_packet->level);
			players[other_id]->set_hp(my_packet->hp, my_packet->maxhp);
			players[other_id]->maxhp = (my_packet->maxhp);
		}
	}
	break;
	case SC_PACKET_EFFECT:
	{
		sc_packet_effect* packet = reinterpret_cast<sc_packet_effect*>(ptr);
		int t_id = packet->id;
		int effect_type = packet->effect_type;
		OBJECT* obj;
		if (t_id == g_myid) {
			obj = &avatar;
		}
		else {
			obj = players[t_id];
		}
		float rx = (obj->m_x - g_x_origin) * 65.0f + 8.0f;
		float ry = (obj->m_y - g_y_origin) * 65.0f + 8.0f;
		switch (effect_type)
		{
		case PLAYER_ATTACK:
		{
			Effect* e = player_attack_effect.front();
			e->pos_x = rx + 65.0f;
			e->pos_y = ry;
			play_effect.push_back(e);
			player_attack_effect.pop();

			e = player_attack_effect.front();
			e->pos_x = rx - 65.0f;
			e->pos_y = ry;
			play_effect.push_back(e);
			player_attack_effect.pop();

			e = player_attack_effect.front();
			e->pos_x = rx;
			e->pos_y = ry + 65.0f;
			play_effect.push_back(e);
			player_attack_effect.pop();

			e = player_attack_effect.front();
			e->pos_x = rx;
			e->pos_y = ry - 65.0f;
			play_effect.push_back(e);
			player_attack_effect.pop();

			char tstr[MAX_CHAT_SIZE];
			sprintf_s(tstr, "용사 %s가 주변을 공격 %d의 데미지", obj->m_name.getString().toAnsiString().c_str(), packet->info);
			g_ui.set_mess(tstr);
		}
		break;
		case MONSTER_ATTACK:
		{
			Effect* e = monster_attack_effect.front();
			e->pos_x = rx;
			e->pos_y = ry;
			play_effect.push_back(e);
			monster_attack_effect.pop();


			char tstr[MAX_CHAT_SIZE];
			sprintf_s(tstr, "몬스터가 용사 %s를 공격 %d의 데미지", obj->m_name.getString().toAnsiString().c_str(), packet->info);
			g_ui.set_mess(tstr);
		}
		break;
		case LEVEL_UP:
		{
			Effect* e = level_up_effect.front();
			e->pos_x = rx - 20.f;
			e->pos_y = ry - 20.f;
			play_effect.push_back(e);
			level_up_effect.pop();
			char tstr[MAX_CHAT_SIZE];
			sprintf_s(tstr, "용사 %s 레벨업!! %d -> %d", obj->m_name.getString().toAnsiString().c_str(), packet->info-1,packet->info);
			g_ui.set_mess(tstr);
		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

bool client_main(float elapsed)
{
	char net_buf[BUF_SIZE];
	size_t	received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result == sf::Socket::Disconnected)
	{
		wcout << L"서버 접속 종료.\n";
		return false;
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);



	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_x_origin;
			int tile_y = j + g_y_origin;
			if ((tile_x < 0) || (tile_y < 0)) continue;

			if (map_data[tile_y][tile_x] == 1) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}

	avatar.draw();
	avatar.animate(elapsed);
	for (auto pl : players) {
		pl->animate(elapsed);
		pl->draw();
	}
	vector<Effect*> tmp{ play_effect };
	for (auto* ef : tmp) {
		if (false == ef->show(elapsed)) {
			switch (ef->type) {
			case PLAYER_ATTACK:
				player_attack_effect.push(ef);
				break;
			case MONSTER_ATTACK:
				monster_attack_effect.push(ef);
				break;
			case LEVEL_UP:
				level_up_effect.push(ef);
				break;
			}
			auto p = find_if(play_effect.begin(), play_effect.end(), [&](Effect* rhs) {
				return rhs == ef;
				});
			(*p)->frame_time = 0;
			play_effect.erase(p);
		}
	}
	g_ui.draw();

	sf::Text text;
	text.setFont(g_font);
	char buf[100];
	sprintf_s(buf, "(%d, %d)", avatar.m_x, avatar.m_y);
	text.setString(buf);
	g_window->draw(text);
	return true;
}

void send_move_packet(char dr)
{
	cs_packet_move packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_MOVE;
	packet.direction = dr;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_login_packet(string& name)
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_LOGIN;
	strcpy_s(packet.name, name.c_str());
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_attack_packet()
{
	cs_packet_login packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_ATTACK;
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}

void send_chat_packet(const wchar_t* mess, int size)
{
	cs_packet_chat packet;
	packet.size = sizeof(packet);
	packet.type = CS_PACKET_CHAT;
	size_t mess_size;
	wcstombs_s(&mess_size, packet.message, MAX_CHAT_SIZE, mess, MAX_CHAT_SIZE);
	size_t sent = 0;
	socket.send(&packet, sizeof(packet), sent);
}



bool get_key_time(float& keytime, float keytimeMax = 1.f)
{
	if (keytime >= keytimeMax)
	{
		keytime = 0.f;
		return true;
	}
	return false;
}

void update_key_time(float& keytime)
{
	if (keytime < 1.f)
		keytime += 10.f * 1.f / 120.f;;
}

int main()
{
	wcout.imbue(locale("korean"));
	setlocale(LC_ALL, "korean");

	//sf::Socket::Status status = socket.connect("127.0.0.1", SERVER_PORT);

	//sf::Socket::Status status = socket.connect("121.139.87.12", 4000);

	string ip;
	int port;

	cout << "ip와 포트번호를 입력하세요 ex) 127.0.0.1 4000\n";
	cin >> ip >> port;

	sf::Socket::Status status = socket.connect(ip, port);

	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();
	string name{};
	cout << "ID를 입력하세요 : ";
	cin >> name;
	//string name{ "PL" };
	//auto tt = chrono::duration_cast<chrono::milliseconds>
	//	(chrono::system_clock::now().
	//		time_since_epoch()).count();
	//name += to_string(tt % 1000);
	send_login_packet(name);
	avatar.set_name(name.c_str());

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;

	float ft = 0;
	auto pre_t = chrono::system_clock::now();

	sf::String chat_mess{ "" };
	float dt = 1.f / 120.f;
	float keytime = 0.f;
	float keytimeMax;
	bool is_chatting = false;
	sf::RectangleShape chat_background;
	chat_background.setSize(sf::Vector2f(350, 35.f));

	while (window.isOpen())
	{
		auto cur_t = chrono::system_clock::now();
		float elapsed = chrono::duration_cast<chrono::milliseconds>(cur_t - pre_t).count() / float(1000);
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();


			{
				if (!is_chatting && event.type == sf::Event::KeyPressed) {
					int direction = -1;
					switch (event.key.code) {
					case sf::Keyboard::Left:
						direction = 2;
						break;
					case sf::Keyboard::Right:
						direction = 3;
						break;
					case sf::Keyboard::Up:
						direction = 0;
						break;
					case sf::Keyboard::Down:
						direction = 1;
						break;
					case sf::Keyboard::Space:
						send_attack_packet();
						break;
					case sf::Keyboard::Enter:
						is_chatting = true;
						chat_mess = "";
						break;
					case sf::Keyboard::Escape:
						window.close();
						break;
					}
					if (-1 != direction) send_move_packet(direction);
				}
				else if (is_chatting)
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::BackSpace) && get_key_time(keytime))
					{
						if (chat_mess.getSize() > 0) {
							chat_mess.erase(chat_mess.getSize() - 1); //마지막글자를 지운다.
						}
					}

					{
						if (event.type == sf::Event::TextEntered)
						{
							if (event.text.unicode == 8) //백스페이스
								continue;
							else if (chat_mess != "" && event.text.unicode == 13) {
								std::wstring w_mess = chat_mess.toWideString();

								send_chat_packet(w_mess.c_str(), w_mess.length());
								chat_mess = "";
								is_chatting = false;
							}
							else if (event.text.unicode == 13)
								continue;
							chat_mess += event.text.unicode;

						}
					}
				}
			}


		}

		ft += elapsed;

		window.clear();

		if (false == client_main(elapsed))
			window.close();
		if (is_chatting) {
			update_key_time(keytime);

			sf::Text text(chat_mess, g_font);
			//텍스트값 설정 (설정은 여기있을필요는 없음 외부로빼서 초기화해주면됨.)
			text.setPosition(WINDOW_WIDTH / 2.f - 150.f, 10);
			text.setCharacterSize(30);
			text.setStyle(sf::Text::Bold);
			text.setFillColor(sf::Color(0, 0, 0, 255));

			chat_background.setPosition(WINDOW_WIDTH / 2.f - 150.f, 10);
			chat_background.setFillColor(sf::Color(255, 255, 255, 128));
			window.draw(chat_background);
			//텍스트 그리기.
			window.draw(text);
		}
		window.display();

		pre_t = cur_t;
		if (chrono::system_clock::now() - pre_t < 16ms) {//30프레임

			this_thread::sleep_for(16ms - (chrono::system_clock::now() - cur_t));
			//cout << "update " << chrono::duration_cast<chrono::milliseconds>(end_t - start_t).count() << "ms\n";
		}
	}
	client_finish();

	return 0;
}