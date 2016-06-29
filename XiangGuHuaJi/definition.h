/* XiangGuHuaJi 2016, definition.h
 * 
 */

#ifndef _XIANGGUHUAJI_DEFINITION_H__
#define _XIANGGUHUAJI_DEFINITION_H__

typedef unsigned char TId;       //���Id
#define TMatId CV_8UC1
typedef unsigned char TMapSize;  //��ͼ�ߴ�(x,y)
typedef unsigned int  TMapArea;  //��ͼ���
typedef unsigned int  TRound;    //�غ���

typedef unsigned char TMapPara;  //��ͼ����(MapResource, MapDefenseRatio, MapAttackRatio)
#define TMatMapPara CV_8UC1
typedef unsigned int  TSaving;   //�û������

typedef unsigned char TDefense;  //���ص���(DefensePoints)
#define TMatDefense CV_8UC1
typedef unsigned char TAttack;   //��������(AttackPoints)
#define TMatAttack CV_8UC1

typedef unsigned char TMilitary; //һ������õı���
#define TMatMilitary CV_8UC1
typedef unsigned int  TMilitarySummary; //��ҵ��ܱ���

//�⽻��ϵ
enum TDiplomaticStatus
{
    Undiscovered, //δ֪����
    Neutral, //����
    Union,   //ͬ��
    War,     //ս��
    StopWar  //ͣս
};
//�⽻ָ��
enum TDiplomaticCommand
{
    KeepNeutral, //��������
    AskForUnion, //����ͬ��
    ClaimWar     //��ս
};

//�����Ϣ
struct TPlayerInfo
{    
    TId id; //���ID
    bool Visible; //�����ο���Ϣ: ����һ�����Ϣ�Ƿ����ɼ�
    bool Union;   //�����ο���Ϣ: ����ҵ�������Ϣ�Ƿ����ɼ����Ƿ���ͬ�ˣ�
//������Ϣ
    TMapArea MapArea; //���������
    TMilitarySummary MilitarySummary; //�ѱ����õ��ܱ���
//������Ϣ
    TSaving Saving; //�����
};


#endif