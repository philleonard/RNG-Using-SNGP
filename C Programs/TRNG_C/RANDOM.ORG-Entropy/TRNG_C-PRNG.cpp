#define _CRT_SECURE_NO_WARNINGS

//Check other c files for consistency with handling J sized char arrays

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <dos.h>
#include <ctime>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <wininet.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib,"wininet.lib")

#define H_MAX 7
#define J 16384
#define MAX_ENTROPY (H_MAX * (H_MAX+1))/2
#define itoa            _itoa
#define getch _getch

int KMP_matching (char *t, char *p);
void compute_prefix_function(char *p, int m, int *pi), get_binary_sub_seq(char* j2, int j, int h), fitness_function(), get_total(), get_c_rand_seq(), download_bits(), check_quota();
void write_data(int), print_progress(char* text, int progress, int w, int n), write_header();
int create_file_name(char c), get_true_rand_seq();

char bit_seq[J+1];
double scalar_entropy[H_MAX];
double e_scalar[H_MAX];
double total_entropy = 0;
char file_name[100];
double sum_total = 0;
void bmp();

char file_time[50];
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
		if (bit_seq[i] == '1') {
			image[i].rgbtBlue = 255;
			image[i].rgbtGreen = 255;
			image[i].rgbtRed = 255;
		}
		else {
			image[i].rgbtBlue = 0;
			image[i].rgbtGreen = 0;
			image[i].rgbtRed = 0;
		}
		
    }
 
    WriteFile(file, image, fileInfo.biSizeImage, &write, NULL);
 
    CloseHandle(file);
 
}


int main() {
	char c;
	int seed = time(NULL);
	srand(seed);
	while (true) {
		while (true) {
			system("cls"); 
			printf("OPTIONS [t: Random.org TRNG | q: Random.org Quota | p: C Rand | e: Exit]");
			c = getch();
			printf("\n\n");
			if (c == 't' || c == 'p' || c == 'q') {
				break;
			}
			if (c == 'e')
				return 0;
		}
		
		if (c == 'q') {
			//remove("BitQuota.txt");
			printf("\n            Retrieving Bit quota from www.random.org...\n");
			check_quota();
		}
		

		else if (c == 't' || c == 'p') {
			int runs = 0;
			do {
				printf("Enter Number of Runs (greater than 0): ");
				scanf("%d", &runs);
			} while(runs < 1);
			
			printf("\n");
			create_file_name(c);
			write_header();
			sum_total = 0;
			bool cont = true;
			for (int i = 0; i < runs; i++) {
				
				if (c == 't') {
					download_bits();
					int x = get_true_rand_seq();
					bmp();
					if (x == 1) {
						cont = false;
						break;
					}
					else
						cont = true;
				}

				else if (c == 'p') {
					get_c_rand_seq();
				}

				
				if (cont) {
					fitness_function();
					get_total();

					sum_total += total_entropy;
					write_data(i + 1);

					print_progress("Gather & Proc: ", i + 1, 50, runs);
				}
			}

			if (cont) {
				printf("\n\n");
				double average_entropy = sum_total/runs;

				if (c == 't')
					printf("\n     ========== www.random.org Entropy Evaluation ==========\n");
				else if (c == 'p') 
					printf("\n     ========= C rand() Funtion Entropy Evaluation =========\n");

				printf("                Average: %f bits of entropy        \n", average_entropy);
				int max = MAX_ENTROPY;
				double percentage = (average_entropy / max) * 100;
				printf("                   Average: %.2f%% entropy                \n", percentage);
				printf("     =======================================================\n\n");
			}
		}
		printf("                 PRESS ANY KEY TO CONTINUE...");
		getch();
	}
}

void write_header() {
	FILE *fp;
	fp = fopen(file_name, "a");

	char line[1024] = "Run No., Total Entropy, ";

	for (int i = 0; i < H_MAX; i++) {
		char this_scalar[100];
		if (i + 1 == H_MAX)
			sprintf(this_scalar, "Scalar Entropy (%d bit)", i + 1); 
		else
			sprintf(this_scalar, "Scalar Entropy (%d bit), ", i + 1); 
		strcat(line, this_scalar);
	}

	fprintf(fp, "%s\n", line);
	fclose(fp);
}

void print_progress(char* text, int progress, int w, int n) {
 
    float ratio = progress / (float) n;
    int c = ratio * w;
 
	if (ratio == 1)
		printf("\r%s%d%% [", text, (int)(ratio*100) );
	else if (ratio < 0.1)
		printf("\r%s%d%%   [", text, (int)(ratio*100) );
	else
		printf("\r%s%d%%  [", text, (int)(ratio*100) );
 
    for (int progress=0; progress<c; progress++)
       printf("=");
    for (int progress=c; progress<w; progress++)
       printf(" ");

	printf("]");
    fflush(stdout);

}

void get_time() {
	time_t rawtime;
	time (&rawtime);
	struct tm  *timeinfo = localtime (&rawtime);
	strftime(file_time, sizeof(file_time)-1, "%d.%m.%y_%H.%M.%S\0", timeinfo);
}

int create_file_name(char c) {
	get_time();

	if (c == 'p')
		sprintf(file_name, "C_Rand()_PRNG-log-%s.txt\0", file_time);
	else if (c == 't')
		sprintf(file_name, "Random.ORG_TRNG-log-%s.txt\0", file_time);

	printf("Writing Data to: %s\n", file_name);

	return 1;
}

void write_data(int run_number) {
	FILE *fp;
	fp = fopen(file_name, "a");

	char line[1024];
	char scalar[256];
	int length = 0;
	for (int i = 0; i < H_MAX; i++) {
		if (i + 1 != H_MAX)
			length += sprintf(scalar + length, "%f, ", e_scalar[i]);
		else 
			length += sprintf(scalar + length, "%f", e_scalar[i]);
	}

	scalar[length + 1] = '\0';

	sprintf(line, "%d, %f, %s\n", run_number, total_entropy, scalar);
	fprintf(fp, "%s", line);
	fclose(fp);
}

void check_quota() {
	HRESULT hr;
	LPCTSTR Url = _T("http://www.random.org/quota/?format=plain"), File = _T("BitQuota.txt");
	DeleteUrlCacheEntry(Url);
	hr = URLDownloadToFile (0, Url, File, 0, 0);
	

	FILE *file;
	file = fopen("BitQuota.txt", "r");
	char c;
	int x = 0;
	char quota[10];
	while ((c = getc(file)) != EOF) {
		quota[x] = c;
		x++;
	}
	quota[x] = '\0';
	int quota_int = atoi(quota);
	double percentage = ((double)quota_int / 1000000) * 100;
	printf("\n     ================ www.random.org Quota =================\n");
	printf ("                    %d Bits Remaining         \n", quota_int);
	printf("     =======================================================\n\n");
	fclose(file);
}

void download_bits() {
	HRESULT hr;
	LPCTSTR Url = _T("http://www.random.org/cgi-bin/randbyte?nbytes=2048&format=b.txt"), File = _T("RandomBits.txt");
	DeleteUrlCacheEntry(Url);
	hr = URLDownloadToFile (0, Url, File, 0, 0);
	
}

void get_c_rand_seq() {
	for (int i = 0; i < J; i++) {
		int bit = rand()%2;
		if (bit == 0)
			bit_seq[i] = '0';
		else 
			bit_seq[i] = '1';
	}
}

void get_total() {
	total_entropy = 0;
	for (int x = 0; x < H_MAX; x++) {
		total_entropy = total_entropy + e_scalar[x];
	}
}

int get_true_rand_seq() {
	FILE *file;
	file = fopen("RandomBits.txt", "r");
	char c;
	int position = 0;

	if (file) {
		
		for (position; position < J; position++) {
			c = getc(file);

			while (c != '1' && c != '0')
				c = getc(file);
			bit_seq[position] = c;

		}
		//printf("POSITION: %d\n", position);
		bit_seq[J] = '\0';
	}
	else {
		printf("\nERROR: Failed to Download and Read Random Bits. Check internet connection or status of the servers\n\n");
		return 1;
	}
	fclose(file);
	//printf("%d", sizeof(bit_seq));
	return 0;
}

void fitness_function() {

	for (int g = 0; g < H_MAX; g++) {
		e_scalar[g] = 0;
	}

	for (int h = 1; h <= H_MAX; h++) {
		
		int total_occ = 0;

		int j = 0;
		
		int limit = (pow(2, (double) h) - 1);
		int occourrence;
		
		int occurrence_array[1024];
		int array_count = 0;

		for (j; j <= limit; j++) {
			
			occourrence = 0;

			char j2[H_MAX + 1];
			get_binary_sub_seq(j2, j, h);
			
			occourrence = KMP_matching(j2, bit_seq);

			occurrence_array[array_count] = occourrence;
			array_count++;

			total_occ += occourrence;		
		}

		//Calculating entropy from probabilities of occourence of subsequences in bin_seq
		for (int z = 0; z < array_count; z++) {
			int occ = occurrence_array[z];
			//printf("\nOccourrance = %d", occ);
			double prob = 0;
			if (total_occ != 0) 
				prob = occ / (double)total_occ;

			double e_h_increment;
			if (prob == 0 || prob == 1) //Avoiding log(0) and negative zero
				e_h_increment = 0;
			else
				e_h_increment = (-1 *((prob * log10(prob)) / log10(2.0)));
			
			//printf("\n%f", e_h_increment);
			e_scalar[h - 1] = e_scalar[h - 1] + e_h_increment;
		}
		//printf("\nEntropy for %d = %f", h, e_scalar[h - 1]);
		
	}
	for (int k = 0; k < H_MAX; k++) {
		scalar_entropy[k] = e_scalar[k];
	}
}

void get_binary_sub_seq(char* j2, int j, int h) {
	
			char j2_bin[H_MAX + 1];

			for (int x = 0; x < H_MAX + 1; x++) {
				j2_bin[x] = '\0';
			}

			itoa(j, j2_bin, 2);
			
			//printf("%s\n", j2_bin);

			int y = 0;
			for (y = 0; y < H_MAX + 1; y++) {
				if (j2_bin[y] == '\0')
					break;
			}

			char zeros[H_MAX];
			int f = 0;
			char zero = '0';
			//printf("y = %d\n", y);
			for (f; f < h - y; f++) {
					zeros[f] = zero;
			}
			zeros[f] = '\0';
			
			//printf("%s\n", zeros);

			sprintf(j2, "%s%s", zeros, j2_bin);

}

int KMP_matching(char *p, char *t) {
	int n = strlen(t);
    int m = strlen(p);
    
	int matches = 0;

    int *pi = (int *)malloc(sizeof(int)*m);
    int j  = 0;
    compute_prefix_function(p, m, pi);
 
    int i = 0; 
    while(i < n - m) {
		if(p[j] == t[i]) {
		  j++;
		  i++;
		}
		if (j == m) {
		  matches++;
		  j = pi[j-1];
		}

		else if(p[j] != t[i]) {
			if(j != 0)
				j = pi[j-1];
			else
				i = i+1;
		}
    }
    free(pi);

	return matches;
}
 
void compute_prefix_function(char *p, int m, int *pi) {
    int len = 0;
    int i;
 
    pi[0] = 0;
    i = 1;
 
    while(i < m) {
		if(p[i] == p[len]) {
			len++;
			pi[i] = len;
			i++;
		}
		else {
			if( len != 0 )
				len = pi[len-1];
			else {
				pi[i] = 0;
				i++;
			}
	    }
	}
}