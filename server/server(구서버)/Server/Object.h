#pragma once

using namespace std;
using namespace DirectX;
enum
{
	FIRE,
	ICE,
	LIGHT
};


class Object
{
protected:
	//오브젝트 ID부여
	DWORD			id;
	//위치 방향 거리
	XMFLOAT3		mPosition;
	XMFLOAT3		mDirection;
	FLOAT			mDist;
	BYTE			mElementType;
	FLOAT			mSpeed;
	//Sector*			 currentSector;
	BYTE			 mAction;
	XMFLOAT3		 mTargetPos;
	XMFLOAT3		 mPrePos;
	float			 mNewSpeed;
	//atomic<bool>	 is_DeadReckoning;
	//pair<WORD, WORD> tmpList[TMP_SIZE];

	//
	vector<WORD>	 nearList;	// 주변 플레이어 id list
	//vector<Sector*>	 nearSectors;	// 플레이어 주변 Sector
public:
	Object();
	Object(const XMFLOAT3& pos, const FLOAT speed);
	~Object();

	//ViewList		 viewList;

	// object vector accessor
	const XMFLOAT3	getPos()const  { return mPosition; }
	const void	setPos(const XMFLOAT3 pos) { mPosition = pos; }
	const XMFLOAT3	getDir()const  { return mDirection; }
	void setDir(const XMFLOAT3 dir) { mDirection = dir; }

	void setPrePos(const XMFLOAT3 pos){ mPrePos = pos; }
	const XMFLOAT3 getPrePos()const { return mPrePos; }

	// player id accessor
	DWORD					getID()const { return id; }
	void					setID(const DWORD id) { this->id = id; }
	// speed
	FLOAT					getSpeed()const{ return mSpeed; }
	void					setSpeed(const FLOAT sp){ mSpeed = sp; }
	// sector
	//Sector*				getCurrentSector() { return currentSector; }
	//void					setCurrentSector(Sector* currentSector) { this->currentSector = currentSector; }
	// dist
	FLOAT					getDist()const { return mDist; }
	void					setDist(const FLOAT di) { mDist = di; }
	// elementType
	BYTE					getEleType()const { return mElementType; }
	void					setEleType(const BYTE ele) { mElementType = ele; }

	/////////////////// view check /////////////////////
	void					updateNearList();
	// nearList accessor
	const vector<WORD>&		getNearList(){ return nearList; }
	// 올바른 좌표를 가진 Rect인지 검사
	void					correctRect(RECT& rt);
	// action
	void					setAction(const BYTE ac) { mAction = ac; }
	const BYTE				getAction()const { return mAction; }
	// targetPos
	const XMFLOAT3&			getTargetPos(){ return mTargetPos; }
	void					setTargetPos(const XMFLOAT3& tp) { mTargetPos = tp; }
	// move type
	virtual const BYTE		getMoveType()const { return 1; }
	virtual void			setMoveType(const BYTE mt) {}
	// DeadReckoning
	void					ObjectDeadReckoning(const float);
	//void					setDeadReckoning(const bool tf) { is_DeadReckoning = tf; }
	//bool					getDeadReckoning()const { return is_DeadReckoning; }

	void setObjectVec(const XMFLOAT3& pos, const XMFLOAT3& dir, const float di)
	{
		mPosition = pos; mDirection = dir; mDist = di;
	}

	/*void updateViewList()
	{
		viewList.updateViewList(nearList);
	}*/
};

