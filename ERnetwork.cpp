// Creation of a scale-free graph: a new node is attached to two old nodes, the probability of attachment 
// is proportional to the number of links of the old node.
// written by 王震 March, 2011     

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

#define NODES       10000		// 节点总数number of nodes
#define LINK	    (4*NODES-6)		// 总共的有向边的个数number of directed links

// main variables
struct Agent
{
	int *neighbours; // contains players' neighbours
	int link; //the number of links of the player
};


Agent Player[NODES];



//以下是随机数产生模块，不用管它,直接用就行，用randf()可以直接产生0-1满足均匀分布的随机数，randi(x),产生0---x-1的随机整数

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
static int mti=NN+1; /* mti==NN+1 means mt[NN] is not initialized */
void sgenrand(unsigned long seed)
{int i;
 for (i=0;i<NN;i++) {mt[i] = seed & 0xffff0000; seed = 69069 * seed + 1;
                     mt[i] |= (seed & 0xffff0000) >> 16; seed = 69069 * seed + 1;
  }
  mti = NN;
}
void lsgenrand(unsigned long seed_array[])
{ int i; for (i=0;i<NN;i++) mt[i] = seed_array[i]; mti=NN; }
double genrand() 
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    if (mti >= NN) 
    {
        int kk;
        if (mti == NN+1) sgenrand(4357); 
        for (kk=0;kk<NN-MM;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+MM] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<NN-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(MM-NN)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[NN-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[NN-1] = mt[MM-1] ^ (y >> 1) ^ mag01[y & 0x1];
        mti = 0;
    }  
    y = mt[mti++]; y ^= TEMPERING_SHIFT_U(y); y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C; y ^= TEMPERING_SHIFT_L(y);
    return y;  
}

double randf(){ return ( (double)genrand() * 2.3283064370807974e-10 ); }
long randi(unsigned long LIM){ return((unsigned long)genrand() % LIM); }

/********************** END of RNG ************************************/


// creates the scale-free graph (called only once)
void Prodgraph(void) 
{
	
	int start[LINK], end[LINK];		// 每条定向边的起点和终点，it is easier to generate the graph by using directed links (just for generation!)
								// start[i] yields the node of the 'i'-th directed link  
								// end[i] shows the end of the start[i] link
	int f=0,n,i,r;
	double p;
	int sz;
	// reset nodes and links
	for (i=0; i<LINK; i++) 
	{ 
		start[i]=-1;						// resets the start (starting node) of the i-th directed link
		end[i]=-1;							// resets the end (ending node) of the i-th directed link
	}


	for (i=0; i<NODES; i++) Player[i].link=0;	// initially all nodes have no link (nobody is connected to nobody)

	// the first two nodes are connected: both have 1 link, means we have two directed links so far
	Player[0].link=1;
	Player[1].link=1;
	
	start[0]=0; 
	end[0]=1;

	start[1]=1; 
	end[1]=0;

	// remember number of directed links (so far)
	sz=2; 

	for (i=2; i<NODES; i++) 
	{
		f=0;  // number of new links
		r=-1; // serial number of node whose attachment is already done 

		while (f<2) // meaning that a new node (i) is attached to two(f<2) old nodes
		{
			n=(int)randi(i); // an old node is choosen [e.g. randi(2) is either 0 or 1]; note that rand(i) always selects a node whose attachment is already done ate least once
			if (n!=r) 
			{
				p = (double) Player[n].link/sz;	// this is the probability the old node 'n' gets a link; depending on how many links it already has and what is the total number of links (we consider only directed links for the generation of the scale-free network)
				if (randf()<p)				// if a new link is added to node n, then n(old node) and i(new node) are connected
				{
					Player[n].link++;		// the connectivity of link 'n' increases by 1
					Player[i].link++;		// the connectivity of link 'i' increases by 1
					start[sz] = i;			// beginning of the link pointing out of 'i'
					end[sz] = n;			// ending of the link pointing out of 'i' (points to 'n')
					start[sz+1] = n;		// beginning of the link pointing out of 'n'
					end[sz+1] = i;			// ending of the link pointing out of 'n' (points to 'i')
					sz+=2;					// number of directed links increases by 2 (from 'i' to 'n' and from 'n' to 'i' - we have two directed links)
					f++;					// counts the links of node 'i' (a new i is chosen once two links have been established)
					r=n;					// prevents to connect twice to the same node
				}
			} 
		}
	}

	// exploration of the generated graph: consideration the number of links of a node 
	// now store the generated graph in the p_nb[i][k] array; node 'i' has 'k' neighbours (connectivity 'k')
	
	int LinkCount[NODES];
	for (i=0; i<NODES; i++)  
	{
		LinkCount[i] = 0;	// 记录已经增加到每个个体邻居向量中的个体数
	}

	for (i=0; i<NODES; i++) 
	{
		Player[i].neighbours=new int [Player[i].link] ;// (int *) calloc(Player[i].link, sizeof(int));	// p_nb[][] actually allocates for every node 'i' as much memory as node 'i' has neighbours (node[i])
	}

	for (i=0; i<LINK; i++) 
	{
		Player[start[i]].neighbours[LinkCount[start[i]]]=end[i];  //p_nb[start[i]][p_nb2[0][start[i]]] = end[i];
		LinkCount[start[i]]++;
	}

}

void Initial(void)//初始化函数，形成每个格子的初始状态
{
  
  //初始化个体的邻居
	

  Prodgraph();// creates the scale-free graph
  
}
void Destroy(void)
{
	for (int i=0;i<NODES;i++)
	{
		delete[] Player[i].neighbours;
		Player[i].neighbours=NULL;
	}
}

void main()
{
	sgenrand((unsigned)time(NULL));// initialize RNG

	Initial();// inital strategy distribution

	Destroy();//最后销毁使用过的动态数组
} // main 




























