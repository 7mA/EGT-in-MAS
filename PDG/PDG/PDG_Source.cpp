//98PRE
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <sstream>
using namespace std;

#define MC_STEPS  20000 //run-time in MCS
#define Record_STEPS  2000 //记录最后100步的结果
#define TryTime 10   //取10次的结果平均
#define NODES 200		// 节点总数number of nodes
#define LINK (4*NODES-6)		// 总共的有向边的个数number of directed links

#define K  0.1  //noise effects
#define B_steps 0.001
double b;

struct Agent
{
	int *neighbours; // contains players' neighbours
	int link; //the number of links of the player
	short strategies;///策略；0 cooperator,1 defector	
};
struct Agent Player[NODES];
ofstream outfile;
int Num[2];//count the people of cooperators and defectors
double Result[TryTime];
//randf() generate 0-1,randi(x)generate 0-(x-1)
/*************************** RNG procedures ****************************************/
#define NN 624
#define MM 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[NN]; /* the array for the state vector  */
static int mti = NN + 1; /* mti==NN+1 means mt[NN] is not initialized */
void sgenrand(unsigned long seed)
{
	int i;
	for (i = 0; i<NN; i++) {
		mt[i] = seed & 0xffff0000; seed = 69069 * seed + 1;
		mt[i] |= (seed & 0xffff0000) >> 16; seed = 69069 * seed + 1;
	}
	mti = NN;
}
void lsgenrand(unsigned long seed_array[])
{
	int i; for (i = 0; i<NN; i++) mt[i] = seed_array[i]; mti = NN;
}
double genrand()
{
	unsigned long y;
	static unsigned long mag01[2] = { 0x0, MATRIX_A };
	if (mti >= NN)
	{
		int kk;
		if (mti == NN + 1) sgenrand(4357);
		for (kk = 0; kk<NN - MM; kk++) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + MM] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (; kk<NN - 1; kk++) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (MM - NN)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[NN - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[NN - 1] = mt[MM - 1] ^ (y >> 1) ^ mag01[y & 0x1];
		mti = 0;
	}
	y = mt[mti++]; y ^= TEMPERING_SHIFT_U(y); y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C; y ^= TEMPERING_SHIFT_L(y);
	return y;
}

double randf(){ return ((double)genrand() * 2.3283064370807974e-10); }
long randi(unsigned long LIM){ return((unsigned long)genrand() % LIM); }

/********************** END of RNG ************************************/
void Prodgraph(void)//defines nerghbors on a square lattice
{
	int start[LINK], end[LINK];
	// 每条定向边的起点和终点，it is easier to generate the graph by using directed links 
	//(just for generation!)
	// start[i] yields the node of the 'i'-th directed link  
	// end[i] shows the end of the start[i] link
	int f = 0, n, i, r;
	double p;
	int sz;///所有的有向边的条数
	// reset nodes and links
	for (i = 0; i<LINK; i++)
	{
		start[i] = -1;	// resets the start (starting node) of the i-th directed link
		end[i] = -1;		// resets the end (ending node) of the i-th directed link
	}
	///断开所有有向边


	for (i = 0; i<NODES; i++) Player[i].link = 0;
	// initially all nodes have no link 
	//(nobody is connected to nobody)

	// the first two nodes are connected: both have 1 link, means we have two directed links so far
	Player[0].link = 1;//节点0有一条边
	Player[1].link = 1;

	start[0] = 0; //第1条边：始于0终于1
	end[0] = 1;

	start[1] = 1;
	end[1] = 0;

	// remember number of directed links (so far)
	sz = 2; //有向边的条数

	for (i = 2; i<NODES; i++)
	{
		f = 0;  // number of new links
		r = -1; // serial number of node whose attachment is already done 

		while (f<2) // meaning that a new node (i) is attached to two(f<2) old nodes
		{

			n = (int)randi(i); // an old node is choosen [e.g. randi(2) is either 0 or 1]; 
			//note that rand(i) always selects a node whose attachment
			// is already done ate least once
			///选取旧节点，i的初始值为2，也就是说，至少选取的前两次的节点，
			///这些节点已经初始化过:有节点与其相连，这是一个有边的节点


			///n应该是个正数？if的意义不明
			///注意，randi函数产生从[0,i)（左闭右开）的数字，不会到达i
			///下文解释，防止两次连接到一个节点上
			if (n != r)

			{
				p = (double)Player[n].link / sz;
				// this is the probability the old node 'n' gets a link; 
				//depending on how many links it already has
				// and what is the total number of links 
				//(we consider only directed links for the generation of the scale-free network)
				///新节点i与选择的旧节点n连接的可能性依赖于旧节点的连通度
				///以及当前图中“边的条数”
				if (randf()<p)				// if a new link is added to node n, then n(old node) and i(new node) are connected
					///如果可能性成立，处理的部分
					///相关节点的连边数++
					///边矩阵对应初始，终止节点赋值
					///“图”中“总有向边数”+2
					///r=n记录上次连接的旧节点，防止多次连接到一个节点上
					///为什么所有节点加入的时候只连接两条边？查看无标度网络定义
				{
					Player[n].link++;		// the connectivity of link 'n' increases by 1
					Player[i].link++;		// the connectivity of link 'i' increases by 1
					start[sz] = i;			// beginning of the link pointing out of 'i'
					end[sz] = n;			// ending of the link pointing out of 'i' (points to 'n')
					start[sz + 1] = n;		// beginning of the link pointing out of 'n'
					end[sz + 1] = i;			// ending of the link pointing out of 'n' (points to 'i')
					sz += 2;					// number of directed links increases by 2 (from 'i' to 'n' and from 'n' to 'i' - we have two directed links)
					f++;					// counts the links of node 'i' (a new i is chosen once two links have been established)
					r = n;					// prevents to connect twice to the same node
				}
			}
		}
	}

	// exploration of the generated graph: consideration the number of links of a node 
	// now store the generated graph in the p_nb[i][k] array; 
	///node 'i' has 'k' neighbours (connectivity 'k')

	//把生成好的矩阵转成邻接矩阵
	int LinkCount[NODES];
	for (i = 0; i<NODES; i++)
	{
		LinkCount[i] = 0;	// 记录已经增加到每个个体邻居向量中的个体数
	}

	for (i = 0; i<NODES; i++)
	{
		Player[i].neighbours = new int[Player[i].link];
		// (int *) calloc(Player[i].link, sizeof(int));	
		// p_nb[][] actually allocates for every node 'i' 
		///as much memory as node 'i' has neighbours (node[i])
	}

	for (i = 0; i<LINK; i++)
	{
		Player[start[i]].neighbours[LinkCount[start[i]]] = end[i];
		//p_nb[start[i]][p_nb2[0][start[i]]] = end[i];
		///从节点出去的边的终点是节点的邻居
		LinkCount[start[i]]++;
	}

}//prodgraph

void Initial(void)//初始化函数，形成每个格子的初始状态
{
	Num[0] = 0; Num[1] = 0;
	for (int i = 0; i<NODES; i++)
	{
		Player[i].strategies = (short)randi(2);//0 cooperator,1 defector
		Num[Player[i].strategies]++;
	}
	//Prodgraph();// creates the scale-free graph
}

double Game(int x)
{
	int y;
	double payoff = 0;
	int strategy = Player[x].strategies;
	for (int i = 0; i < Player[x].link; ++i)
	{
		/* code */
		y = Player[x].neighbours[i];
		if ((strategy == 0) && (Player[y].strategies == 0))
			payoff += 1;
		else if ((strategy == 1 && Player[y].strategies == 0))
			payoff += b;
		else
			payoff += 0;
	}
	return payoff;
}
void ChangeStrat(int playerX, double payoffX, double payoffY)
{
	double probability = 1 / (1 + exp((payoffX - payoffY) / K));
	if (randf()<probability)
	{
		Num[Player[playerX].strategies]--;
		Num[1 - Player[playerX].strategies]++;
		Player[playerX].strategies = 1 - Player[playerX].strategies;
	}
}

double Average()
{
	double x = 0.0;
	for (int i = 0; i<TryTime; i++)
	{
		x += Result[i];
	}
	x = x / TryTime;
	return x;

}
void ResultOut(double pc)
{
	outfile << b << "," << pc << endl;
	cout << b	 << "\t" << pc << endl;
}
void main()
{
	sgenrand((unsigned)time(NULL));//initialize RNG
	int steps, i, playerX, playerY, stratX, stratY;
	double payoffX, payoffY, P_c, AverP_c;
	outfile.open("data.csv");
	//初始化个体的邻居
	Prodgraph();
	for (b = 1; b<2 + B_steps; b += B_steps)
	{
		for (int test = 0; test<TryTime; test++)
		{
			P_c = 0;
			Initial();//inital strategy distribution
			for (steps = 0; steps<MC_STEPS; steps++)
			{
				for (i = 0; i<NODES; i++)
				{
					playerX = (int)randi(NODES);//choose a source site
					playerY = Player[playerX].neighbours[(int)randi(Player[playerX].link)];//by pow(age,alpha)/sumAge
					stratX = Player[playerX].strategies;
					stratY = Player[playerY].strategies;
					if ((stratX + stratY) == 1)
					{
						payoffX = Game(playerX);
						payoffY = Game(playerY);
						ChangeStrat(playerX, payoffX, payoffY);
					}//end of a source selection

				}//end of elementary MC step
				if (steps >= MC_STEPS - Record_STEPS)
				{
					P_c += (double)Num[0] / NODES;
				}
				if (steps == MC_STEPS - 1)
				{
					P_c = P_c / Record_STEPS;
				}
				if ((double)Num[0] / NODES >= 0.9995)//只剩下4个背叛者了
				{
					P_c = 1;
					break;
				}
				if ((double)Num[0] / NODES <= 0.0005)//只剩下4个合作者了
				{
					P_c = 0;
					break;
				}
			}//end of MC
			Result[test] = P_c;//将每次试验的结果保存
		}//end of test
		AverP_c = Average();
		ResultOut(AverP_c);
		if (AverP_c == 0)
		{
			break;
		}
	}//end of b
	outfile.close();
}