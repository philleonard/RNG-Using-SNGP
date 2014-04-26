#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctime>
#include <conio.h>
#include <string.h>

#define getch _getch
#define itoa _itoa

#define NUM_TERMINALS 2
#define NUM_FUNCTIONS 5
#define NUM_CONSTANTS 4
#define NUM_R_TERMINALS 4
#define MAX_PROG_SIZE 1000
#define FPR 0.1
#define RUNS 10
#define INTERN_NODE_PROB 0.8
#define POP_SIZE 500
#define NUM_FPR_OPS (int) floor((POP_SIZE * FPR)/2)
#define NUM_CROSS_OPS (int) ceil((POP_SIZE / 1 - FPR)/2) - NUM_FPR_OPS
#define MAX_GEN 51
#define FIT_TARG 27.99
#define J 16384 
#define H_MAX 7
#define MAX_OCC pow(2, (double)H_MAX) + 1 
#define MAX_INIT_DEPTH 6
#define MAX_CROSS_DEPTH 15
#define PARTITION_SIZE   (POP_SIZE/(MAX_INIT_DEPTH - 1))

void generate_initial_pop(), print_tree(char*), evaluate_fitness(int), calc_rand_bit_seq(int), print_progress(int, int, int, int), get_tree(char *), evolution(), crossover(int, int, int, int, int, int), out_fittest();
int fittest_program(), write_data(int, double, int, bool, float), fpr_selection(), subtree_end(char[], int);

long protected_div(long, long), protected_mod(long, long), calculate_tree_output();
void compute_prefix_function(char *p, int m, int *pi), write_header(), write_run_header();
int KMP_matching (char *t, char *p);

char *koza_fittest = "-J/+++JJJ*+J2J+%*-21//+*JJ/-/*J%/J3%JJ/*32/21-3/+*JJ-213*3+%10J33+-2J1+/%J3-%20%%0JJ-33\0";
double fitness_sum;
int fittest = 0;

struct token_entry {
   char *name;
   int arity;
};

struct token_entry token_table [] = {{"#", -1}, {"J", -1}, {"R", -1}, {"+", 1}, {"-", 1}, {"*", 1}, {"%", 1}, {"/", 1}, {"0", -1}, {"1", -1}, {"2", -1}, {"3", -1} };

#define random_function() ((char) (rand() % NUM_FUNCTIONS) + 1)
#define random_terminal() ((char) (rand() % NUM_TERMINALS) + 1)
#define random_token() ((char) (rand() % NUM_TERMINALS + NUM_FUNCTIONS) + 1) 
#define random_r() ((char) (rand() % NUM_R_TERMINALS) + 8) //This way, the probability of R being chosen is still 1/2 and not 3/4 (given J and R are the possible terminals).

char *dummy = "%*J33\0";
int current_j = 1;
char file_name[100];
char tree_as_char[MAX_PROG_SIZE + 1];
char run_file_name[100];
float tree_calc_time;
float mod_time;
char *prog_code;
char file_time[50];
double old_fittest = 0;
double old_average_fitness = 0;

struct population_member {
	char code[MAX_PROG_SIZE + 1];
	char output[J + 1];
	int proglen;
	double scalar_entropy[H_MAX];
	double total_fitness;
};

struct population_member population[POP_SIZE];
struct population_member population_new[POP_SIZE];
struct population_member population_old[POP_SIZE];

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

int write_run_data(int run_number, float total_time, bool solved, int fittest) {
	
	struct population_member *progp = &population[fittest];

	FILE *fp;
	fp = fopen(run_file_name, "a");

	char *solved_char = solved ? "TRUE" : "FALSE";
	char line[1024 + POP_SIZE * POP_SIZE];
	sprintf(line, "%d, %f, %s, %f, %d\n", run_number, total_time, solved_char, progp->total_fitness, strlen(tree_as_char));
	fprintf(fp, "%s", line);
	fclose(fp);
	return 1;
}

void write_header() {
	FILE *fp;
	fp = fopen(file_name, "a");

	char line[1024] = "Smut Count, Time (sec), Fitness Sum, Average Fitness, Fittest Fitness, ";

	for (int i = 0; i < H_MAX; i++) {
		char this_scalar[100];
		sprintf(this_scalar, "Scalar Entropy (%d bit), ", i + 1); 
		strcat(line, this_scalar);
	}
	strcat(line, "Solution?, Fittest RNG Tree\n");
	fprintf(fp, "%s", line);
	fclose(fp);
}

void write_run_header() {
	FILE *fp;
	fp = fopen(run_file_name, "a");

	char line[] = "Run No., Time (sec), Solution?, Fittest Fitness, Solution Size";

	fprintf(fp, "%s\n", line);
	fclose(fp);
}

int create_file_name() {
	char file_time[50];
	time_t rawtime;
	time (&rawtime);
	struct tm  *timeinfo = localtime (&rawtime);
	strftime(file_time, sizeof(file_time)-1, "%d.%m.%y_%H.%M.%S\0", timeinfo);

	sprintf(file_name, "GP-log-%s.txt", file_time);
	printf("Writing Run Data to: %s\n\n", file_name);

	return 1;
}

double get_average_fitness(){
	double acc_fit = 0;
	for (int y = 0; y <= POP_SIZE; y++) {
		struct population_member *progp = &population[y];
		acc_fit = acc_fit + progp->total_fitness;
	}
	fitness_sum = acc_fit;
	return acc_fit/(double) POP_SIZE;
}

void get_time() {
	time_t rawtime;
	time (&rawtime);
	struct tm  *timeinfo = localtime (&rawtime);
	strftime(file_time, sizeof(file_time)-1, "%d.%m.%y_%H.%M.%S\0", timeinfo);
}

int main() {
	int test_count = 0;
	get_time();
	sprintf(run_file_name, "RUN-log-%s.txt", file_time);
	write_run_header();
	printf("Writing All Run Data to: %s\n", run_file_name);
	//Seeding Random Number Generator.
	srand(time(NULL));

	do {
		test_count++;
		clock_t s_time = clock();

		create_file_name();
		write_header();
		int generation_count = 0;

		clock_t gen_stime = clock();
		generate_initial_pop();
		clock_t gen_ftime = clock();
		float gen_time = (((float)gen_ftime - (float)gen_stime) / 1000000.0F ) * 1000;
		printf("\n\n");
		bool solved = false;

		while (true) {
			printf("GENERATION %d\n\n", generation_count);
			clock_t total_stime = clock(); 

			char* text = "Calculating  output: ";

			for (int x = 0; x <= POP_SIZE; x++) {
				calc_rand_bit_seq(x);
				if(x%5 == 0)
					print_progress(text, x, 50, POP_SIZE);
			}
			printf("\n");
			text = "Calculating fitness: ";

			clock_t fit_stime = clock();
			for (int y = 0; y <= POP_SIZE; y++) {
				evaluate_fitness(y);
				if(y%5 == 0)
					print_progress(text, y, 50, POP_SIZE);
			}

			fittest = fittest_program();
			struct population_member *progp = &population[fittest];
	
			double average_fitness = get_average_fitness();
			double fittest_fitness = progp->total_fitness;
			printf("\nEntropy of fittest RNG = %f bits\n", progp->total_fitness);
			printf("Average fitness = %f bits\n", average_fitness);

			clock_t total_ftime = clock();
			float total_time = (((float)total_ftime - (float)s_time) / 1000000.0F ) * 1000;
		
			if (progp->total_fitness >= FIT_TARG)
				solved = true;

			if (old_fittest < progp->total_fitness) 
				write_data(generation_count, average_fitness, fittest, solved, total_time);

			//out_fittest(); //This method can be used to write the binary output of the fittest candidate to a file

			//Termination Criteria
			if (progp->total_fitness >= FIT_TARG)
				break;
			if (generation_count >= MAX_GEN)
				break;
	
			//HILL CLIMBING TEST

			//Time for selection & crossover
			evolution();

			generation_count++;
		}

		printf("\nFINISHED!");
		clock_t f_time = clock();
		double time = ((double)f_time - (double)s_time) / CLOCKS_PER_SEC;
		printf("\n\nExecution took %f seconds\n\n", time);

		write_run_data(test_count, time, solved, fittest);
		getch();
	} while (test_count <= RUNS);
	
	return 0;
}

void out_fittest() {
	struct population_member *progp = &population[fittest];
	FILE *fp;
	char file_name_rand[100];
	sprintf(file_name_rand, "%s%s", "fittest.rand.out.", file_name);
	fp = fopen(file_name_rand, "a");

	for (int x = 0; x < J; x++) {
		if (x%8 == 0)
			fprintf(fp, ",%c", progp->output[x]);
		else
			fprintf(fp, "%c", progp->output[x]);
	}
	fprintf(fp, "\n");
	fclose(fp);
}

int fpr_selection() {
	double total_select = fitness_sum; //Adding POP_SIZE so that programs with 0 entropy still have chance
	double selection = ( (double)rand() * total_select ) / (double)RAND_MAX;
	int s = 0;
	double acc_fitness = 0;
	double old_acc_fitness = 0;
	for (s = 0; s < POP_SIZE; s++) {
		struct population_member *progp = &population[s];
		acc_fitness = acc_fitness + progp->total_fitness;
		if (acc_fitness >= selection && selection >= old_acc_fitness) {
			break;
			//s is the selection for parent 1
		}
	}
	return s;
}

int node_selection(int parent) {
	double node_type = ( (double)rand() * 1 ) / (double)RAND_MAX;
	
	int crossover_point = 0;

	struct population_member *progp = &population[parent];
	

	if (node_type <= INTERN_NODE_PROB && progp->proglen != 1) {
		char token;
		do{
			crossover_point = rand()%progp->proglen;
			token = progp->code[crossover_point];
		} while(token_table[token].arity != 1);
	}

	else {
		char token;
		do{
			crossover_point = rand()%progp->proglen;
			token = progp->code[crossover_point];
		} while(token_table[token].arity != -1);
	}

	return crossover_point;
}

void crossover(int parent_a, int parent_b, int a_cross_point, int a_subtree_end, int b_cross_point, int b_subtree_end, int a_new_index, int b_new_index) {
	struct population_member *prog_parent_a = &population[parent_a];
	struct population_member *prog_parent_b = &population[parent_b];
	struct population_member *prog_child_a = &population_new[a_new_index];
	struct population_member *prog_child_b = &population_new[b_new_index];

	char T_a_new[MAX_PROG_SIZE + 1];
	char T_b_new[MAX_PROG_SIZE + 1];

	int T_a_new_count = 0;
	//Create child a
	for (int x = 0; x < a_cross_point; x++) {
		T_a_new[T_a_new_count] = prog_parent_a->code[x];
		T_a_new_count++;
	}

	for (int y = b_cross_point; y < b_subtree_end + 1; y++) {
		T_a_new[T_a_new_count] = prog_parent_b->code[y];
		T_a_new_count++;
	}

	for (int z = a_subtree_end + 1; z < prog_parent_a->proglen; z++) {
		T_a_new[T_a_new_count] = prog_parent_a->code[z];
		T_a_new_count++;
	}
	T_a_new[T_a_new_count] = '\0';

	int T_b_new_count = 0;
	//Create child b
	for (int x = 0; x < b_cross_point; x++) {
		T_b_new[T_b_new_count] = prog_parent_b->code[x];
		T_b_new_count++;
	}

	for (int y = a_cross_point; y < a_subtree_end + 1; y++) {
		T_b_new[T_b_new_count] = prog_parent_a->code[y];
		T_b_new_count++;
	}

	for (int z = b_subtree_end + 1; z < prog_parent_b->proglen; z++) {
		T_b_new[T_b_new_count] = prog_parent_b->code[z];
		T_b_new_count++;
	}
	T_b_new[T_b_new_count] = '\0';

	prog_child_a->proglen = strlen(T_a_new);
	prog_child_b->proglen = strlen(T_b_new);

	int k = 0;
	for (int k = 0; k <= prog_child_a->proglen; k++) {
		prog_child_a->code[k] = T_a_new[k];
	}

	int l = 0;
	for (int l = 0; l <= prog_child_b->proglen; l++) {
		prog_child_b->code[l] = T_b_new[l];
	}
}

void evolution() {

	int x = 0;
	int new_index = -1;
	for (x = 0; x < NUM_FPR_OPS; x++) {
		//SELECT TWO PARENTS BASED ON FITNESS
		int parent_a = 0;
		int parent_b = 0;
		int potential_size_child_a = 0;
		int potential_size_child_b = 0;
		int a_cross_point = 0;
		int b_cross_point = 0;
		int a_subtree_end = 0;
		int b_subtree_end = 0;
		int a_new_index = 0;
		int b_new_index = 0;
		do{
			do{
				parent_a = fpr_selection();
				parent_b = fpr_selection();
			} while(parent_a == parent_b);

			a_cross_point = node_selection(parent_a);
			b_cross_point = node_selection(parent_b);

			struct population_member *progp_a = &population[parent_a];
			 a_subtree_end = subtree_end(progp_a->code, a_cross_point);
			struct population_member *progp_b = &population[parent_b];
			b_subtree_end = subtree_end(progp_b->code, b_cross_point);
		
			if ((progp_a->proglen + (b_subtree_end - b_cross_point)) - (a_subtree_end - a_cross_point) > progp_a->proglen)
				potential_size_child_a = (progp_a->proglen + (b_subtree_end - b_cross_point)) - (a_subtree_end - a_cross_point);
			else
				potential_size_child_a = progp_a->proglen;

			if ((progp_b->proglen + (a_subtree_end - a_cross_point)) - (b_subtree_end - b_cross_point) > progp_b->proglen)
				potential_size_child_b = (progp_b->proglen + (a_subtree_end - a_cross_point)) - (b_subtree_end - b_cross_point);
			else
				potential_size_child_b = progp_b->proglen;

			a_new_index = new_index++;
			b_new_index = new_index++;
		} while(potential_size_child_a > MAX_PROG_SIZE || potential_size_child_b > MAX_PROG_SIZE);
		
		crossover(parent_a, parent_b, a_cross_point, a_subtree_end, b_cross_point, b_subtree_end, a_new_index, b_new_index);
		
		print_progress("Evolving population: ", x, 50, POP_SIZE/2);
	}

	for (int j = 0; j < NUM_CROSS_OPS; j++) {
		//SELECT ONE PARENT BASED ON FITNESS
		//SELECT ANOTHER EQUALLY
		int parent_a = 0;
		int parent_b = 0;
		int potential_size_child_a = 0;
		int potential_size_child_b = 0;
		int a_cross_point = 0;
		int b_cross_point = 0;
		int a_subtree_end = 0;
		int b_subtree_end = 0;
		int a_new_index = 0;
		int b_new_index = 0;
		
		do{
			do {
				parent_a = fpr_selection();
				parent_b = rand()%POP_SIZE;
			} while(parent_a == parent_b);

			a_cross_point = node_selection(parent_a);
			b_cross_point = node_selection(parent_b);

			struct population_member *progp_a = &population[parent_a];
			 a_subtree_end = subtree_end(progp_a->code, a_cross_point);
			struct population_member *progp_b = &population[parent_b];
			b_subtree_end = subtree_end(progp_b->code, b_cross_point);
		
			if ((progp_a->proglen + (b_subtree_end - b_cross_point)) - (a_subtree_end - a_cross_point) > progp_a->proglen)
				potential_size_child_a = (progp_a->proglen + (b_subtree_end - b_cross_point)) - (a_subtree_end - a_cross_point);
			else
				potential_size_child_a = progp_a->proglen;

			if ((progp_b->proglen + (a_subtree_end - a_cross_point)) - (b_subtree_end - b_cross_point) > progp_b->proglen)
				potential_size_child_b = (progp_b->proglen + (a_subtree_end - a_cross_point)) - (b_subtree_end - b_cross_point);
			else
				potential_size_child_b = progp_b->proglen;

			a_new_index = new_index++;
			b_new_index = new_index++;
		} while(potential_size_child_a > MAX_PROG_SIZE || potential_size_child_b > MAX_PROG_SIZE);
		
		crossover(parent_a, parent_b, a_cross_point, a_subtree_end, b_cross_point, b_subtree_end, a_new_index, b_new_index);
		
		print_progress("Evolving population: ", j + x + 1, 50, POP_SIZE/2);
	}

	printf("\n\n");
	for (int x = 0; x < POP_SIZE; x++) {
		population[x] = population_new[x];
	}
	
}

int subtree_end(char tree[], int index)
{  int limbs = 0;
   
   while ((limbs += token_table[tree[index]].arity) >= 0) {
	   
	   index++;
   }
   return (index);
}

int write_data(int gen_no, double average, int fittest, bool solved, float total_time) {

	FILE *fp;
	struct population_member *progp = &population[fittest];
	fp = fopen(file_name, "a");

	char *solved_char = solved ? "TRUE" : "FALSE";
	get_tree(progp->code);
	char line[1024 + MAX_PROG_SIZE];
	char scalar[1024];

	int length = 0;
	for (int i = 0; i < H_MAX; i++) {
		length += sprintf(scalar + length, "%f, ", progp->scalar_entropy[i]);
	}

	scalar[length + 1] = '\0';

	sprintf(line, "%d, %f, %f, %f, %s%s, %s\n", gen_no, total_time, average, progp->total_fitness, scalar, solved_char, tree_as_char);
	fprintf(fp, "%s", line);
	fclose(fp);
	return 1;
}

void get_tree(char *code) {
	char token;
	int i = 0;
	while (token = *code++) {
			char t = *token_table[token].name;
			tree_as_char[i] = t;
			i++;
	}
	tree_as_char[i] = '\0';
}

int fittest_program() {
	int fittest_index = 0;
	int max_fitness = -1;
	for (int d = 0; d < POP_SIZE; d++) {
		struct population_member *progp = &population[d];
		if (progp->total_fitness > max_fitness) {
			fittest_index = d;
			max_fitness = progp->total_fitness;
		}
	}

	return fittest_index;
}

void calc_rand_bit_seq(int index) {
	struct population_member *progp = &population[index];
	prog_code = NULL;
	prog_code = progp->code;

	for (int x = 0; x < J; x++) {
		prog_code = progp->code;
		current_j = x + 1;
		
		clock_t t1, t2, t3, t4;
		t1 = clock();
		long output_for_j = calculate_tree_output();
		t2 = clock();
		float diff = (((float)t2 - (float)t1) / 1000000.0F ) * 1000;
		tree_calc_time = tree_calc_time + diff;

		t3 = clock();
				int lsb = output_for_j % 2;
		t4 = clock();
		float diff2 = (((float)t4 - (float)t2) / 1000000.0F ) * 1000;
		mod_time = mod_time + diff2;
		
		if (lsb == 0)
			progp->output[x] = '0';
		else
			progp->output[x] = '1';
	}
	progp->output[J + 1] = '\0';
}

long calculate_tree_output() {
	char code_token = *prog_code++;
	char token = *token_table[code_token].name; 
	switch(token) {
		case '+': return(calculate_tree_output() + calculate_tree_output());
		case '-': return(calculate_tree_output() - calculate_tree_output());
		case '*': return(calculate_tree_output() * calculate_tree_output());
		case '%': return(protected_mod(calculate_tree_output(), calculate_tree_output()));
		case '/': return(protected_div(calculate_tree_output(), calculate_tree_output()));
		case 'J': return(current_j);
		default: return(token-'0'); //Must be R = {0, 1, 2, 3} 
	}
}

long protected_div(long dom, long num) {
	if (dom == 0) {
		return (0);
	}
	else {
		return ((long) num/ (long)dom); 
	}
}
long protected_mod(long dom, long num) {
	if (dom == 0) {
		return (1);
	}
	else
		return ((long)num%(long)dom);
}

void get_binary_sub_seq(char* j2, int j, int h) {
	
			char j2_bin[H_MAX + 1];

			for (int x = 0; x < H_MAX + 1; x++) {
				j2_bin[x] = '\0';
			}

			itoa(j, j2_bin, 2);
			
			int y = 0;
			for (y = 0; y < H_MAX + 1; y++) {
				if (j2_bin[y] == '\0')
					break;
			}

			char zeros[H_MAX];
			int f = 0;
			char zero = '0';

			for (f; f < h - y; f++) {
					zeros[f] = zero;
			}
			zeros[f] = '\0';

			sprintf(j2, "%s%s", zeros, j2_bin);

}

void fitness_function(int index) {
	struct population_member *progp = &population[index];
	char *bit_seq = progp->output; 

	double e_scalar[H_MAX];

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
			double prob = 0;
			if (total_occ != 0) 
				prob = occ / (double)total_occ;

			double e_h_increment;
			if (prob == 0 || prob == 1) //Avoiding log(0) and negative zero
				e_h_increment = 0;
			else
				e_h_increment = (-1 *((prob * log10(prob)) / log10(2.0)));
			
			e_scalar[h - 1] = e_scalar[h - 1] + e_h_increment;
		}
		
	}
	for (int k = 0; k < H_MAX; k++) {
		progp->scalar_entropy[k] = e_scalar[k];
	}
}

void get_total_entropy(int index) {
	struct population_member *progp = &population[index];
	double e_total = 0;
	progp->total_fitness = 0;
	for(int s = 0; s < H_MAX; s++){
		progp->total_fitness = progp->total_fitness + progp->scalar_entropy[s];
	}

}

void evaluate_fitness(int index) {
	struct population_member *progp = &population[index];
	char *bin_seq;
	bin_seq = progp -> output;

	fitness_function(index);
	get_total_entropy(index);
	
}

char *create_tree(char *tree, int depth, int full) { 
   char token; int arity_m1, i;
   if (depth == 1) {
      token = random_terminal();
	  if (strcmp(token_table[token].name, "R") == 0) {
		  token = random_r();
	  }
	  *tree++ = token;
   }
   else { 
	  token = (full ? random_function() : random_token());
	  if (strcmp(token_table[token].name, "R") == 0) {
		  token = random_r();
	  }
	  *tree++ = token;
      arity_m1 = token_table[token].arity;
      for (i=0; i <= arity_m1; i++)
         tree = create_tree(tree, depth-1, full);
   }
   return(tree);
}

void generate_initial_pop() { 
  struct population_member *progp = population;
  int i, j, depth, full;
  char *end_tree;

  char* text = "Generating init pop: ";

  for (i = 2; i <= MAX_INIT_DEPTH; i++) {
     for (j = 0; j < PARTITION_SIZE; j++)  {  
		 full = j%2;
		 depth = i;
         end_tree = create_tree(progp->code, depth, full);
         *end_tree = '\0';
         progp->proglen = strlen(progp->code);
         if (depth < MAX_INIT_DEPTH) 
			 depth++;
        progp++;
     }
	 print_progress(text, i, 50, MAX_INIT_DEPTH);
  }
}

void print_tree(char *code) {
		char token;
		while (token = *code++) {
				fputs(token_table[token].name, stdout);
				printf(" ");
		}
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