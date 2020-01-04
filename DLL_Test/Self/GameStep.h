#pragma once
#include "GameStruct.h"
#include <My/Common/Link.hpp>
#include <vector>
#include <list>

using namespace std;

typedef unsigned char      u_char;
typedef unsigned int       u_int;
typedef unsigned long long qword;

#define STEP_IS_MOVE(step) (step.OpCode==OP_MOVE)
#define STEP_IS_CLICK(step) (step.OpCode==OP_CLICK)
#define STEP_IS_SELECT(step) (step.OpCode==OP_SELECT)
#define STEP_IS_MOUMOVE(step) (step.OpCode==OP_MOUMOVE)
#define STEP_IS_WAIT(step) (step.OpCode==OP_KEY)
#define STEP_IS_WAIT(step) (step.OpCode==OP_WAIT)

#define USE_MY_LINK 1

enum STEP_CODE
{
	OP_UNKNOW  = 0x00,  // δ֪
	OP_MOVE,            // �ƶ�
	OP_MOVEFAR,         // ����
	OP_NPC,             // NPC�Ի�
	OP_SELECT,          // ѡ��Ի�ѡ��
	OP_MAGIC,           // ����
	OP_MAGIC_PET,       // ����-����
	OP_CRAZY,           // ��˦
	OP_CLEAR,           // ���
	OP_PICKUP,          // ����
	OP_CHECKIN,         // ����
	OP_USEITEM,         // ʹ����Ʒ
	OP_DROPITEM,        // ����
	OP_SELL,            // ������
	OP_BUTTON,          // �����ť
	OP_WAIT,            // �ȴ�
	OP_SMALL,           // С�Ų���
};

struct Point
{
	int x;
	int y;
	int flag; // 0-��Ļ���겻��Ҫת�� 1-������Ϸ������Ҫת��
};
// ��Ϸִ�в���
struct _step_
{
	STEP_CODE OpCode;   // ������

	DWORD     X;           // Ҫ������λ��X
	DWORD     Y;           // Ҫ������λ��Y
	DWORD     X2;
	DWORD     Y2;
	DWORD     NPCId;       // Ҫ�Ի���NPCID 
	CHAR      NPCName[32]; // Ҫ�Ի���NPC����
	DWORD     SelectNo;    // �Ի�ѡ������ 0��ʼ
	CHAR      Name[32];    // ���� ���ݲ�����������
	CHAR      Magic[32];   // ����
	DWORD     WaitMs;      // �ȴ����ٺ�����Ƿ�ȴ�������ȴ���ܿ����ж�������ȴ
	DWORD     OpCount;     // ��������
	DWORD     ButtonId;    // ��ťID
	int       SmallV;      // С�Ų���ֵ
	DWORD     Extra[8];    // ��չ
	__int64   ExecTime;    // ִ��ʱ��
	bool      Exec;        // ����ִ��
};

class Explode;
class GameStep
{
public:
	// ...
	GameStep();
#if USE_MY_LINK
	// ��ȡ����ִ�еĲ���
	_step_* Current(Link<_step_*>& link);
	// ��һ������ָ��
	_step_* Prev(Link<_step_*>& link);
	// �������ִ�в��� ������һ��
	_step_* CompleteExec(Link<_step_*>& link);
	// ��õ�ǰ���������
	STEP_CODE CurrentCode(Link<_step_*>& link);
	// �����һ���������
	STEP_CODE NextCode(Link<_step_*>& link);
#else
	// ��ȡ����ִ�еĲ���
	_step_* Current(vector<_step_*>& link);
	// �������ִ�в��� ������һ��
	_step_* CompleteExec(vector<_step_*>& link);
	// ��õ�ǰ���������
	STEP_CODE CurrentCode(vector<_step_*>& link);
	// �����һ���������
	STEP_CODE NextCode(vector<_step_*>& link);
#endif
	// ����ִ�в�������
	void ResetStep(int index = 0);
	// ��ʼ������
	bool InitSteps(const char* path, const char* file);
	// ��ʼ��ȥ��������
	int InitGoLeiMingSteps();
#if USE_MY_LINK
	// ��������
	int ParseStep(const char* data, Link<_step_*>& link);
#else
	// ��������
	int ParseStep(const char* data, vector<_step_*>& link);
#endif
private:
	// ת��ʵ��ָ��
	STEP_CODE TransFormOP(const char* data);
	// ת��ʵ������
	bool TransFormPos(const char* str, _step_& step);
	// ת��ʵ������
	bool TransFormPos(const char* str, DWORD& x, DWORD& y);
	// ת��ʵ�ʼ���
	bool TransFormMagic(Explode& line, _step_& step);
	// ת��ʵ�ʼ���
	MagicType TransFormMagic(const char* str);
	// ת��Wait����
	bool    TransFormWait(Explode& line, _step_& step);
	// С��
	void TransFormSmall(Explode& line, _step_& step);
	// ��Ӳ���
	void AddStep(_step_& step);
public:
#if USE_MY_LINK
	// ��Ϸ��������
	Link<_step_*> m_Step;
	// ���ȥ������������
	Link<_step_*> m_GoLeiMingStep;
#else
	// ��Ϸ��������
	vector<_step_*> m_Step;
	// ���ȥ������������
	vector<_step_*> m_GoLeiMingStep;
#endif
	// ��Ϸ��������
	int m_iStepCount = 0;
	// ��ǰִ�в�������
	int m_iStepIndex = 0;
	// ��ǰִ�в�������[m_GoLeiMingStep]
	int m_iStepGoLMIndex = 0;
	// ��Ϸ���˲������
	_step_* m_Steps;
	// ���迪ʼ����
	int m_iStepStartIndex = 0;
};