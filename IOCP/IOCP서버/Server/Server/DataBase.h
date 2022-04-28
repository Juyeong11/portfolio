#pragma once

#include "Network.h"
void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

const int MAX_NAME_LEN = 20;
struct PlayerDataTableRow {
	SQLWCHAR p_name[MAX_NAME_SIZE];

	SQLINTEGER p_level, p_exp;
	SQLINTEGER p_x, p_y;
	SQLINTEGER p_using;

	SQLLEN cb_id = 0, cb_name = 0, cb_level = 0, cb_exp = 0;
	SQLLEN cb_x = 0, cb_y = 0, cb_using = 0;
};
struct PlayerData {
	std::wstring name;
	int level, exp;
	int x, y;

};

class DataBase
{
public:
	PlayerDataTableRow PlayerDataSchema;
	//std::vector<PlayerData> db_player_data;
	//std::map<int, PlayerData> client_map_data;
	SQLHENV henv;
	SQLHDBC hdbc;

	bool isConnect;

	DataBase();
	~DataBase();

	bool check_player_data(PlayerData& Player);
	void insert_player_data(const PlayerData& Player);
	void update_player_data(const GameObject * const Player);

};

