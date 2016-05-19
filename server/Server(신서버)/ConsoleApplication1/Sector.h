#pragma once
#include "stdafx.h"

class Sector
{
	std::vector<DWORD> inPlayers;

public:
	int number;
	Sector();
	~Sector();

	bool					erasePlayer(const DWORD id);
	bool					insertPlayer(const DWORD id);

	const std::vector<DWORD>&		const getPlayers() { 
		
		return inPlayers; 
	}
	
	static bool				isinView( float x1, float y1, float x2, float y2);
	static bool				isinMonsterView(float x1, float y1, float x2, float y2);
	static CRITICAL_SECTION sCS;
};