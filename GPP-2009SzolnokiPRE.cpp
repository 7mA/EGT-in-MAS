//  multi-group public goods game on square lattice for "PHYSICAL REVIEW E 80, 056109 (2009)"                        
// written by 王震 June, 2010     
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <windows.h>
using namespace std;


#define L           100       // lattice size
#define SIZE        (L*L)     // number of players
#define MC_STEPS    50000  // run-time in MCS
//#define r           1.4   //增长率
#define K           0.5   //噪音系数
double r=1.4; //增长率
struct Agent
{
	short strategies; //contains players' strategies ,0为合作者，1为背叛者
	int neighbours[4]; // contains players' neighbours
};

Agent Player[SIZE];
int Num[2];  //用于存放每种策略的个体的人数

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
	

void Prodgraph(void)             
// defines neighbors on square lattice 
{ int iu, ju;
  long int player1, player2;
  int i,j;
 
  for(i=0; i<L; i++)                     
  {
   for(j=0; j<L; j++)
   { 
      player1 = L * j + i;       /* consider a site >> a player */
       iu = i + 1;  ju = j;           /* east */
         if (iu==L) iu = 0;
        player2 = L * ju + iu;   /* the location of player2 */
        Player[player1].neighbours[0] = player2;      /* the east link of player1 ends at player 2 */
       iu = i;      ju = j + 1;       /* south */
         if (ju==L) ju = 0;
        player2 = L * ju + iu;
        Player[player1].neighbours[1] = player2;
       iu = i - 1;  ju = j;           /* west */
         if (i==0) iu = L - 1;
        player2 = L * ju + iu;
        Player[player1].neighbours[2] = player2;
       iu = i;     ju = j - 1;       /* north */
         if (j==0) ju = L - 1;
        player2 = L * ju + iu;
        Player[player1].neighbours[3] = player2;
   } // j
  } // i
} // prodgraph 
		   
void Initial(void)//初始化函数，形成每个格子的初始状态
{
  Num[0]=0;Num[1]=0;
  for (int i=0;i<SIZE;i++)
  {
	  Player[i].strategies=(short)randi(2);//0为合作者，1为背叛者
	  Num[Player[i].strategies]++;
  }
  //初始化个体的邻居
  Prodgraph();
}

double Game(int x)
{
	//calculates the payoff of palyer x
	double payoff=0.0,Nc=0;
	int strat=Player[x].strategies,center;
	for (int nbh=0; nbh<5; nbh++)
	{
		Nc=0;
		
		//确定中心个体
		if (nbh==4)
		{
			center=x;
		}
		else
		{
			center=Player[x].neighbours[nbh];
		}

		if (Player[center].strategies==0)
		{
			Nc++;
		}

		for (int cn=0;cn<4;cn++)
		{
			int y=Player[center].neighbours[cn];
			if (Player[y].strategies==0)
			{
				Nc++;
			}
		}

		payoff+= r*Nc/5-(1-strat);
	}
	return payoff;
}

void ChangeStrat(int playerX,double payoffX,double payoffY)
{
	double probability=1/(1+exp((payoffX-payoffY)/K));
	if (randf()<probability)
	{
		Num[Player[playerX].strategies]--;
		Num[1-Player[playerX].strategies]++;
		Player[playerX].strategies=1-Player[playerX].strategies;

	}
}			 
void main()
{
	sgenrand((unsigned)time(NULL));// initialize RNG
	int steps,i,playerX,playerY,stratX,stratY;	
	double payoffX,payoffY,P_c;
	ofstream outfile;
	outfile.open("data.txt");
	for (r=3.4;r<6.0;r+=0.5)
	{
		Initial();// inital strategy distribution
		P_c=0;
		for (steps=0; steps<MC_STEPS; steps++)
		{ 
			for (i=0; i<SIZE; i++)
			{
				
				playerX = (int) randi(SIZE);      // choose a source site
				playerY = Player[playerX].neighbours[(int) randi(4)];  // choose the target site
				
				stratX = Player[playerX].strategies;       // strategy of the source site
				stratY = Player[playerY].strategies;       // the strategy of the target site

				if ((stratX+stratY) == 1)         // source and target have different strategies
				{
					payoffX=Game(playerX);
					payoffY=Game(playerY);
					ChangeStrat(playerX,payoffX,payoffY);
				}//end of a source selection
			} // end of elementary MC step
			if (steps>=45000)
			{
				P_c+=Num[0];
			}
		} //end of MC
		P_c=P_c/(SIZE*5000);
		outfile<<r/5<<"\t"<<P_c<<endl;
		cout<<r/5<<"\t"<<P_c<<endl;


	}//end of r
	outfile.close();

    

    
  
} // main 
