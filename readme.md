# 香菇滑稽 Version3 Update3

> 修订：Du\wxk14<br>
> 规则：Du <br>
> 鸣谢：Pierre ;Starlight 

## Getting Started——三分钟教你学会香菇滑稽！

TODO

谁来背一下这个锅

## 选手接口（或曰definition.h简介）

每回合开始的时候，选手的ai会收到一个信息结构info。其定义如下：

```C++
//definition.h
struct Info
{
	TId id;//选手的编号
	TId playerSize;//总的选手数目（包括死亡的数目）
	TMap rows;//地图的行数（高度）
	TMap cols;//地图的列数（宽度）
	TRound round;//当前的轮数（从第一轮开始。第0轮是选择出生点的虚拟轮次）
    
    //地图。注意地图传的是一个指针。请使用->符来调用地图。
    //示例：vector<vector<TMoney> > mapRes = map->getMapRes()；
	BaseMap* map;

    //以下使用了C++ STL库Vector.大家当一维/二维数组使用它就可以了，可以把它当成一个动态数组来用
    //示例：MapPointInfo mpi = mapPointInfo[x][y]；表示第(x,y)点的MapPointInfo
	vector<PlayerInfo> playerInfo;//玩家信息列表
	vector<vector<MapPointInfo> > mapPointInfo;//地图信息列表
  
    //这三个是你填充在info里面，回传给程序的部分
	vector<TMilitaryCommand> MilitaryCommandList;//军事指令列表
	vector<TDiplomaticCommand> DiplomaticCommandList;//外交态度指令列表
	TPosition newCapital;//下回合的新首都
};
```

在上面的信息中使用了两个结构PlayerInfo和MapPointInfo，以及一个类BaseMap，其分别定义如下：

```C++
//definition.h
struct PlayerInfo
{    
	TDiplomaticStatus dipStatus; //你和该AI的外交状态
	bool isVisible; // this player is visible to you ; you two share basic info.
	bool isUnion;   // this player has reached an alliance with you ; you two share all info.
	//以下四个是你至少isVisible才能知道的信息
	int mapArea; // area size of this player's land
	vector<TId> warList;//player at war
	//以下两个是你至少isUnion才能知道的信息
	TMoney saving; // resource of this player
	TPosition capital;//capital
};

struct MapPointInfo
{
	TMask isVisible; //这个点你看不看得见
    //如果你看得见的话，以下两个信息是确切的；不然以下两个信息没有意义
	TId owner;//这个点的归属
	TMask isSieged;//这个点是否已经处于被包围(isSieged)状态
};

//这是一个C++中的类。类其实是结构（Struct）中比较高级的一种（你就姑且这么认为吧），完全不用在意区别。没什么区别的
//类可以有所谓“方法”.事实上你可以把“方法”全部理解成结构中的函数指针。调用起来和调用结构中的函数指针一模一样。
//调用示例：vector<vector<TMoney> > mapRes = map->getMapRes()；
class BaseMap
{
public:
	BaseMap(){;}
	inline TMap getRows() const {return rows;}
	inline TMap getCols() const {return cols;}
	inline vector<vector<TMoney> > getMapRes() const {return MapResource_;}//获得地图各点资源的二维数组表
	inline vector<vector<TMilitary> > getMapAtk() const {return MapAttackRatio_;}//获得地图各点攻击的二维数组表
	inline vector<vector<TMilitary> > getMapDef() const {return MapDefenseRatio_;}//获得地图各点防御的二维数组表
protected:
     //这些元素你也访问不了 就不用关心了。
     //所以说类比较高级嘛
	TMap	rows, cols;
	vector<vector<TMoney> >  MapResource_;
	vector<vector<TMilitary> > MapDefenseRatio_, MapAttackRatio_;
};
```

另外，游戏中使用一些自解释的枚举和小结构，我这里就不用多解释了：

```C
enum TDiplomaticStatus
{
    Undiscovered,   // a country that has never appeared in your visible area
    Neutral,        // default status of a newly discovered country
    Allied,          
    AtWar
};
enum TDiplomaticCommand
{
    KeepNeutral, 
    FormAlliance, 
    JustifyWar,
	Backstab
};

struct TMilitaryCommand
{
	TPosition place; //your goal place
	TMilitary bomb_size;
};

struct TPosition
{
	TMap x;
	TMap y;
};
```

最后是一大波常数：由于常数有许多，这里就不一一注释了，参见definition.h。

## 剧本简介

参见https://github.com/hzsydy/XiangGuHuaJi/blob/dev-3.0/Scenarios/

## 工程介绍

参见同目录下project.md

## （按照时序的）游戏文档（供写代码的时候查阅）

### 概述

领土扩张~~核弹战争~~游戏

- 外交：通过外交上的合纵连横保存实力
- 军事：消耗资源投掷炸弹，以夺得/保卫领土主权
- 经济：领土为国家提供资源

以下给出一些定义：

- 敌对国：外交关系为战争的国家。只有战争中的国家才能相互侵占对方领土。
- 同盟国：外交关系为同盟的国家。
- 中立国：外交关系为中立/停战的国家。
- 军通区：所有你可以放置炸弹的区域。也就是你的领土+同盟国领土。注意，你的军通区可能不是连通的。
- 首都：首都用于组织己方和同盟国领土的防御。首都可以建立在军通区的任意位置，即可以放在同盟国境内。与首都**连通**的军通区有防御力加成，详见**军事**。

### 回合时序

第0回合

- 选择出生点（调用玩家AI）

从第1回合开始

- 调用玩家AI，获取外交指令，军事指令
- 外交
  - 处理外交指令，更新国际关系。
  - 如果你的首都在同盟国境内，而此回合同盟被解除了，你在此回合将被迫失去首都。
- 军事
  - 每个国家只能在军通区放置有限个炸弹。此时不合法的指令将被自动忽略。注意，在同盟解除时军通区会瞬间发生变化，因此你的部分指令可能被迫失效。
  - 每个炸弹将在周围有限范围内产生距离递减的震慑力。
    - 若你的首都存在，在与首都**连通**的军通区上，你的震慑力记为额外防御力。注意，此时你可以为你的同盟国提供防御力。这些国家的防御力直接累加。
    - 在中立或敌对国领土上，你的震慑力之和为攻击力。不同国家的攻击力数值独立计算。
  - 在一块土地上，你的攻击力最高且高于领土总防御力一定数值时，你获得此领土。
  - 更新你的首都位置。如果你此回合指定的首都位置不合法（不在你的领土+同盟国领土内），你失去首都。
- 经济
  - 收取此时本国领土上的资源。
  - 如果此时你的首都存在，你获得一份额外固定收益。 ~~也就是低保~~ 
- 检查胜负
  - 一个没有首都且没有领土的玩家立即出局。只剩下一名玩家时游戏结束。
    - 没有领土，但在同盟国上建有首都的玩家（~~流亡政府~~）有低保收入，将继续游戏，参见 **资源**。
  - （乱斗规则）任何一个尚未出局的玩家完成了剧本要求，游戏结束。
  - 回合数达到上限时游戏结束
- 游戏综合排名，从最重要到次重要
  - （乱斗）完成剧本要求
  - 坚持的回合数目。存活时间越久，排名越高 ~~不论你曾如何卧薪尝胆/苟且偷生~~
  - 游戏结束时的领土面积。
  - 玩家离开游戏时的资源数量

### 出生点的选择

游戏开始前，每一位玩家都有固定量的初始资源。
规则将先读取各位玩家为了优先选取出生点而愿意拿出的初始资源，然后决定出生点选取顺序，流程如下：

- 出价高的先选点。
- 出价相同，则对战积分低的先选。
- 否则随机排列。

选点时，规则将给出地图详细信息，和当前已经被选为出生点的位置。

### 地图

- 二维四边形网格。地图尺寸是给定的，预计最大50*50。
- 主权：游戏开始时，除了出生点，所有土地都是中立的。
- 地形和资源
    - 地形包括攻击系数和防御系数两部分，影响军事阶段。
    - 资源的富裕程度一般和地形是一一对应的（ ~~因为我们没空给每个点都自设资源~~），剧本指明的特殊点除外。
- 土地的特殊效果：乱斗规则特有，由剧本指明

### 外交阶段

外交阶段改变国际关系。作为回合时序的第一步，外交战术不可忽视。

- 视野
  - 外交关系决定了你的**地图视野**和**国家内政视野**。
  - 地图视野
    - 你的视野为：对你联盟的国家领土加相邻的2格范围 + 对你中立的国家领土。你能够得知这些土地的主权。
    - 对于你没有视野的土地，你只能获得土地的攻防参数和资源数。
      - 事实上，土地的攻防参数和资源数是你所早就知道的：因为我们的地图是恒定不变且在赛前公开的。~~你可以先验的用if-else来规定你AI的策略，比如我们都知道六教比较肥（误）~~
  - 国家内政视野
    - 战争/未发现：你无法获得此国家的任何内政信息
    - 中立：你可以获得此国家的
      - 领土总面积；与该国家处于战争和停战状态的国家列表。
    - 同盟：你可以获得此国家的
      - 领土总面积；首都位置；当前库存资源总量；该国家与所有国家的外交关系列表。（也就是所有信息。）
- 侵犯  
  - 只有处于 **战争** 状态的国家之间才会发生领土变更。
  - 中立领土不属于任何一个国家（或属于一个总是和所有玩家宣战但是从不出兵的国家）
- 军通和联防
  - 同盟国可以在对方的领土内放置兵力，视作有**军通**。
  - 同盟国之间的额外防御力是叠加计算的，称为**联防** 。

外交态度指令：

- 包括3种常规指令
  - 保持中立KeepNeutral
  - 默许同盟FormAlliance；双方同时进行此操作才会同盟。
  - 战争合理化JustifyWar；
    - 在双方并未战争的情况下
      - 如果有一方保持JustifyWar3回合，那么双方进入战争状态。~~战争合理化和战前动员过程~~
      - 必须是连续的3回合。例如，如果2回合之后撤除JustifyWar指令，那么JustifyWar进程清零，下次又要保持三回合才能进入战争。
      - 然而背刺是立刻效果。~~邻居快亡国了想捡漏？背刺呀~~
      - 由于外交态度指令并不能被其他AI知晓，所以你永远不知道一个挂着中立对你的AI是真心想中立呢，还是已经造了两回合战争借口了呢 滑稽
    - 在双方已经战争的情况下
      - 任何一方发生此指令，战争将维持。
    - 无论什么情况下，主动发这个指令的人每发一次这个指令就要扣钱（军费1元）。嗯打仗是很烧钱的所以请速战速决。
- 1种特殊指令
  - 背刺Backstab；一场游戏只能使用一次，不论如何(同盟/停战)立即变成战争状态。第二次使用当作KeepNeutral指令。
- 国际关系和外交态度的关系总结表（常规指令）


| 代码中名称 | Undiscovered |                 Neutral                  |     Allied      | AtWar |
| :---: | :----------: | :--------------------------------------: | :-------------: | :---: |
|       |     未发现      |                    中立                    |       同盟        |  战争   |
|  注释   |  从未在地图视野中出现  |                 见面时的默认状态                 | 等于中立，但x回合内不能宣战； |       |
| 可能来自  |      -       | 战争（当双方态度均不为“战争”）；未发现（自动）；同盟（当至少一方不为“默许同盟”） |       中立        | 中立；背刺 |
| 可能变为  |      中立      | 同盟（当双方态度均为“默许同盟”）；战争（当至少一方态度为“战争”，且持续3回合以上） |       中立        |  中立   |







### 军事阶段

军事阶段是改变地图领土归属的阶段。这一阶段通过消耗资源投放炸弹来实现。

炸弹的震慑力只保留一回合。只有领土的变更继承到下一回合。

- 炸弹的投放
  - 炸弹只能投放在军通区上。炸弹的价格是线性离散的。
- 领土变更的计算
  - 威慑力：炸弹能量\*一个随距离衰减的系数。即卷积一个尺寸有限的高斯核。
    - 对中立和敌对领土，威慑力累积为攻击力。攻击力每个国家单独计算。
    - 对和己方首都连通的军通区，威慑力累积为防御力。防御力由所有联盟国数值累加。中立土地没有防御力。
  - 在一块土地上，攻击力\*攻击系数-防御力\*防御系数 > 阈值X 时，该格领土被攻击最高的一方**击破**此领土。（注意，还要经过连通性判定才可以夺取领土）
    - 如果有两方相同且高于阈值，那么这块地被打成中立。
  - 所有玩家检查自己击破的领土与己方(军事结算之前的)军通区的连通性，若连通则获得自己击破的领土。否则被击破的领土恢复原有主权。
    - 你领土的攻占总是连续的。例如，隔河击穿了沿线的所有地方，然而却没有击穿桥头堡，那么隔河的部分因为不连通所以无法攻下。
    - 在易攻难守的地形上，打的太猛可能会出现换家的现象。
- 首都是玩家指定的一个地点。如果玩家不指定，默认为上一回合的值。初始值被置为出生点。
  - 首都可以不设在自己领地而是设在**军通区**的同盟领地上。
  - 如果首都被设置在了不合法的地点（一般是由于时序问题），你将暂时（本回合）失去首都。
    - 设在同盟军通领土上，但本回合同盟被撤除会导致这一现象。
    - 如果你指定的首都位置在本回合的军事流程时被攻陷，也会导致这一现象。
  - 首都除了有组织防御的功能，首都还有一定的攻击力 ~~参考明代和清代将首都设在北京~~ 。只要你的首都本回合存在，就相当于在首都位置丢下等效于x的炸弹~~首都自带威慑力防御~~，x为你每回合收入的30%等效的炸弹规模，向上取整。
    - tips:在初期，利用频繁改变首都快速扩张领地是非常重要的~~红警里面拖着基地车到处跑~~
  - 如果你的某个军通区域和首都不连通，则你无法在这些区域产生防御力。此时如果没有远处的同盟国帮你，这些区域的防御能力和中立地一样。
    - 但是你依然可以投放兵力进一步组织进攻。~~强行连回来或者强行拆对面首都 胆子大你就直接把首都搬过去~~ 
    - 但是远处盟友的有效兵力依然可以产生有效防御。~~是啊我们就是鼓励支援盟友 鼓励远远的交朋友 如果你不救盟友 下回合你盟友就全被敌人吞掉了哟 ~~

### 经济阶段

经济阶段是收取资源的阶段。

- 计算公式：下一回合的资源 = 上一回合的资源 * （1 - 腐败系数*土地面积） + （土地收入 + 城市收入）
    - 土地收入：你领土上所有资源之和
    - 首都收入：只要首都存在，就可以得到来自你首都的工资~~低保~~。
      - 这意味着流亡政府（没有自己领土，只是靠同盟领地中的首都苟活）也能领到来自首都的工资~~低保~~。
    - 腐败系数：每单位土地让你造成的资源收获损耗。 ~~领土越大，中央集权越弱，腐败越严重~~ 
- tips
    - ~~计算你每回合产生的资源是一件吃时间的事情，如果没有时间计算的话，不妨试试直接存储上回合你的资源量，相减就知道你上回合产了多少，这回合也差不多~~

### TIPS

- 利用回合的时序的战术动作
  - 攻击一个中立国家：主动宣战，并在宣战的回合部署重兵，几乎可以保证打出先手速攻。
    - ~~友情提示：如果不能一回合达到战术目的，请考虑准备预备队。~~
    - 如果一回合打崩对面首都当然更好，下回合直接强势收割；背刺当然效果拔群；所以后期请务必注意首都的站位。
  - 利用同盟军通瞬移：主动请求同盟，同时尝试在对方领土的边缘部署兵力。如果对方不同意同盟，你在对方边境的部署命令将被忽略；而如果对方同意了同盟，你将能够立即在很远的地方扩展出自己的疆土。
    - ~~友情提示：下回合被卖了就傻逼了~~
  - 流亡政府：在本国领土即将被攻占，感觉无力回天时，立即将首都部署到自己最强的一个同盟国领土中。原则上说，你的命运从此就掌握在大哥手中了（大哥一旦取消同盟，回合结束的时候你可能就没有首都了）；不过，因为你对大哥只有益处（联防）没有害处，大哥可能不会劝退你。
    - ~~友情提示：这样虽然可以苟活很久，但是如果你到最后一回合还没有获得自己的领土的话，大哥是很有动机在最后一回合劝退你的~~


## 比赛形式

【赛程慢慢来嘛】

- 积分赛~~天梯模式~~
  - 根据标准规则和标准地图进行 *一局比赛* 。标准地图是公开的。
  - 关于 *一局比赛* 的定义，参见本段最后。
- 初赛
  - 初赛的时候将根据标准规则和初赛特制的地图进行 *一轮比赛* ，前7名进入决赛（刨掉乱斗模式的胜利者之外的）
  - 关于 *一轮比赛* 的定义，参见本段最后。
- 乱斗模式
  - 每两周一次，采用特别制定的地图和特别制定的胜利条件和特殊规则，决出胜负。每个乱斗周期结束的时候，进行 *一轮比赛* ，前三名作为本届乱斗的优胜者保送进入决赛。【并且应该还有其他优惠】
  - 特制地图将在该轮次乱斗的前一周公布。
- 决赛
  决赛将采用标准规则和决赛特制的地图进行 *一轮比赛*。
- 一局比赛的定义
  - 一局比赛中，将由赛程规定数目的玩家参与比赛，获得初始资源，并且决出优胜者（或相对优胜者）。关于如何决出优胜者，参见 **游戏规则-胜利条件** 。
- 一轮比赛的定义
  - 一轮比赛将由若干局比赛组成。在第一局比赛中，所有玩家的出生点将获得等量的资源。从第二局比赛开始，上一局中的优胜者（或相对优胜者）获得上一局初始资源的1.5倍。持续此过程，直到有玩家的初始资源为原来资源的1.5^5倍，则此玩家获得优胜并从下一局游戏开始被剔除。持续此过程直到足够多的玩家（赛程规定数目的玩家）获得优胜。
