#include"stdfx.h"
#include "DataBase.h"


void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{

	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

DataBase::DataBase() {
	setlocale(LC_ALL, "korean");
	isConnect = false;
	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (false == (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		std::cout << "SQLHandle 생성 실패\n";
		return;
	}

	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (false == (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		std::cout << "ODBC버전 선택 실패\n";
		return;
	}
	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	ret = SQLConnect(hdbc, (SQLWCHAR*)L"GamServer", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	if (false == (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)) {
		std::cout << "ODBC 연결 실패\n";
		return;
	}
	isConnect = true;

}

DataBase::~DataBase() {
	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

bool DataBase::check_player_data(PlayerData& Player)
{
	if (!isConnect) { 
		Player.exp = 0;
		Player.level = 1;
		Player.name = L"happy";
		Player.x = 0;
		Player.y = 0;
		return true; 
	}
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	std::wstring command = std::format(L"EXEC check_user '{}'", Player.name);
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)command.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

		// Bind columns 1, 2, and 3  
		// 미리 읽어둘 변수를 bind해준다.
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &PlayerDataSchema.p_name, MAX_NAME_SIZE, &PlayerDataSchema.cb_name);
		retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &PlayerDataSchema.p_level, 4, &PlayerDataSchema.cb_level);
		retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &PlayerDataSchema.p_exp, 10, &PlayerDataSchema.cb_exp);
		retcode = SQLBindCol(hstmt, 4, SQL_C_LONG, &PlayerDataSchema.p_x, 10, &PlayerDataSchema.cb_x);
		retcode = SQLBindCol(hstmt, 5, SQL_C_LONG, &PlayerDataSchema.p_y, 10, &PlayerDataSchema.cb_y);
		retcode = SQLBindCol(hstmt, 6, SQL_C_LONG, &PlayerDataSchema.p_using, 1, &PlayerDataSchema.cb_using);

		// Fetch and print each row of data. On an error, display a message and exit.  
		for (int i = 0; ; i++) {
			retcode = SQLFetch(hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				Player.name = PlayerDataSchema.p_name;
				Player.level = PlayerDataSchema.p_level;
				Player.exp = PlayerDataSchema.p_exp;
				Player.x = PlayerDataSchema.p_x;
				Player.y = PlayerDataSchema.p_y;

				int is_using = PlayerDataSchema.p_using;
				SQLCancel(hstmt);
				SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
				
				if(is_using)
					return false;
				return true;

			}
			else {
				break;
			}
		}
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		
		Player.level = 1;
		Player.exp = 0;
		Player.x = 0;
		Player.y = 0;
		insert_player_data(Player);
		return true;

	}
	else {

		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		
		return false;
	}



}

void DataBase::insert_player_data(const PlayerData& Player)
{
	if (!isConnect) {
		return ;
	}

	SQLHSTMT hstmt = 0;
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	std::wstring command = std::format(L"EXEC insert_new_player '{}'", Player.name);

	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)command.c_str(), command.length());
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	}
	else {

		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	// Process data  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	}
}

void DataBase::update_player_data(const GameObject* const Player)
{
	if (!isConnect) {
		return;
	}
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	PlayerData p;
	p.name = std::wstring(Player->name, &Player->name[strlen(Player->name)]);
	p.exp = Player->exp;
	p.level = Player->level;
	p.x = Player->x;
	p.y = Player->y;

	std::wstring command = std::format(L"EXEC update_player_pos '{}', {}, {}, {}, {}", p.name, p.level, p.exp, p.x, p.y);
	
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)command.c_str(), command.length());
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
	}
	else {

		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
	}
	// Process data  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}
}