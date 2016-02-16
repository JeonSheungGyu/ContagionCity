// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>

// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
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

//����� ������ �����ϱ� ���� ��� ������ ���� ��ȣ�� �����Ѵ�. 
#define PS_SLOT_LIGHT			0x00
#define PS_SLOT_MATERIAL		0x01
//�ؽ��Ŀ� ���÷� ���¸� �����ϱ� ���� ���̴��� ���� ��ȣ�� �����Ѵ�. 
#define PS_SLOT_TEXTURE		0x00
#define PS_SLOT_SAMPLER_STATE		0x00

enum ObjectLayer{ LAYER_PLAYER = 0, LAYER_NPC, LAYER_ENEMY, LAYER_BACKGROUND };
enum ObjectType_BACKGROUND { BACK_GROUND = 10, BACK_SHOP, BACK_WALL, BACK_FENCE, BACK_SHALTER };
enum ObjectType_ENEMY { ENEMY_ZOMBIE = 20, ENEMY_BOSS, };
enum ObjectType_NPC { NPC_QUEST = 30, NPC_SHOP };
enum ObjectType_Player{ PLAYER_MAN, PLAYER_WAMEN};