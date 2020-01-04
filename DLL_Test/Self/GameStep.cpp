#include "GameStep.h"
#include "Game.h"
#include <ShlObj_core.h>
#include <stdio.h>
#include <memory.h>
#include <My/Common/mystring.h>
#include <My/Common/OpenTextFile.h>
#include <My/Common/Explode.h>

#define ZeroStep(p) memset(p, 0, sizeof(_step_))
#define InitStep(n) _step_ n; ZeroStep(&n);

#define MAX_STEP 512

// ...
GameStep::GameStep()
{
	//InitSteps();
}

#if USE_MY_LINK
// 获取当前可执行的步骤
_step_* GameStep::Current(Link<_step_*>& link)
{
	NODE<_step_*>* p = link.Current();
	
	return p ? p->value : nullptr;
}

// 上一个链表指针
_step_ * GameStep::Prev(Link<_step_*>& link)
{
	NODE<_step_*>* p =  link.Prev();
	return p ? p->value : nullptr;
}

// 完成正在执行步骤 返回下一个
_step_* GameStep::CompleteExec(Link<_step_*>& link)
{
	link.Next();
	//INLOGVARN(32, "Index:%d", m_iStepIndex);
	return Current(link);
}

// 获得当前步骤操作码
STEP_CODE GameStep::CurrentCode(Link<_step_*>& link)
{
	_step_* pStep = Current(link);
	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// 获得下一步骤操作码
STEP_CODE GameStep::NextCode(Link<_step_*>& link)
{
	return OP_UNKNOW;
}
#else
// 获取当前可执行的步骤
_step_* GameStep::Current(vector<_step_*>& link)
{
	if (m_iStepIndex >= link.size())
		return nullptr;

	return link[m_iStepIndex];
}

// 完成正在执行步骤 返回下一个
_step_* GameStep::CompleteExec(vector<_step_*>& link)
{
	m_iStepIndex++;
	//INLOGVARN(32, "Index:%d", m_iStepIndex);
	return Current(link);
}

// 获得当前步骤操作码
STEP_CODE GameStep::CurrentCode(vector<_step_*>& link)
{
	_step_* pStep = Current(link);
	return pStep ? pStep->OpCode : OP_UNKNOW;
}

// 获得下一步骤操作码
STEP_CODE GameStep::NextCode(vector<_step_*>& link)
{
	return OP_UNKNOW;
}
#endif

// 初始化步骤
bool GameStep::InitSteps(const char* path, const char* file)
{
	char fb_file[255];
	sprintf_s(fb_file, "%s\\%s", path, file);
	
	printf("刷副本流程文件:%s\n", fb_file);
	OpenTextFile hfile;
	if (!hfile.Open(fb_file)) {
		printf("找不到'%s'文件！！！", fb_file);
		return false;
	}

	// 初始化神殿去雷鸣步骤
	InitGoLeiMingSteps();

	int i = 0, index = 0;
	int length = 0;
	char data[128];
	while ((length = hfile.GetLine(data, 128)) > -1) {
		//printf("length:%d\n", length);
		if (length == 0) {
			continue;
		}
		char log[64];

		int ret = ParseStep(trim(data), m_Step);
		if (ret == 0)
			break;
		if (ret > 0) {
			if (ret == 2) {
				index = i;
			}
			i++;
		}
	}
#if USE_MY_LINK
	printf("执行流程数量：%d\n", m_Step.Count());
#else
	printf("执行流程数量：%d\n", m_Step.size());
#endif
	ResetStep(index);

	hfile.Close();
	return true;
}

// 初始化去雷鸣步骤
int GameStep::InitGoLeiMingSteps()
{
	// 尤兰达
	ParseStep("移动 66,60 62,65", m_GoLeiMingStep);
	//ParseStep("小号 出副本", m_GoLeiMingStep);
	//ParseStep("小号 进副本", m_GoLeiMingStep);
	ParseStep("NPC 罗德·兰", m_GoLeiMingStep);
	ParseStep("等待 2", m_GoLeiMingStep);
	ParseStep("按钮 0x9F1", m_GoLeiMingStep);
	ParseStep("等待 10", m_GoLeiMingStep);
	ParseStep("NPC 娜塔莉", m_GoLeiMingStep);
	ParseStep("选择 0x00", m_GoLeiMingStep);
	ParseStep("等待 10", m_GoLeiMingStep);
	printf("初始化从神殿传送到雷鸣大陆流程完成:%d\n", 8);
	return 8;
}

#if USE_MY_LINK
int GameStep::ParseStep(const char* data, Link<_step_*>& link)
#else
// 解析步骤
int GameStep::ParseStep(const char* data, vector<_step_*>& link)
#endif
{
	Explode explode(" ", data);
	if (explode.GetCount() < 2)
		return -1;
	if (*data == '=')
		return 0;

	int result = 1;
	char* cmd = explode.GetValue(0);
	if (*cmd == '-') {
		result = 2;
		cmd++;
	}

	InitStep(step);
	try {
		step.OpCode = TransFormOP(cmd);
		switch (step.OpCode)
		{
		case OP_MOVE:
		case OP_MOVEFAR:
			if (!TransFormPos(explode[1], step)) {
				printf("GameStep::InitSteps.TransFormPos失败\n");
				result = -1;
			}
			TransFormPos(explode[2], step.X2, step.Y2);
			break;
		case OP_NPC:
			strcpy(step.NPCName, explode[1]);
			break;
		case OP_SELECT:
			step.SelectNo = explode.GetValue2Int(1);
			step.OpCount = explode.GetValue2Int(2);
			if (step.OpCount == 0)
				step.OpCount = 1;
			break;
		case OP_MAGIC:
		case OP_MAGIC_PET:
			TransFormMagic(explode, step);
			break;
		case OP_CRAZY:
		case OP_CLEAR:
			strcpy(step.Magic, explode[1]);
			break;
		case OP_PICKUP:
		case OP_CHECKIN:
			break;
		case OP_USEITEM:
			strcpy(step.Name, explode[1]);     // 物品名称
			if (explode.GetCount() > 2) {
				if (strstr(explode[2], ",")) { // 是否验证传送坐标
					TransFormPos(explode[2], step);
				}
				if (explode.GetCount() > 3) {
					step.Extra[0] = explode.GetValue2Int(3);
				}
			}
			break;
		case OP_DROPITEM:
			strcpy(step.Name, explode[1]);     // 物品名称
			if (explode.GetCount() > 2) {
				step.Extra[0] = explode.GetValue2Int(2);
			}
			break;
		case OP_SELL:
			break;
		case OP_BUTTON:
			step.ButtonId = explode.GetValue2Int(1);
			break;
		case OP_WAIT:
			TransFormWait(explode, step);
			break;
		case OP_SMALL:
			TransFormSmall(explode, step);
			break;
		default:
			result = -1;
			break;
		}
	}
	catch (...) {
		result = -1;
		printf("读取流程错误:%s\n", data);
	}
	if (result) {
		_step_* pStep = new _step_;
		memcpy(pStep, &step, sizeof(_step_));
#if USE_MY_LINK
		link.Add(pStep);
#else
		link.push_back(pStep);
#endif
	}
	return result;
}

// 转成实际指令
STEP_CODE GameStep::TransFormOP(const char* data)
{
	if (data == nullptr)
		return OP_UNKNOW;
	// 以下为建筑类型
	if (strcmp(data, "移动") == 0)
		return OP_MOVE;
	if (strcmp(data, "传送") == 0)
		return OP_MOVEFAR;
	if (strcmp(data, "NPC") == 0)
		return OP_NPC;
	if (strcmp(data, "选择") == 0)
		return OP_SELECT;
	if (strcmp(data, "选项") == 0)
		return OP_SELECT;
	if (strcmp(data, "技能") == 0)
		return OP_MAGIC;
	if (strcmp(data, "宠技") == 0)
		return OP_MAGIC_PET;
	if (strcmp(data, "狂甩") == 0)
		return OP_CRAZY;
	if (strcmp(data, "清怪") == 0)
		return OP_CLEAR;
	if (strcmp(data, "捡物") == 0)
		return OP_PICKUP;
	if (strcmp(data, "存物") == 0)
		return OP_CHECKIN;
	if (strcmp(data, "使用") == 0)
		return OP_USEITEM;
	if (strcmp(data, "丢物") == 0)
		return OP_DROPITEM;
	if (strcmp(data, "售卖") == 0)
		return OP_SELL;
	if (strcmp(data, "出售") == 0)
		return OP_SELL;
	if (strcmp(data, "按钮") == 0)
		return OP_BUTTON;
	if (strcmp(data, "等待") == 0)
		return OP_WAIT;
	if (strcmp(data, "小号") == 0)
		return OP_SMALL;
}

// 转成实际坐标
bool GameStep::TransFormPos(const char* str, _step_& step)
{
	Explode explode(",", str);
	if (explode.GetCount() == 2) {
		step.X = explode.GetValue2Int(0);
		step.Y = explode.GetValue2Int(1);
	}

	return explode.GetCount() == 2;
}

// 转成实际坐标
bool GameStep::TransFormPos(const char * str, DWORD & x, DWORD & y)
{
	if (str == nullptr)
		return false;

	Explode explode(",", str);
	if (explode.GetCount() == 2) {
		x = explode.GetValue2Int(0);
		y = explode.GetValue2Int(1);
	}

	return explode.GetCount() == 2;
}

// 转成实际技能
bool GameStep::TransFormMagic(Explode& line, _step_ & step)
{
	//printf("TransFormMagic\n");
	strcpy(step.Magic, line[1]);
	if (line.GetCount() > 2) {
		int index = 2;
		if (strstr(line[2], ",")) { // 参数是坐标
			TransFormPos(line[2], step);
			index++;
		}
		step.WaitMs = line.GetValue2Int(index) * 1000;
	}
	//printf("TransFormMagic End\n");

	return true;
}

// 转成实际技能
MagicType GameStep::TransFormMagic(const char* str)
{
	if (str == nullptr)
		return 未知技能;
	if (strcmp(str, "星陨") == 0)
		return 星陨;
	if (strcmp(str, "影魂契约") == 0)
		return 影魂契约;
	if (strcmp(str, "诸神裁决") == 0)
		return 诸神裁决;
	if (strcmp(str, "虚无空间") == 0)
		return 虚无空间;
	if (strcmp(str, "凤珠") == 0)
		return 凤珠;
	if (strcmp(str, "电击术") == 0)
		return 电击术;
	if (strcmp(str, "最终审判") == 0)
		return 最终审判;

	return 未知技能;
}

// 转成Wait数据
bool GameStep::TransFormWait(Explode& line, _step_& step)
{
	int index = 1;
	int test = line.GetValue2Int(1);
	if (test == 0) { // 等待技能冷却
		strcpy(step.Magic, line[1]);
		index++;
	}
	int v = line.GetValue2Int(index);
	step.WaitMs = v < 150 ? v * 1000 : v;

	return 0;
}

// 小号
void GameStep::TransFormSmall(Explode & line, _step_ & step)
{
	int v = -1;
	if (line.GetCount() > 1) {
		if (strcmp(line[1], "出副本") == 0)
			v = 0;
		if (strcmp(line[1], "进副本") == 0)
			v = 1;
	}
	step.SmallV = v;
}

// 添加步骤
void GameStep::AddStep(_step_& step)
{
	//memset(&m_Steps[m_iStepIndex], 0, sizeof(_step_));
	memcpy(&m_Steps[m_iStepIndex], &step, sizeof(_step_));
	//INLOGVARN(32, "添加步骤:%d Exec:%d", step.OpCode, step);
	m_iStepCount = ++m_iStepIndex;
}

// 重置执行步骤索引
void GameStep::ResetStep(int index)
{
	if (index > 0) {
		m_iStepStartIndex = index;
	}
	m_iStepIndex = index;
#if USE_MY_LINK
	m_Step.Reset(m_iStepIndex);
	m_GoLeiMingStep.Reset(m_iStepIndex);
#endif
}