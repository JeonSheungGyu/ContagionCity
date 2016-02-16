// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
#include <mmsystem.h>
#include <math.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <D3DX10math.h>
#include <xnamath.h>
#include "MathHelper.h"

#define FRAME_BUFFER_WIDTH	1024
#define FRAME_BUFFER_HEIGHT	768
#define VS_SLOT_CAMERA	0x00
#define VS_SLOT_WORLD_MATRIX 0x01
#define VS_SLOT_COLOR	0x02

//조명과 재질을 설정하기 위한 상수 버퍼의 슬롯 번호를 정의한다. 
#define PS_SLOT_LIGHT			0x00
#define PS_SLOT_MATERIAL		0x01
//텍스쳐와 샘플러 상태를 설정하기 위한 쉐이더의 슬롯 번호를 정의한다. 
#define PS_SLOT_TEXTURE		0x00
#define PS_SLOT_SAMPLER_STATE		0x00

enum ObjectLayer{ LAYER_PLAYER = 0, LAYER_NPC, LAYER_ENEMY, LAYER_BACKGROUND };
enum ObjectType_BACKGROUND { BACK_GROUND = 10, BACK_SHOP, BACK_WALL, BACK_FENCE, BACK_SHALTER };
enum ObjectType_ENEMY { ENEMY_ZOMBIE = 20, ENEMY_BOSS, };
enum ObjectType_NPC { NPC_QUEST = 30, NPC_SHOP };
enum ObjectType_Player{ PLAYER_MAN, PLAYER_WAMEN};