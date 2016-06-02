#pragma once
#include "stdafx.h"
#include "User.h"
extern User users[MAX_USER];

class RequestToDB
{
public:
	//유저 데이터로드
	static void RequestState(Overlap_ex * over, const WORD id, SQLHSTMT& hstmt)
	{
		SQLRETURN retcode;
		char client_ID[20];
		double x{}, y{}, z{}, s{};
		SQLINTEGER cbClientid = SQL_NTS;
		SQLINTEGER cb_ret[13] = { 0, };

		//아이디가져오기
		memcpy(client_ID, users[id].getUserID(), ID_LEN);

		auto ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(client_ID), 0, &client_ID, sizeof(client_ID), &cbClientid);
		retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC RequestState ?", SQL_NTS);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			//ID는 쓰지 않지만 받아오는걸로
			retcode = SQLBindCol(hstmt, 1, SQL_CHAR, &client_ID, sizeof(client_ID), &cb_ret[0]);
			retcode = SQLBindCol(hstmt, 2, SQL_INTEGER, &over->status.lv, sizeof(int), &cb_ret[1]);
			retcode = SQLBindCol(hstmt, 3, SQL_INTEGER, &over->status.hp, sizeof(int), &cb_ret[2]);
			retcode = SQLBindCol(hstmt, 4, SQL_INTEGER, &over->status.ap, sizeof(int), &cb_ret[3]);
			retcode = SQLBindCol(hstmt, 5, SQL_INTEGER, &over->status.damage, sizeof(int), &cb_ret[4]);
			retcode = SQLBindCol(hstmt, 6, SQL_INTEGER, &over->status.defense, sizeof(int), &cb_ret[5]);
			retcode = SQLBindCol(hstmt, 7, SQL_INTEGER, &over->status.exp, sizeof(int), &cb_ret[6]);
			retcode = SQLBindCol(hstmt, 8, SQL_INTEGER, &over->status.requestEXP, sizeof(int), &cb_ret[7]);
			retcode = SQLBindCol(hstmt, 9, SQL_DOUBLE, &x, sizeof(double), &cb_ret[9]);
			retcode = SQLBindCol(hstmt, 10, SQL_DOUBLE, &y, sizeof(double), &cb_ret[10]);
			retcode = SQLBindCol(hstmt, 11, SQL_DOUBLE, &z, sizeof(double), &cb_ret[11]);

			retcode = SQLFetch(hstmt);

			over->operation = OP_DB_EVENT;
			over->db_type = DB_QUERY::REQUEST_STATE;
			over->pos.x = x;
			over->pos.y = y;
			over->pos.z = z;

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				printf("RequestToDB::RequestState : %s is loaded\n", client_ID);
				over->isSuccess = true;
			}else if (retcode == SQL_NO_DATA){
				printf("RequestToDB::RequestState : %s is new\n", client_ID);
				over->isSuccess = false;
				//초기값으로 전송
				over->status.reset();
				over->pos = XMFLOAT3(0, 0, 0);
			}
			else {
				printf("RequestToDB::RequestState : %s is not loaded\n", client_ID);
			}

		}
	}
	//유저데이터 업데이트
	static void RequestUpdate(Overlap_ex * over, const WORD id, SQLHSTMT& hstmt)
	{
		SQLRETURN retcode;
		SQLINTEGER cbCheckName = SQL_NTS;
		char client_ID[20];
		DWORD level, hp, ap, damage, defense, exp, rexp, etype;
		double posX, posY, posZ, speed;
		User &player = users[id];

		//아이디가져오기
		memcpy(client_ID, player.getUserID(), ID_LEN);

		level = player.getStatus().lv;
		hp = player.getStatus().hp;
		ap = player.getStatus().ap;
		damage = player.getStatus().damage;
		defense = player.getStatus().defense;
		exp = player.getStatus().exp;
		rexp = player.getStatus().requestEXP;
		posX = player.getPos().x;
		posY = player.getPos().y;
		posZ = player.getPos().z;

		retcode = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, sizeof(client_ID), 0, &client_ID, sizeof(client_ID), &cbCheckName);
		retcode = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &level, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &hp, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &ap, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &damage, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 6, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &defense, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 7, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &exp, sizeof(int), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 8, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &rexp, sizeof(int), SQL_NULL_HANDLE);;
		retcode = SQLBindParameter(hstmt, 9, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &posX, sizeof(double), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 10, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &posY, sizeof(double), SQL_NULL_HANDLE);
		retcode = SQLBindParameter(hstmt, 11, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &posZ, sizeof(double), SQL_NULL_HANDLE);

		retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC RequestUpdate ?,?,?,?,?,?,?,?,?,?,?", SQL_NTS);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			printf("RequestToDB::RequestUpdate : %s is updated \n", client_ID);
		else
			printf("RequestToDB::RequestUpdate : %s is not updated \n", client_ID);
	}
};