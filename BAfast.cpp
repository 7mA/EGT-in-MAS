//度相关耦合网上的囚徒困境模型
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <direct.h>
using namespace std;
                                                                //Scale-Free Networks的宏定义
#define SIZE  10000                                               //节点数
#define INI_NODES  3                                            //初始节点数
#define K 4                                                     //平均度,必须为偶数
#define SUMLINKS ( ( SIZE*K-(K-2)*INI_NODES )/2 )               //总边数

struct Agent
{
    int degree;                                                 //记录该点的度
    int *neighbours;                                            //与该点相连的点的序号
    short strategies;                                           // strategies, 0 cooperator, 1 defector.
    double payoff;
};
struct Agent Network[SIZE];
short strnex[SIZE];
int Num[2];                                                     //quantity of cooperators and defectors, respectively.
int edge_sta[ SUMLINKS ]={0};
int edge_end[ SUMLINKS ]={0};                                       //record the information of links

ofstream outfile;//写到文件中
ifstream infile;//读入文件
//以下是随机数产生模块
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

static unsigned long mt[NN]; /* the array for the state vector */
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
		for (kk=0;kk<NN-MM;kk++)
        {
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

//sgenrand((unsigned)time(NULL));// initialize RNG
/*********************************** END of RNG ************************************/
void Set_net( void )                                            //set up scale-free network
{
    int i,j,k;
    int sum;                                                    //记录网络中的边数
    sum = INI_NODES;                                            //初始网络中的边数
                                                                //初始化网络
    for ( i=0; i<INI_NODES; i++ )
    {
	edge_sta[i] = i;
	edge_end[i] = (i + 1) % INI_NODES;
    }                                                           //初始的节点首尾相连
                                                                //开始加边
    int node,ran;                                               //随机选的点
    int LNodes[K/2];                                            //记录新增一个节点时建立边的节点序号
    for (i = INI_NODES; i < SIZE; i++)
    {
	    for ( j=0; j<K/2; )
	    {
	        ran = randi( 2*sum );                                   //从已存在的节点中随机选一个节点，试建立联系
	        if( ran%2 == 0 )
            node = edge_sta[ ran/2 ];
            else
            node = edge_end[ ran/2 ];
            for (k = 0; k < j; k++)
		    {
		        if ( LNodes[ k ] == node )  k = j+1;                //check wheather there is the overlink. If so,relink.
		    }
		    if ( k <= j )
		    {
		        LNodes[j] = node;
		        edge_sta[ sum + j ] = i;
		        edge_end[ sum + j ] = node;
		        j++;                                                //建立一条边，j+1
		    }
	    }                                                           //增长机制加边
	    for ( j--; j>=0; j-- )
	    {
	        Network[i].degree++;
	        Network[ LNodes[j] ].degree++;
	    }
	    sum += K/2;
    }
}                                                               //finish the found of network

void Save_net( void )                                           //save the network we have just founded
{
    int i,j;
	Set_net();
	outfile.open( "Setnet.csv" );
	for( j=0; j<SUMLINKS; j++ )
	{
	    outfile<<edge_sta[j]<<","<<edge_end[j]<<endl;
	}
	outfile.close();
}

void Read_net( void )                                           //read the network
{
    int i;
    char unused;
    infile.open( "Setnet.csv" );
    for( i=0; i<SIZE; i++ )
    {
	Network[i].degree=0;
    }
    if( infile.is_open() )
    {
	for( i=0; i<SUMLINKS; i++ )
	{
	    infile>>edge_sta[i];
	    infile>>unused;
	    infile>>edge_end[i];
	    Network[ edge_sta[i] ].degree++;
	    Network[ edge_end[i] ].degree++;
	}
	infile.close();
    }
    else
    {
	cout<<"can't open file!"<<endl;
    }
    //存储节点的邻接点的信息
    int NodesNumber[SIZE];//记录邻接点的数量
    for ( i=0; i<SIZE; i++ )
    {
	Network[i].neighbours = new int [ Network[i].degree ];
	NodesNumber[i] = 0;
    }
    for ( i=0; i<SUMLINKS; i++)
    {
	Network[ edge_sta[i] ].neighbours[ NodesNumber[ edge_sta[i] ] ] = edge_end[i];
	Network[ edge_end[i] ].neighbours[ NodesNumber[ edge_end[i] ] ] = edge_sta[i];
	NodesNumber[ edge_sta[i] ]++;
	NodesNumber[ edge_end[i] ]++;
    }
}

void Destroy( struct Agent *p )                                 //销毁动态数组
{
    int i;
    for ( i=0; i<SIZE ; i++ )
    {
	delete []p[i].neighbours;
	p[i].neighbours = NULL;
    }
}

int main()
{
    sgenrand((unsigned)time(NULL));                             //initialize RNG
    Save_net();                                                 //including Set_net()
    Read_net();
    Destroy( Network );
    return 0;
}
