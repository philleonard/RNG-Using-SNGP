// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctime>
#include <conio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include "stdafx.h"

#define getch            _getch
void create_tex(char*), bmp();
int pointer = 0;
char *expression = "/+/*%+-J2*21+J%23%%-3J3++*21+J%23+/-022+0-03-*-++0-030*0J-03++0-030J++*21+J%23-+%+-J2*21+J%23*%+J%23*-++0-030*0J-03+J%23//-3J33\0";
int randomiser(int, int);
int protected_div(int, int), protected_mod(int, int);
int seed = 1;
int main()
{
	seed = time(NULL);
	bmp();
	//create_tex(expression);
	printf("Image Created\n");

	getch();
	clock_t start_my = clock();
	for (int i = 0; i < 10000 ; i++) {
		int rand = randomiser(0, 1000);
	}
	clock_t fin_my = clock();
	float my_time = (((float)fin_my - (float)start_my) / 1000000.0F ) * 1000;
	printf("Randomiser = %fsec\n", my_time);


	srand(time(NULL));
	clock_t start_c = clock();
	for (int i = 0; i < 10000 ; i++) {
		int crand = rand()%1000;
	}
	
	clock_t fin_c = clock();
	float c_time = (((float)fin_c - (float)start_c) / 1000000.0F ) * 1000;

	printf("\n\n");
	
	printf("C rand = %fsec\n", c_time);
	getch();
	return 0;
}
 
using namespace std;
void bmp() {

    HANDLE file;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER fileInfo;
    RGBTRIPLE *image;
    DWORD write = 0;
    image = new RGBTRIPLE[128*128*24];
 
	LPCWSTR a = L"random.bmp";
    file = CreateFile(a,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);  //Sets up the new bmp to be written to
 
    fileHeader.bfType = 19778;                                                                    //Sets our type to BM or bmp
    fileHeader.bfSize = sizeof(fileHeader.bfOffBits) + sizeof(RGBTRIPLE);                                                //Sets the size equal to the size of the header struct
    fileHeader.bfReserved1 = 0;                                                                    //sets the reserves to 0
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);                    //Sets offbits equal to the size of file and info header
 
    fileInfo.biSize = sizeof(BITMAPINFOHEADER);
    fileInfo.biWidth = 128;
    fileInfo.biHeight = 128;
    fileInfo.biPlanes = 1;
    fileInfo.biBitCount = 24;
    fileInfo.biCompression = BI_RGB;
    fileInfo.biSizeImage = 128 * 128 * (24/8);
    fileInfo.biXPelsPerMeter = 2400;
    fileInfo.biYPelsPerMeter = 2400;
    fileInfo.biClrImportant = 0;
    fileInfo.biClrUsed = 0;
 
    WriteFile(file,&fileHeader,sizeof(fileHeader),&write,NULL);
    WriteFile(file,&fileInfo,sizeof(fileInfo),&write,NULL);
 
    for (int i = 0; i < 128*128*1; i++)
    {
		if ((rand() % 2) == 1) { //Or substitute; if ((protected_div(2 - (2 - seed), protected_mod(seed, (2 - (protected_div(protected_div(seed, 3), 3))))) % 2) == 1)
			image[i].rgbtBlue = 255;
			image[i].rgbtGreen = 255;
			image[i].rgbtRed = 255;
		}
		else {
			image[i].rgbtBlue = 0;
			image[i].rgbtGreen = 0;
			image[i].rgbtRed = 0;
		}
		seed++;
    }
 
    WriteFile(file, image, fileInfo.biSizeImage, &write, NULL);
 
    CloseHandle(file);
}


int randomiser(int min, int max) {
	max++; //Incrementing Max as to include it as a possible random output.
	int dec = 0;
	//Taking into account a negative minimum
	if (min < 0)
		max = (-1*min) + max;
	int bit_max = (int) ceil(log10((double)max)/log10((double)2)); //Calculating the length of the stream we need
	//For that length we can read in the binary output of the random number generator
	for (int i = 0; i < bit_max; i++) {
		if ((protected_div(2 - (2 - seed), protected_mod(seed, (2 - (protected_div(protected_div(seed, 3), 3))))) % 2) == 1)
			dec = dec + pow((double)2, i);
		seed++; //Running through the sequence as we do.
	}
	//Returning the value between the defined range, taking into account a negative min
	int rand_return = 0;
	if (min < 0)
		rand_return = (dec % max) - (-1*min);
	else
		rand_return = ((dec % (max-min)) + min);
	return rand_return;
}
int protected_div(int num, int dom) {
	if (dom == 0) {
		return (1);
	}
	else {
		return (num/dom);
	}
}

int protected_mod(int num, int dom) {
	if (dom == 0) {
		return (1);
	}
	else
		return (num%dom);
}

void create_tex(char* tex_tree) {
        char a = expression[pointer];
        if (a == '\0')
            return;
        if (a == '+' || a == '*' || a == '/' || a == '-' || a == '%') {
            if (a == '%') 
                printf("[.\\%c ", a); 
            else
                printf("[.%c ", a); 
            pointer++;
            create_tex(tex_tree);
            pointer++;
            create_tex(tex_tree);
            printf("]");
        }
        else {
            printf("%c ", a); 
        }
    }

