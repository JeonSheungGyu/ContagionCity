#pragma once
#include "stdafx.h"

typedef struct _SLOT_USER_DATA
{
	WCHAR		UserID[32];
	DWORD_PTR	SessionID;
	WCHAR		VirtualAddress[32];
	USHORT		VirtualPort;
	WCHAR		RealAddress[32];
	USHORT		RealPort;
	BOOL		IsReady;
} SLOT_USER_DATA;

typedef enum TCP_PROTOCOL
{
	PT_VERSION	= 0x1000000,

#ifdef _INCLUDE_DATABASE
	PT_CHANNEL_NICKNAME,
	/*
	WCHAR			:	USER_ID[32]
	WCHAR			:	NICK_NAME[32]
	*/
	PT_CHANNEL_NICKNAME_SUCC_U,
	/*
	WCHAR			:	USER_ID[32]
	WCHAR			:	NICK_NAME[32]
	*/
	PT_CHANNEL_NICKNAME_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_REQ_USERINFO,
	/*
	WCHAR			:	USER_ID[32]
	*/
	PT_CHANNEL_REQ_USERINFO_SUCC_U,
	/*
	WCHAR			:	NICK_NAME[32]
	INT64			:	GAME_MONEY
	INT64			:	EXP
	INT64			:	CHAR_INDEX_1
	INT64			:	CHAR_INDEX_2
	INT64			:	CHAR_INDEX_3
	*/
	PT_CHANNEL_REQ_USERINFO_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_REQ_CHARINFO,
	/*
	USHORT			:	CHAR_SLOT
	*/
	PT_CHANNEL_REQ_CHARINFO_SUCC_U,
	/*
	INT64			:	CHAR_INDEX
	WCHAR			:	NAME[32]
	INT				:	JOB
	INT				:	TYPE
	INT64			:	EXP
	INT				:	HEAD_TYPE
	INT				:	CHEST_TYPE
	INT				:	PANTS_TYPE
	INT				:	GLOVES_TYPE
	INT				:	SOCKS_TYPE
	INT				:	SHOES_TYPE
	INT				:	ACCESSORY_TYPE
	INT				:	SKILL_TYPE_1
	INT				:	SKILL_TYPE_2
	INT				:	SKILL_TYPE_3
	INT				:	SPECIAL_SHOOT_TYPE_1
	INT				:	SPECIAL_SHOOT_TYPE_2
	INT				:	SPECIAL_SHOOT_TYPE_3
	*/
	PT_CHANNEL_REQ_CHARINFO_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_CHECK_CHARACTER_NAME,
	/*
	WCHAR			:	NAME[32]
	*/
	PT_CHANNEL_CHECK_CHARACTER_NAME_SUCC_U,
	/*
	*/
	PT_CHANNEL_CHECK_CHARACTER_NAME_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_CREATE_CHARACTER,
	/*
	WCHAR			:	NAME[32]
	INT				:	JOB
	INT				:	TYPE
	USHORT			:	SLOT
	*/
	PT_CHANNEL_CREATE_CHARACTER_SUCC_U,
	/*
	*/
	PT_CHANNEL_CREATE_CHARACTER_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_EQUIP,
	/*
	INT64			:	CHAR_INDEX
	USHORT			:	EQUIP_LOCATION
	INT				:	ITEM_TYPE
	*/
	PT_CHANNEL_EQUIP_SUCC_U,
	/*
	*/
	PT_CHANNEL_EQUIP_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_CHANNEL_CHARACTER_SELECT,
	/*
	USHORT			:	SLOT
	*/
	PT_CHANNEL_CHARACTER_SELECT_SUCC_U,
	/*
	*/
	PT_CHANNEL_CHARACTER_SELECT_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
#endif

	PT_CHANNEL_ENTER,
	/*
	WCHAR			:	USER_ID[32]
	WCHAR			:	VIRTUAL_ADDRESS[32]
	USHORT			:	VIRTUAL_PORT
	*/
	PT_CHANNEL_ENTER_SUCC_U,
	/*
	DWORD_PTR		:	SESSION_ID
	WCHAR			:	VIRTUAL_ADDRESS[32]
	USHORT			:	VIRTUAL_PORT
	WCHAR			:	REAL_ADDRESS[32]
	USHORT			:	REAL_PORT
	*/
	PT_CHANNEL_ENTER_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_ROOM_QUICK_JOIN,
	/*
	*/
	PT_ROOM_QUICK_JOIN_SUCC_U,
	/*
	*/
	PT_ROOM_QUICK_JOIN_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
	
	PT_ROOM_ENTER_M,
	/*
	DWORD			:	SLOT_POSITION
	WCHAR			:	USER_ID[32]
	DWORD_PTR		:	SESSION_ID
	WCHAR			:	VIRTUAL_ADDRESS[32]
	USHORT			:	VIRTUAL_PORT
	WCHAR			:	REAL_ADDRESS[32]
	USHORT			:	REAL_PORT
	*/

	PT_ROOM_REQ_INFO,
	/*
	*/
	PT_ROOM_REQ_INFO_SUCC_U,
	/*
	DWORD			:	ROOM_INDEX
	WCHAR			:	TITLE[32]
	DWORD			:	MAP_INDEX
	USHORT			:	CURRENT_USER_COUNT
	DWORD_PTR		:	ROOT_USER_SESSION_ID
	SLOT_USER_DATA	:	DATA[8]
	*/
	PT_ROOM_REQ_INFO_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_ROOM_LEAVE,
	/*
	*/
	PT_ROOM_LEAVE_SUCC_U,
	/*
	*/
	PT_ROOM_LEAVE_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_ROOM_LEAVE_M,
	/*
	DWORD_PTR		:	SESSION_ID
	DWORD_PTR		:	ROOT_USER_SESSION_ID
	*/

	PT_ROOM_MAP_CHANGE,
	/*
	DWORD			:	MAP_INDEX
	*/
	PT_ROOM_MAP_CHANGE_SUCC_U,
	/*
	*/
	PT_ROOM_MAP_CHANGE_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
	PT_ROOM_MAP_CHANGE_M,
	/*
	DWORD			:	MAP_INDEX
	*/

	PT_ROOM_CHATTING,
	/*
	WCHAR			:	CHAT[256]
	*/
	PT_ROOM_CHATTING_SUCC_U,
	/*
	*/
	PT_ROOM_CHATTING_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_ROOM_CHATTING_M,
	/*
	DWORD_PTR		:	SESSION_ID
	WCHAR			:	CHAT[256]
	*/

	PT_ROOM_READY,
	/*
	BOOL			: READY
	*/
	PT_ROOM_READY_SUCC_U,
	/*
	*/
	PT_ROOM_READY_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/

	PT_ROOM_READY_M,
	/*
	DWORD_PTR		:	SESSION_ID
	BOOL			:	READY
	*/

	PT_ROOM_START,
	/*
	*/
	PT_ROOM_START_SUCC_U,
	/*
	*/
	PT_ROOM_START_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
	PT_ROOM_START_M,
	/*
	*/

	PT_GAME_LOAD_COMPLETE,
	/*
	*/
	PT_GAME_LOAD_COMPLETE_SUCC_U,
	/*
	*/
	PT_GAME_LOAD_COMPLETE_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
	PT_GAME_LOAD_COMPLETE_M,
	/*
	DWORD_PTR		:	SESSION_ID
	*/
	PT_GAME_ALL_LOAD_COMPLETE_M,
	/*
	*/

	PT_GAME_INTRO_COMPLETE,
	/*
	*/
	PT_GAME_INTRO_COMPLETE_SUCC_U,
	/*
	*/
	PT_GAME_INTRO_COMPLETE_FAIL_U,
	/*
	DWORD			:	ERROR_CODE
	*/
	PT_GAME_INTRO_COMPLETE_M,
	/*
	DWORD_PTR		:	SESSION_ID
	*/
	PT_GAME_ALL_INTRO_COMPLETE_M,
	/*
	*/

	PT_GAME_START_M,
	/*
	*/

	PT_GAME_END_M,
	/*
	*/
	PT_GAME_PC_INFO_CHANGED_M,
	/*
	DWORD_PTR		:	SESSION_ID
	DWORD			:	HP
	DWORD			:	AP
	*/

	PT_END
};