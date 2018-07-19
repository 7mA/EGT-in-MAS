#include<iostream>
#include<fstream>
#include<string>
#include<math.h>
#include"num.h"
#include"snapshot.h"

using namespace std;

#define L 100
#define SIZE 10000
//#define cost 0.3
//#define alpha 7.0
#define trans 100
#define MC_STEPS 1000
#define ra 0.1
#define rs 0.04
#define K 10
#define N 10 //重复实验次数

#define x0 0.1
#define x1 0.2
#define x2 0.5
#define x 0.5
double alpha=0;
struct Agent{
	int neighbours[4];
	short strategies;//contains players' strategies,0 cooperator,2 defector, 1 reciprocator
	short new_strat;
    double payoff;
	int get;
	int provide;
	double fairness;
};
int transform[3][3]={0};
double transRate[3][3]={0};
Agent Player[SIZE];
int Num[3]={0,0,0};
int lastNum[3]={0};
color Buffer[L][L];
double rp=0;
double thres=1;
double a=0;
double rd=0;
double rdn=0;
double pun=0;
double cost=0.3;
double xn=0;
//defines a square lattice
void Prodgraph(void)
{
    int iu,ju;
    long int player1,player2;
    int i,j;
    for(i=0;i<L;i++)
    {
        for(j=0;j<L;j++)
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
        }//j
    }//i
}//prodgraph

void initialize(){
	Prodgraph();
	
	//h horizontal, v vertical
	int h=0;
	int v=0;
	double prob=0;
	for(int i=0;i<SIZE;i++){
		prob=randf();
		if(prob<=x0){
			Player[i].strategies=0;
		}
		else if(prob<=x1)
			Player[i].strategies=1;
		else
			Player[i].strategies=2;
		//Player[i].strategies=randi(3);
		Num[Player[i].strategies]++;
		Player[i].payoff=0;
		Player[i].get=0;
		Player[i].provide=0;
		h=i/L;
		v=i%L;
		xn=((double)(Num[0]+Num[1]))/SIZE;

		switch (Player[i].strategies){
		case 0:
			Buffer[h][v].b=0;
			Buffer[h][v].g=0;
			Buffer[h][v].r=255;
			break;
		case 1:
			Buffer[h][v].b=0;
			Buffer[h][v].g=255;
			Buffer[h][v].r=0;
			break;
		case 2:
			Buffer[h][v].b=255;
			Buffer[h][v].g=0;
			Buffer[h][v].r=0;
			break;
		}	
	}
	for(int i=0;i<3;i++){
		lastNum[i]=Num[i];
		for(int j=0;j<3;j++)
			transform[i][j]=0;
	}
}
void initialize2(){
	double prob=0;
	for(int i=0;i<SIZE;i++){
		//prob=randf();
		if(i<4){
			Player[i].strategies=1;
		}
		else
			Player[i].strategies=2;
		Num[Player[i].strategies]++;
		Player[i].payoff=0;
		Player[i].get=0;
		Player[i].provide=0;
	}
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			transform[i][j]=0;
}
void changeColor(){
	int h=0;
	int v=0;
	for(int i=0;i<SIZE;i++){
		h=i/L;
		v=i%L;
		switch (Player[i].strategies){
			case 0:
				Buffer[h][v].b=0;
				Buffer[h][v].g=0;
				Buffer[h][v].r=255;
				break;
			case 2:
				Buffer[h][v].b=0;
				Buffer[h][v].g=255;
				Buffer[h][v].r=0;
				break;
			case 1:
				Buffer[h][v].b=255;
				Buffer[h][v].g=0;
				Buffer[h][v].r=0;
				break;
		}
	}
}

//三种激励策略

void Game(){
	//int ngb=0;
	for(int i=0;i<SIZE;i++){
		for(int j=0;j<4;j++){
			switch(Player[Player[i].neighbours[j]].strategies){
			case 0:
				Player[Player[i].neighbours[j]].provide++;
				Player[i].get++;
				break;
			case 1:
				if(Player[i].strategies==0 || Player[i].strategies==1){
					Player[i].get++;
					Player[Player[i].neighbours[j]].provide++;
				}
				break;
			case 2:
				break;
			}
		}
	}
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/4;
		if(Player[i].strategies==1){
			Player[i].payoff-=cost;
		}
		if(Player[i].strategies==0 ||Player[i].strategies==1){
			//Player[i].payoff+=((double)Num[1]*rd/SIZE);
			Player[i].payoff+=rd;
		}
	}

}

void Gamefc(){
	//int ngb=0;
	for(int i=0;i<SIZE;i++){
		for(int j=0;j<4;j++){
			switch(Player[Player[i].neighbours[j]].strategies){
			case 0:
				Player[Player[i].neighbours[j]].provide++;
				Player[i].get++;
				break;
			case 1:
				if(Player[i].strategies==0 || Player[i].strategies==1){
					Player[i].get++;
					Player[Player[i].neighbours[j]].provide++;
				}
				break;
			case 2:
				break;
			}
		}
	}
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/4;
		/*if(Player[i].strategies==1){
			Player[i].payoff-=cost;
		}*/
		switch(Player[i].strategies){
		case 0:
			if(xn <= x)
				Player[i].payoff+=rdn;
			break;
		case 1:
			if(xn <= x)
				Player[i].payoff+=rdn;
			break;
		case 2:
			if(xn > x)
				Player[i].payoff-=rdn;
			break;
		}
	}

}

void gameMixed(){
	double prop=(double)(Num[0]+Num[1])/SIZE;
	for(int i=0;i<SIZE;i++){
		for(int j=0;j<SIZE;j++){
			switch(Player[j].strategies){
			case 0:
				Player[j].provide++;
				Player[i].get++;
				break;
			case 1:
				if(Player[i].strategies==0 || Player[i].strategies==1){
					Player[i].get++;
					Player[j].provide++;
				}
				break;
			case 2:
				break;
			}
		}
	}
	
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/4;
		if(Player[i].strategies==1)
			Player[i].payoff-=cost;
		if(Player[i].strategies==0 || Player[i].strategies==1)
			Player[i].payoff+=rd;
	}
	/*for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/SIZE;
		if(Player[i].strategies==1)
			Player[i].payoff-=cost;
		if(Player[i].strategies==0 || Player[i].strategies==1)
			Player[i].payoff+=(rd*prop);
		if(Player[i].strategies==2)
			Player[i].payoff-=(pun*(1-prop));
	}*/
}

void gameMixed2(){
	double prop=(double)(Num[0]+Num[1])/SIZE;
	for(int i=0;i<SIZE;i++){
		for(int j=0;j<SIZE;j++){
			switch(Player[j].strategies){
			case 0:
				Player[j].provide++;
				Player[i].get++;
				break;
			case 1:
				if(Player[i].strategies==0 || Player[i].strategies==1){
					Player[i].get++;
					Player[j].provide++;
				}
				break;
			case 2:
				break;
			}
		}
	}
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/SIZE;
		if(Player[i].strategies==1)
			Player[i].payoff-=cost;
		if(Player[i].strategies==0 || Player[i].strategies==1){
			//if(prop>0)	
				//Player[i].payoff+=Num[2]/SIZE;
				Player[i].payoff+=rd;
		}
	}


	/*for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/SIZE;
		if(Player[i].strategies==1)
			Player[i].payoff-=cost;
		if(Player[i].strategies==0 || Player[i].strategies==1)
			Player[i].payoff+=(rd*prop);
		if(Player[i].strategies==2)
			Player[i].payoff-=(pun*(1-prop));
	}*/
}

//first carrot
void gameMixed2fc(){
	double prop=(double)(Num[0]+Num[1])/SIZE;
	for(int i=0;i<SIZE;i++){
		for(int j=0;j<SIZE;j++){
			switch(Player[j].strategies){
			case 0:
				Player[j].provide++;
				Player[i].get++;
				break;
			case 1:
				if(Player[i].strategies==0 || Player[i].strategies==1){
					Player[i].get++;
					Player[j].provide++;
				}
				break;
			case 2:
				break;
			}
		}
	}
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=(alpha*Player[i].get-Player[i].provide)/SIZE;
		if(Player[i].strategies==1)
			Player[i].payoff-=cost;
		switch(Player[i].strategies){
		case 0:
			if(xn <= x)
				Player[i].payoff+=rdn;
			break;
		case 1:
			if(xn <= x)
				Player[i].payoff+=rdn;
			break;
		case 2:
			if(xn > x)
				Player[i].payoff-=rdn;
			break;
		}
	}
	
}
//算各种收益

void clear(){
	for(int i=0;i<SIZE;i++){
		Player[i].payoff=0;
		Player[i].get=0;
		Player[i].provide=0;
	}
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			transform[i][j]=0;
}

//以一定的概率学习邻居中收益最好的策略（包括自己）
void Fermi(){
	//cout<<"Fermi"<<endl;
	for(int k=0;k<3;k++)
		lastNum[k]=Num[k];
	for(int i=0;i<SIZE;i++){
		int j=Player[i].neighbours[randi(4)];
		double prob=1/(1+exp((Player[i].payoff-Player[j].payoff)/K));
		if(randf()<prob){
			Player[i].new_strat=Player[j].strategies;
			Num[Player[i].strategies]--;
			Num[Player[j].strategies]++;
		}
		else
			Player[i].new_strat=Player[i].strategies;
		//Player[i].strategies = (randf() < 0.01 ? 2 - Player[i].strategies : Player[i].strategies);
	}
	for(int i=0;i<SIZE;i++){
		Player[i].strategies=Player[i].new_strat;
	}
}

void FermiM(){
	//cout<<"Fermi"<<endl;
	for(int k=0;k<3;k++)
		lastNum[k]=Num[k];
	for(int i=0;i<SIZE;i++){
		int j=randi(SIZE);
		double prob=1/(1+exp((Player[i].payoff-Player[j].payoff)/K));
		if(randf()<prob){
			Player[i].new_strat=Player[j].strategies;
			Num[Player[i].strategies]--;
			Num[Player[j].strategies]++;
		}
		else
			Player[i].new_strat=Player[i].strategies;
	}
	for(int i=0;i<SIZE;i++){
		transform[Player[i].strategies][Player[i].new_strat]++;
		Player[i].strategies=Player[i].new_strat;
	}
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++)
			if(lastNum[i]!=0)
				transRate[i][j]=(double)transform[i][j]/lastNum[i];
			else
				transRate[i][j]=0;
	}
}

void main(){
	sgenrand((unsigned)time(NULL));//initialize RNG
	ofstream file;
	file.open("dyf.csv");
	//initialize();
	/*cout<<sizeof(SIZE)<<endl;
	double rate=(double)Num[0]/SIZE;
	cout<<rate<<endl;*/
	//cout<<(double)Num[0]/SIZE<<" "<<(double)Num[1]/SIZE<<" "<<(double)Num[2]/SIZE<<endl;
	string filename = "";
	string str0 = "Fermi_P";

	string str1 = ".bmp";
	alpha = 1.0;

	cost = 0.3;
	double averCost[21] = { 0.0 };
	double gameSteps[21] = { 0.0 };
	double tempSteps = 0.0;
	int count = 0;

	//cost=0.1;

	//cost=0.2;

	for (int i = 10; i <= 10; i += 1){
		count = 0;
		while (count < N){
			//rd=0.08;
			//rd=0.1*alpha;
			double pro = 0;
			double syscost = 0;
			double Tsyscost = 0;
			double tempSteps = 0.0;
			double delta = 0.05;
			// alpha=1.1;
			// for(pro=0;pro<0.3;pro+=0.05){
			//cost=0.1;

			//rd=0.1; 

			double deltaR = 0.0;
			double context, fn = 0.0;
			rdn = i;

			//rd=0.25;

			//rd=0.3;

			double d1 = 0;
			double d2 = 0;
			cout << "rd=" << pro << "*" << alpha << "=" << cost << endl;
			//file<<"rd="<<pro<<"*"<<alpha<<"="<<cost<<endl;
			Num[0] = Num[1] = Num[2] = 0;
			initialize();
			cout << a << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << endl;

			for (int steps = 0; steps < MC_STEPS; steps++){
				clear();

				//rd=1-(double)Num[1]*alpha/SIZE+delta;
				/*	 if(d1>d2)
						 rd=d2;
						 else
						 rd=d1;*/
				//rd=cost-(alpha-1-(alpha-3)*Num[0]/SIZE-2*cost)/4+0.1;
				//rd=(2*alpha+2*(double)Num[0]/SIZE+2*cost*(1-Num[1]/SIZE)-2*((alpha-1)*Num[1]/SIZE-Num[0]/SIZE)-(alpha-1)*Num[2]/SIZE)/4+0.05;

				//rd=cost+(double)Num[0]/SIZE+(1-alpha)*(double)Num[1]/SIZE+delta;//混合均匀中rd函数

				//rd=((3*(double)Num[0]-3*(alpha-1)*Num[1]-(alpha-1)*Num[2])/SIZE)/4+cost+0.15;//网格中rd函数
				//rd=cost-((double)(3*(alpha-1)*Num[1]+(alpha-1)*Num[2])/SIZE)/4+0.15;//网格中rd函数

				//if(rd<0 || Num[2]==0)
				//		rd=0;

				// else
				//	 rd=0;

				xn = ((double)(Num[0] + Num[1])) / SIZE;//xn

				//syscost=SIZE*rd;

				if (xn <= x) syscost = xn * rdn * SIZE;
				else syscost = (1 - xn) * rdn * SIZE;

				/*if(xn <= x)
				{
				if(Num[0]+Num[1]!=0 && Num[2]!=0 )
				rdn=SIZE*rd/(double)(Num[0]+Num[1]);
				else
				{
				rdn=0;
				syscost=0;
				}
				}
				else
				{
				if(Num[2])
				rdn=SIZE*rd/(double)Num[2];
				else
				{
				rdn=0;
				syscost=0;
				}
				//根据合作情况选择惩罚或奖励
				}*/

				//syscost=(Num[0]+Num[1])*rd;//只执行奖励

				Tsyscost += syscost;
				if (steps % 2 == 0){
					//rd=alpha*Num[2]/SIZE;
					//cout<<"rd="<<rd<<endl;



					if (xn <= x)
					{
						//syscost=(Num[0]+Num[1])*rd;

						//cout<<"reword xn=,"<<xn<<",steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<endl;
						//file<<"reword xn=,"<<xn<<",steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<endl;
						cout << "reword xn=," << xn << ",steps," << steps << ",rdn=" << rdn << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << ",Scost," << syscost << ",Tscost," << Tsyscost << ",Context," << fn << endl;
						file << "reword xn=," << xn << ",steps," << steps << ",rdn=" << rdn << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << ",Scost," << syscost << ",Tscost," << Tsyscost << ",Context," << fn << endl;
					}
					else
					{
						//syscost=Num[2]*rd;

						//cout<<"punish xn=,"<<xn<<",steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<endl;
						//file<<"punish xn=,"<<xn<<",steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<endl;
						cout << "punish xn=," << xn << ",steps," << steps << ",rd=" << rdn << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << ",Scost," << syscost << ",Tscost," << Tsyscost << endl;
						file << "punish xn=," << xn << ",steps," << steps << ",rd=" << rdn << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << ",Scost," << syscost << ",Tscost," << Tsyscost << endl;
						//根据合作情况选择惩罚或奖励
					}

					//double syscost=((Num[0]+Num[1])*alpha*Num[2]/SIZE)/SIZE;
					//double syscost=(Num[0]+Num[1])*rd+Num[2]*pun;

					//cout<<"steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<",Tscost,"<<Tsyscost<<endl;
					//file<<"steps,"<<steps<<",rd="<<rd<<",C,"<<((double)Num[0])/SIZE<<",R,"<<(double)Num[1]/SIZE<<",D,"<<(double)Num[2]/SIZE<<",Scost,"<<syscost<<",Tscost,"<<Tsyscost<<endl;

					/*for(int i=0;i<3;i++){
						if(i>0)
						cout<<endl;
						for(int j=0;j<3;j++)
						cout<<transRate[i][j]<<"\t";
						}*/
					//cout<<endl;
					//cout<<"0->1,"<<(double)transform[0][1]/lastNum[0]<<endl;
					//file<<"steps,"<<steps<<",C,"<<(double)Num[0]/SIZE<<",D,"<<(double)Num[1]/SIZE<<",R,"<<(double)Num[2]/SIZE<<endl;	
					filename=str0+int_to_str(steps)+str1;
					changeColor();
						SaveBmp(filename.data(),L,L,(unsigned char *)Buffer);
				}
				//Game();
				// for(int i=0;i<SIZE;i++){
				// if(i>0 && i%10==0)
				//	 cout<<endl;
				// cout<<i<<"\t"<<Player[i].payoff<<"\t";
				//}
				//Fermi();

				//if(Num[2]<=5000)
				//break;

				//Gamefc();
				//Fermi();

				//gameMixed2();
				//FermiM();

				gameMixed2fc();

				//公平度用到的Context
				deltaR = 0.0;
				for (int i = 0; i < SIZE; i++){
					context = 0.0;
					for (int j = 0; j < SIZE; j++){
						context += 1 - Player[j].strategies; //C as 1, D as -1, R as 0
					}
					if (Player[i].strategies == 0 || Player[i].strategies == 2){
						Player[i].fairness = (1 - Player[i].strategies) * context / SIZE;
					}
					else{
						Player[i].fairness = context * context / (SIZE * SIZE);
					}

					if (Player[i].strategies == 0 || (Player[i].strategies == 1 && context >= 0)){
						deltaR += /*(1 - (Player[i].fairness + 1) / 2) * */(1 - Player[i].payoff); //(1 -（F+1）/2) * (Pdc - P)
					}
					else if (Player[i].strategies == 2 || (Player[i].strategies == 1 && context < 0)){
						deltaR += /*((Player[i].fairness + 1) / 2) * */(Player[i].payoff + alpha);
					}
					fn += Player[i].fairness;
				}
				rdn += deltaR / SIZE;
				rdn = 1.05 * rdn;
				fn /= SIZE; //暂存平均fairness结果以备测试	
				//cout << rdn << endl;*/

				//rdn = (xn == 1 ? 0 : rdn);

				FermiM();

				if (xn == 1) {
					tempSteps = (double)steps;
					break;
				}
				//cout<<endl;
				// }
			}
			 filename=str0+dub_to_str(a)+int_to_str(MC_STEPS)+str1;
			changeColor();
			SaveBmp(filename.data(),L,L,(unsigned char *)Buffer);
			// cout<<endl;
			// cout<<a<<",C,"<<((double)Num[0])/SIZE<<",D,"<<(double)Num[1]/SIZE<<",R,"<<(double)Num[2]/SIZE<<endl;
			// file<<a<<",C,"<<((double)Num[0])/SIZE<<",D,"<<(double)Num[1]/SIZE<<",R,"<<(double)Num[2]/SIZE<<endl;
			cout << "steps," << MC_STEPS << ",C," << ((double)Num[0]) / SIZE << ",R," << (double)Num[1] / SIZE << ",D," << (double)Num[2] / SIZE << endl;
			file << "steps," << MC_STEPS << ",rd=" << rd << ",C," << (double)Num[0] / SIZE << ",D," << (double)Num[1] / SIZE << ",R," << (double)Num[2] / SIZE << endl;
			//		file<<endl;

			file.close();

			 SaveBmp("0.bmp",L,L,(unsigned char *)Buffer);
			 SaveBmp("1.bmp",L,L,(unsigned char *)Buffer);
			 string str="";
			 int inum=100;
			 //string str1=".bmp";
			 str=int_to_str(inum)+str1;
			 cout<<str<<endl;
			averCost[i] += Tsyscost;
			gameSteps[i] += tempSteps;
			count++;
		}
		averCost[i] /= N;
		gameSteps[i] /= N;
		cout << "gameStep:" << gameSteps[i] << endl;
	}

	for (rdn = 5; rdn <= 20; rdn += 1){
		cout << "gameStep with " << rdn << ":" << gameSteps[(int)rdn] << endl;
	}
	getchar();
}