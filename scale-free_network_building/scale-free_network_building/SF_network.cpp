// Creation of a scale-free graph: a new node is attached to two old nodes, the probability of attachment 
// is proportional to the number of links of the old node.
// written by ���� March, 2011     

// standard include
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>   
using   namespace   std;

// define crucial parameters

#define NODES 200		// �ڵ�����number of nodes
#define LINK (4*NODES-6)		// �ܹ�������ߵĸ���number of directed links

// main variables

struct Agent
{
	int *neighbours; // contains players' neighbours
	int link; //the number of links of the player
	short strategies;///���ԣ�0 cooperator,1 defector
};
//count the people of cooperators and defectors
int Num[2];

Agent Player[NODES];
ofstream outfile;

#define MC_STEPS  20000 //run-time in MCS
#define Record_STEPS  2000 //��¼���100���Ľ��
#define TryTime 10   //ȡ10�εĽ��ƽ��

#define K  0.1  //noise effects
#define B_steps 0.001

double b;//?
//���������������ģ�飬���ù���,ֱ���þ��У�
//��randf()����ֱ�Ӳ���0-1������ȷֲ����������
///randi(x),����0---x-1���������

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


// creates the scale-free graph (called only once)
void Prodgraph(void)
{

	int start[LINK], end[LINK];
	// ÿ������ߵ������յ㣬it is easier to generate the graph by using directed links 
	//(just for generation!)
	// start[i] yields the node of the 'i'-th directed link  
	// end[i] shows the end of the start[i] link
	int f = 0, n, i, r;
	double p;
	int sz;///���е�����ߵ�����
	// reset nodes and links
	for (i = 0; i<LINK; i++)
	{
		start[i] = -1;	// resets the start (starting node) of the i-th directed link
		end[i] = -1;		// resets the end (ending node) of the i-th directed link
	}
	///�Ͽ����������


	for (i = 0; i<NODES; i++) Player[i].link = 0;
	// initially all nodes have no link 
	//(nobody is connected to nobody)

	// the first two nodes are connected: both have 1 link, means we have two directed links so far
	Player[0].link = 1;//�ڵ�0��һ����
	Player[1].link = 1;

	start[0] = 0; //��1���ߣ�ʼ��0����1
	end[0] = 1;

	start[1] = 1;
	end[1] = 0;

	// remember number of directed links (so far)
	sz = 2; //����ߵ�����

	for (i = 2; i<NODES; i++)
	{
		f = 0;  // number of new links
		r = -1; // serial number of node whose attachment is already done 

		while (f<2) // meaning that a new node (i) is attached to two(f<2) old nodes
		{

			n = (int)randi(i); // an old node is choosen [e.g. randi(2) is either 0 or 1]; 
			//note that rand(i) always selects a node whose attachment
			// is already done ate least once
			///ѡȡ�ɽڵ㣬i�ĳ�ʼֵΪ2��Ҳ����˵������ѡȡ��ǰ���εĽڵ㣬
			///��Щ�ڵ��Ѿ���ʼ����:�нڵ���������������һ���бߵĽڵ�


			///nӦ���Ǹ�������if�����岻��
			///ע�⣬randi����������[0,i)������ҿ��������֣����ᵽ��i
			///���Ľ��ͣ���ֹ�������ӵ�һ���ڵ���
			if (n != r)

			{
				p = (double)Player[n].link / sz;
				// this is the probability the old node 'n' gets a link; 
				//depending on how many links it already has
				// and what is the total number of links 
				//(we consider only directed links for the generation of the scale-free network)
				///�½ڵ�i��ѡ��ľɽڵ�n���ӵĿ����������ھɽڵ����ͨ��
				///�Լ���ǰͼ�С��ߵ�������
				if (randf()<p)				// if a new link is added to node n, then n(old node) and i(new node) are connected
					///��������Գ���������Ĳ���
					///��ؽڵ��������++
					///�߾����Ӧ��ʼ����ֹ�ڵ㸳ֵ
					///��ͼ���С������������+2
					///r=n��¼�ϴ����ӵľɽڵ㣬��ֹ������ӵ�һ���ڵ���
					///Ϊʲô���нڵ�����ʱ��ֻ���������ߣ��鿴�ޱ�����綨��
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

	//�����ɺõľ���ת���ڽӾ���
	int LinkCount[NODES];
	for (i = 0; i<NODES; i++)
	{
		LinkCount[i] = 0;	// ��¼�Ѿ����ӵ�ÿ�������ھ������еĸ�����
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
		///�ӽڵ��ȥ�ıߵ��յ��ǽڵ���ھ�
		LinkCount[start[i]]++;
	}

}

void Initial(void)//��ʼ���������γ�ÿ�����ӵĳ�ʼ״̬
{

	//��ʼ��������ھ�
	Prodgraph();// creates the scale-free graph

}
void InitialStrategy(void)
{
	Num[0] = 0; Num[1] = 0;
	for (int i = 0; i<NODES; i++)
	{
		Player[i].strategies = (short)randi(2);//0 cooperator,1 defector
		Num[Player[i].strategies]++;
	}
}
///PDG����������
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

void changeStrategy(int playerX, double payoffX, double payoffY)
{
	double probability = 1 / (1 + exp((payoffX - payoffY) / Player[playerX].link));
	if (randf()<probability)
	{
		Num[Player[playerX].strategies]--;
		Num[1 - Player[playerX].strategies]++;
		Player[playerX].strategies = 1 - Player[playerX].strategies;
	}
}

void Destroy(void)
{
	for (int i = 0; i<NODES; i++)
	{
		delete[] Player[i].neighbours;
		Player[i].neighbours = NULL;
	}
}

void playGameMainProcess()
{
	int steps, i;
	int playerX, playerY;
	int strategyX, strategyY;
	double payoffX, payoffY;
	double P_c, AverP_c;
	outfile.open("data.txt");

	Initial();
	/**********************************/
	for (b = 0; b < 2 + B_steps; b += B_steps)//����bΪ��������B_steps = 0.001 ,Ҳ���ǽ���2000������
		///������������ٶ�Ӧ�ò���������������ʱ����
	{
		/* code */
	}

}
void PDG()
{
	sgenrand((unsigned)time(NULL));// initialize RNG

	Initial();// inital strategy distribution



	Destroy();//�������ʹ�ù��Ķ�̬����
}



void main()
{
	PDG();
} // main 