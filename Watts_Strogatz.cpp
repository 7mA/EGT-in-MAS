//从一维格子出发通过断边重连的方式产生小世界网络，每个边有Q的可能性被断掉
//初始每个个体和左右各R个邻居建立联系，网络的平均度为2*R
#include<iostream.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<math.h>
#include<fstream.h>

//define parameters

#define SIZE  10000         /*  number of sites  */
#define R   4          //平均度的一半,初始个体和左右各R个邻居建立联系
#define Q  0.03            /*  Q portion o flinks are rewired  */


struct Agent
{
	int *neighbours; // contains players' neighbours
	int link;//记录每个节点的度数
};

Agent Player[SIZE];
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
// defines neighbors on  //creat first a one-dimension lattice and then rewires Q links
{ 
	
	int* Start=new int [R*SIZE];
	int* End=new int [R*SIZE];
	int* AddedLink=new int [SIZE]; 
	//int Start[SIZE],End[SIZE];
	//int AddedLink[SIZE];
	int first,next,newend,temp,Linked;
	int i,j;
	//建立初始规则网络
	for (i=0;i<SIZE;i++)
	{
		for (j=0;j<R;j++)
		{	
			Start[i*R+j]=i;
			End[i*R+j]=(i+j+1)%SIZE;
		}
		Player[i].link=2*R;//初始每个节点的度都是2
		AddedLink[i]=0;
	}
	//段边重连
	for (i=0;i<R*SIZE;i++)
	{
		if (randf()<Q)//每条边有Q得可能性被断掉重连
		{
			first=Start[i];next=End[i];
			do{
				newend=randi(SIZE);
				Linked=-1;

				if (newend==first||newend==next)
				 {
					 Linked=1;
				 }
				 else
				 {
					 for (j=0;j<R*SIZE;j++)
					 {
						 if (Start[j]==first&&End[j]==newend)
							 Linked=1;
						 if (Start[j]==newend&&End[j]==first)
							 Linked=1;
					 }
				 }
			}while(Linked==1);
			End[i]=newend;
			Player[next].link--;
			Player[newend].link++;
		}
	}
	for (i=0; i<SIZE; i++) 
	{
		Player[i].neighbours=new int [Player[i].link] ;// (int *) calloc(Player[i].link, sizeof(int));	// p_nb[][] actually allocates for every node 'i' as much memory as node 'i' has neighbours (node[i])
	} 
	for (i=0;i<R*SIZE;i++)
	{
		Player[Start[i]].neighbours[AddedLink[Start[i]]]=End[i];
		AddedLink[Start[i]]++;
		Player[End[i]].neighbours[AddedLink[End[i]]]=Start[i];
		AddedLink[End[i]]++;
	}
	delete []Start;
	delete []End;
	delete []AddedLink;

} // prodgraph 	


//initial distribution of strategies
void initial()
{
	Prodgraph();
}


void main()
{
	sgenrand((unsigned)time(NULL));// 随机数种子
	initial();
	ofstream outfile;
	outfile.open("data.txt");
	for (int i=0;i<SIZE;i++)
	{
		outfile<<i<<"\t";
		for (int j=0;j<Player[i].link;j++)
		{
			outfile<<Player[i].neighbours[j]<<"\t";
		}
		outfile<<endl;
	}
}
