#include "SoundManager.h"
#include <fmod_errors.h>

void SoundManager::Init( )
{
	//FMOD 초기화 
	//FMOD 함수는 모두 결과값을 리턴한다. 따라서 에러체크가 가능하다.
	r = System_Create( &m_pFmod );
	ErrorCheck( r );

	r = m_pFmod->init( MAX_SOUND_COUNT, FMOD_INIT_NORMAL, NULL );
	ErrorCheck( r );
	m_volume = 0.5f;

	for (int i = 0; i < MAX_SOUND_COUNT; i++)
	{
		m_pCh[i]->setVolume( m_volume );
	}
}

void SoundManager::Loading( )
{
	r = m_pFmod->createSound( "Sound/BGM.mp3", FMOD_LOOP_NORMAL, NULL, &m_pSound[BGM] );
	ErrorCheck( r );
}

void SoundManager::Play( int _type )
{
	m_pFmod->update( );
	r = m_pFmod->playSound( FMOD_CHANNEL_FREE, m_pSound[_type], false, &m_pCh[_type] );
	ErrorCheck( r );
}

void SoundManager::Stop( int _type )
{
	m_pCh[_type]->stop( );
}

void SoundManager::ErrorCheck( FMOD_RESULT _r )
{
	if (_r != FMOD_OK)
	{
		TCHAR szStr[256] = { 0 };
		MultiByteToWideChar( CP_ACP, NULL, FMOD_ErrorString( r ), -1, szStr, 256 );
	}
}

void SoundManager::IncreaseVolume( )
{
	m_volume += 0.03f;

	if (m_volume > 1.0f)
		m_volume = 1.0f;

	for (int i = 0; i < MAX_SOUND_COUNT; i++)
	{
		m_pCh[i]->setVolume( m_volume );
	}
}

void SoundManager::DecreaseVolume( )
{
	m_volume -= 0.03f;

	if (m_volume < 0.0f)
		m_volume = 0.0f;

	for (int i = 0; i < MAX_SOUND_COUNT; i++)
	{
		m_pCh[i]->setVolume( m_volume );
	}
}
