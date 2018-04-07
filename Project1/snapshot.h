#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

class color
{
public:
	unsigned char b,g,r;
};

string int_to_str(int number)
{
	stringstream s;
	s<<number;
	string str=s.str();
	return str;
} 

string dub_to_str(double number)
{
	stringstream s;
	s<<number;
	string str=s.str();
	return str;
} 

void SaveBmp(const char* filename,int height,int width,unsigned char *data){
	unsigned int size = width*height*3+54;
	unsigned short head[]={
		0x4D42,size%0x10000,size/0x10000,0,0,0x36,0,0x28,
		0,width%0x10000,width/0x10000,height%0x10000,height/0x10000,0x10,0x18,0,
		0,0,0,0,0,0,0,0,0,0,0
	};
	FILE* fp=fopen(filename,"wb");
	if(!fp)
	{
		cout << "image open failed" << endl;
		getchar();
		exit(1);
		//return;
	}
	fwrite(head,1,sizeof(head),fp);
	fwrite(data,1,size,fp);
	fclose(fp);
}

#endif