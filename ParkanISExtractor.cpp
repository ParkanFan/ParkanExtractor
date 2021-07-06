#include <iostream>
#include <fstream>
#include <Windows.h>

using namespace std;

#define MAX_NAME_LENGTH 100
#define MAX_FILES 50000
#define NOT_NRES_FILE "\nThis file not is 'NRes' type\n"
#define EXTRENSTION_WRONG_ENTERED "\nExtension wrong\n"
#define MAX_FILESIZE 67108864
#define RAM_64		 67108864 

char** list;


char* GetFilename(char* filename, char* extension){
	short i=0, j=0; char* temp;
	temp=new char[strlen(filename)+6];	
	
	if (filename[strlen(filename)-4]=='.'){
		j=4;
	}
	while (i<strlen(filename)-j){
		temp[i]=filename[i];
		i++;
	}
	j=strlen(filename)-j;
	while (i-j<strlen(extension)){
		temp[i]=extension[i-j];
		i++;
	}

	temp[i]='\0';
	return temp;
}

int Unpack_trf(char filename[MAX_NAME_LENGTH])
{
	char Nres[]="NRes", valchar[1], *FileTypes, *buffer;
	unsigned char valuchar, **Names;
	int i, QuantityFiles, EndFile, j=0, valint=0, *Positions, *FileLength, k=0;
	short valsh;
	buffer=new char[4];

	//filename=GetFilename(filename, ".lib");

	ifstream nres(filename, ios::binary);
	for (i=0; i<4; i++)
	{
		nres.read((char*)&valchar,1);
		if (valchar[0]!=Nres[i]) { cout<<NOT_NRES_FILE; system("pause");return -1;}
	}
	nres.read((char*)&i,4);
	nres.read((char*)&QuantityFiles,4);
	nres.read((char*)&EndFile,4);

	Positions=new int [QuantityFiles];
	FileLength=new int [QuantityFiles];
	Names=new unsigned char *[QuantityFiles];

	nres.seekg(EndFile, ios::beg);
	nres.seekg(-8, ios::cur);
	nres.read((char*)&Positions[QuantityFiles],4);
	nres.seekg(-48, ios::cur);
	nres.read((char*)&FileLength[QuantityFiles],4);

	i=FileLength[QuantityFiles]+Positions[QuantityFiles]+3; //diffrence
	nres.seekg(i, ios::beg);
	
	for (j=0; j<QuantityFiles; j++)
	{
		Names[j]=new unsigned char[MAX_NAME_LENGTH];
		k=i=0;
		while (i<12) //12 bit for name
		{
			nres.read((char*)&valuchar,1);
			if ((int)valuchar!=0){	
				if ((int)valuchar>64 && (int)valuchar<123){		Names[j][k]=(int)valuchar; k++;}
				if ((int)valuchar>47 && (int)valuchar<58) {		Names[j][k]=(int)valuchar; k++;}
				
				}
			i++;
		}
		Names[j][k]='\0';
		nres.read((char*)&FileLength[j],4);
		nres.seekg(4, ios::cur);
		i=0;
		nres.seekg(36, ios::cur);
		nres.read((char*)&Positions[j],4);
		if (j+1==QuantityFiles){break;}
		nres.seekg(4, ios::cur);
	}

	//for (j=0; j<QuantityFiles; j++){cout<<Names[j]<<endl;}
	
	for (j=0; j<QuantityFiles; j++)
	{
		ofstream file((char*)Names[j], ios::binary);
		cout<<Names[j]<<endl;
		//nres.seekg(0, ios::beg);
		nres.clear(0);
		nres.seekg(Positions[j], ios::beg);
		i=0;
		while (i<FileLength[j])
		{
			nres.read((char*)&valsh,1);
			file.write((char*)&valsh,1);
			i++;
		}
		file.flush();
		file.close();
	}

}

int Unpack_lib(char filename[MAX_NAME_LENGTH], short step)
{
	char Nres[]="NRes", valchar[1], *FileTypes, *buffer, *Type=new char[4],**Names;
	unsigned char valuchar;
	int i, QuantityFiles, EndFile, j=0, valint=0, *Positions, *FileLength, k=0;
	short valsh;

	//filename=GetFilename(filename, ".lib");

	ifstream nres(filename, ios::binary);
	for (i=0; i<4; i++)
	{
		nres.read((char*)&valchar,1);
		if (valchar[0]!=Nres[i]) { cout<<NOT_NRES_FILE; system("pause");return -1;}
	}
	nres.read((char*)&i,4);
	nres.read((char*)&QuantityFiles,4);
	nres.read((char*)&EndFile,4);

	Positions=new int [QuantityFiles];
	FileLength=new int [QuantityFiles];
	Names=new char *[QuantityFiles];

	nres.seekg(EndFile, ios::beg);
	nres.seekg(-8, ios::cur);
	nres.read((char*)&Positions[QuantityFiles],4);
	nres.seekg(-48, ios::cur); //36 bit for name, 4 bit int, 8 long long
	nres.read((char*)&FileLength[QuantityFiles],4);
	nres.seekg(-16, ios::cur);
	for (j=0; j<4; j++){nres.read((char*)&Type[j],1);}
	Type[j]='\0';

	ofstream dat(GetFilename(filename, ".dat"), ios::binary);
	dat<<Type;
	dat.write((char*)&QuantityFiles,4);
	

	i=FileLength[QuantityFiles]+Positions[QuantityFiles]+step;
	nres.seekg(i, ios::beg);
	
	
	#pragma loop(hint_parallel(4));
	for (j=0; j<QuantityFiles; j++)
	{
		nres.read((char*)&FileLength[j],4);
		nres.seekg(4, ios::cur);
		Names[j]=new char[MAX_NAME_LENGTH];
		k=i=0;
		while (i<36) //36 bit for name
		{
			nres.read((char*)&valchar[0],1);
			if ((int)valchar[0]!=0){	Names[j][k]=valchar[0]; k++;}
			i++;
		}
		Names[j][k]='\0';
		dat.write((char*)&k,4); //NameLength
		dat.write(Names[j], k);
		nres.read((char*)&Positions[j],4);
		nres.read((char*)&k,4); //ID
		dat.write((char*)&k,4);
		if (j+1==QuantityFiles){break;}
		nres.seekg(12, ios::cur);
	}
	buffer=new char[RAM_64];

	#pragma loop(hint_parallel(4));
	for (j=0; j<QuantityFiles; j++)
	{
		//buffer=new char[RAM_64];
		ofstream file((char*)Names[j], ios::binary);
		cout<<Names[j]<<endl;
		//nres.seekg(0, ios::beg);
		nres.clear(0);
		nres.seekg(Positions[j], ios::beg);
		i=0;
		nres.read(buffer, FileLength[j]);
		file.write(buffer, FileLength[j]);
		//delete[] buffer;
		/*
		while (i<FileLength[j])
		{
			nres.read((char*)&valsh,1);
			file.write((char*)&valsh,1);
			i++;
		}
		*/
		file.flush();
		file.close();
	}

	dat.close();
	nres.close();
}

int Allocation(char* filename)
{
	char **ext;
	bool bl=true;
	ext=new char*[4];
	ext[0]="lib\0";
	ext[1]="trf\0";
	ext[2]="rlb\0";
	ext[3]="lib0";

	//if (filename[strlen(filename)-4]!='.')
	//{cout<<EXTRENSTION_WRONG_ENTERED;}
	
		for (int i=0; i<3; i++)
		{
			if (ext[0][i]!=filename[strlen(filename)-3+i]){bl=false; break;}
		}
		if (bl){return Unpack_lib(filename,12);}
		bl=true;
		for (int i=0; i<3; i++)
		{
			if (ext[1][i]!=filename[strlen(filename)-3+i]){bl=false; break;}
		}
		if (bl){return Unpack_trf(filename);}
		bl=true;
		for (int i=0; i<3; i++)
		{
			if (ext[2][i]!=filename[strlen(filename)-3+i]){bl=false; break;}
		}
		if (bl){return Unpack_lib(filename,16);}
		bl=true;
		for (int i=0; i<4; i++)
		{
			if (ext[3][i]!=filename[strlen(filename)-4+i]){bl=false; break;}
		}
		if (bl){return Unpack_lib(filename,18);}

	cout<<EXTRENSTION_WRONG_ENTERED;
	system("pause");
	return !false && !true;

}

int Pack_lib(char* filename)
{
	long long *FileLength;
	int i=0, QuantityFiles/*=(int)list[0][0]*/, FilesSize=0, *Offset,*NameLength, *ID;
	short valsh, j;
	char *head="NRes", *buffer, *Type=new char[4], **Filename,Zeros[1];
	buffer=new char [MAX_FILESIZE];
//	FileLength=new long long [QuantityFiles];
//	Offset=new int [QuantityFiles];
	ofstream lib(GetFilename(filename, ".lib"), ios::binary);
	ifstream dat(GetFilename(filename, ".dat"), ios::binary);

	lib<<head;
	valsh=0;	lib.write((char*)&valsh, 1);
	valsh=1;	lib.write((char*)&valsh, 1);
	valsh=0;	lib.write((char*)&valsh, 1);	lib.write((char*)&valsh, 1);

	dat.seekg(0, ios::beg);
	dat.read(Type,4);
	dat.read((char*)&QuantityFiles,4);

	Filename=new char*[QuantityFiles];
	NameLength=new int[QuantityFiles];
	ID=new int[QuantityFiles];
	FileLength=new long long [QuantityFiles];
	Offset=new int [QuantityFiles];

	#pragma loop(hint_parallel(4));
	for (i=0; i<QuantityFiles; i++)
	{
		dat.read((char*)&NameLength[i],4);
		Filename[i]=new char[NameLength[i]];
		dat.read(Filename[i], NameLength[i]);
		Filename[i][NameLength[i]]='\0';
		dat.read((char*)&ID[i],4);
	}

	lib.write((char*)&QuantityFiles, 4);
	lib.write((char*)&QuantityFiles, 4); //need rewrite
	FilesSize=16;

	buffer=new char[RAM_64];
	#pragma loop(hint_parallel(4));
	for (i=0; i<QuantityFiles; i++)
	{
		ifstream file(Filename[i], ios::binary);		
		file.read(buffer, RAM_64);
		FileLength[i]=file.gcount();
		lib.write(buffer, FileLength[i]);
		if (i==0) {Offset[i]=16;}
		else 
		{
			Offset[i]=Offset[i-1]+FileLength[i-1];
		}
		lib.flush();
		file.close();
		FilesSize+=FileLength[i];
		//delete[] buffer;
	}
	Zeros[0]=(char)0;
	valsh=0;
	#pragma loop(hint_parallel(4));
	for (i=0; i<QuantityFiles; i++)
	{
		lib.write(Type, 4);
		for (j=0; j<8; j++){lib.write((char*)&valsh,1);}
		lib.write((char*)&FileLength[i],8);		
		lib.write(Filename[i], NameLength[i]);
		for (j=0; j<36-NameLength[i]; j++){lib.write((char*)&valsh,1);}
		lib.write((char*)&Offset[i],4);
		lib.write((char*)&ID[i],4);
		FilesSize+=64;
	}
	lib.flush();
	lib.seekp(12, ios::beg);
	lib.write((char*)&FilesSize,4);
	lib.flush();
	lib.close();

	
	dat.close();

return 0;
}

void GetAllFilenames(char* extension)
{
	
	short i=0;
	list=new char*[MAX_FILES];
	WIN32_FIND_DATA win;
	HANDLE han = FindFirstFile(extension,&win);
       if(han != INVALID_HANDLE_VALUE){
		   list[0]=new char [MAX_NAME_LENGTH];
		    list[1]=new char [MAX_NAME_LENGTH];
			strcpy((char*)list[1],win.cFileName);
			cout<<list[1]<<endl;
			i=2;
			while(FindNextFile(han,&win)){
			  if ((win.cFileName[0]!='.')&&(win.cFileName[1]!='.'))
			  {
				  list[i]=new char [MAX_NAME_LENGTH];
				strcpy((char*)list[i],win.cFileName);
				cout<<list[i]<<endl;
				i++;
			  }
			}
			if (i<256)
			{
			list[0][0]=(unsigned char)(i-1); list[0][1]='\0';
			}
			else {cout<<"\nError files > 255\n"; system("pause");}
	//	CloseHandle(han); //hide it's not dangerous
	   } else if (han == INVALID_HANDLE_VALUE){cout<<"Folder for can't be read";}
}

int main(int argc, char* argv[])
{
	char* filename=new char[MAX_NAME_LENGTH], command[1], *ext;
	int i;
	cout<<"Enter switch process [-unpack, +pack]\n";
	cin>>command[0];
	if (command[0]=='-')
	{
	cout<<"\nEnter filename\n";
	cin>>filename;
	return Allocation(filename);
	}
	else if (command[0]=='+')
	{
		/*
		cout<<"\nEnter extension\n.";
		cin>>filename;
		ext=new char[MAX_NAME_LENGTH];
		ext[0]='*';
		ext[1]='.';
		for (i=0; i<strlen(filename); i++)
		{ext[i+2]=filename[i];}
		ext[i+2]='\0';
		cout<<"\nEnter wished filename\n";
		filename=new char[MAX_NAME_LENGTH];
		cin>>filename;
		GetAllFilenames(ext);

		*/
		cout<<"\nEnter wished filename\n";
		cin>>filename;
		return Pack_lib((filename));
	}
	else cout<<"\n Error Command\n"; system("pause");

return 0;
}