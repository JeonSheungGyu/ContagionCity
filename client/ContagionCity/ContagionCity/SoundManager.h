#pragma once
#include "fmod.hpp"

using namespace FMOD;

#define MAX_SOUND_COUNT 40

enum {
	SOUND_BGM, SOUND_ATTACK,
};

class SoundManager
{
private:
	SoundManager( ){ ; }

public:
	static SoundManager *Instance;
	static SoundManager *GetInstance( ){ if (Instance == NULL) Instance = new SoundManager; return Instance; }
	~SoundManager( ){ delete Instance; }

	System* m_pFmod;
	Channel* m_pCh[MAX_SOUND_COUNT];
	Sound* m_pSound[MAX_SOUND_COUNT];
	FMOD_RESULT r;
	float m_volume;

	void Init( );
	void Loading( );
	void Loading( const char* pFileName, FMOD_MODE mode, int SoundType );
	void Play( int _type );
	void Stop( int _type );
	void ErrorCheck( FMOD_RESULT _r );
	void IncreaseVolume( );
	void DecreaseVolume( );
};
