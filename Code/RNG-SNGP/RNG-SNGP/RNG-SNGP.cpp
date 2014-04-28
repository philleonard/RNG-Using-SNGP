#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <ctime>
#include <conio.h>
#include <string.h>

#define getch            _getch
#define itoa            _itoa

#define NUM_TERMINALS 5
#define NUM_FUNCTIONS 5
#define POP_SIZE 100
#define MAX_ATTEMPTS 25000
#define MAX_SMUT 301
#define FIT_TARG 2700 //27.99
#define J 16384 //1538
#define H_MAX 7
#define RUNS 1

#define MAX_OCC pow(2, (double)H_MAX) + 1 //129 //64
#define MAX_IND_FITNESS 27.999
#define random_function() ((char) (rand() % NUM_FUNCTIONS + NUM_TERMINALS) + 1)

void generate_init_pop(), calc_fitness(int), successor_mutation(), calc_term_output(int), get_tree(int), print_tree(char*), get_binary_sub_seq(char*, int, int), get_total_entropy(int), init_pred(), dynamic_calc(int), create_update_list(int), init_update_list(), print_progress(char*, int, int, int);
int fittest_program(), get_next_update(), get_tree_size(int);
double get_fitness_sum();

long protected_div(long, long), protected_mod(long, long);

void compute_prefix_function(char *p, int m, int *pi), get_tree_as_char(char *), out_fittest(int), get_time(), write_run_header(), create_size_list(int);
int KMP_matching(char *t, char *p);

char current_tree[POP_SIZE * POP_SIZE];
char tree_as_char[POP_SIZE * POP_SIZE];
char file_name[100];
char run_file_name[100];
int update_size = 0;
int update_list[POP_SIZE];
int update_pos;

char file_time[50];

int char_pos = 0;
int current_j = 1;

struct token_entry {
   char *name;
   int arity;
};

struct token_entry token_table [] = {{"#", -1}, {"0", -1}, {"1", -1}, {"2", -1}, {"3", -1}, {"J", -1}, {"+", 1}, {"-", 1}, {"*", 1}, {"%", 1}, {"/", 1} };

struct population_member {
	char node_value;
	int depth;
	//The output vector needs to be the actuall numbers 
	long dec_output[J];
	char bin_output[J + 1];
	//Predecessors and successors refer to indexes in the population array.
	int pred[POP_SIZE]; 
	//Terminal nodes will not have the following, but if arity == -1 then no need to check
	int suc_1; 
	int suc_2;
	double scalar_entropy[H_MAX];
	double total_fitness;
};

struct population_member graph[POP_SIZE];
struct population_member graph_old[POP_SIZE];

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
	get_time();

	sprintf(file_name, "SNGP-log-%s.txt", file_time);
	
	printf("Writing This Run Data to: %s\n", file_name);
	printf("Writing Binary Data to: fittest.rand.out.%s\n\n", file_name);

	return 1;
}

void get_time() {
	time_t rawtime;
	time (&rawtime);
	struct tm  *timeinfo = localtime (&rawtime);
	strftime(file_time, sizeof(file_time)-1, "%d.%m.%y_%H.%M.%S\0", timeinfo);
}

int write_data(int smut_no, double fitness_sum, double average, int fittest, bool solved, float total_time) {

	FILE *fp;
	fp = fopen(file_name, "a");

	struct population_member *curr_node = &graph[fittest];

	char_pos = 0;
	get_tree(fittest);
	current_tree[char_pos] = '\0';
	char_pos = 0;

	char *solved_char = solved ? "TRUE" : "FALSE";
	char line[1024 + sizeof(tree_as_char)];
	char scalar[100];

	int length = 0;
	for (int i = 0; i < H_MAX; i++) {
		length += sprintf(scalar + length, "%f, ", curr_node->scalar_entropy[i]);
	}

	scalar[length + 1] = '\0';

	get_tree_as_char(current_tree);
	int size = get_tree_size(fittest);
	sprintf(line, "%d, %f, %f, %f, %f, %s%s, %d, %s\n", smut_no, total_time, fitness_sum, average, curr_node->total_fitness, scalar, solved_char, size, tree_as_char);
	fprintf(fp, "%s", line);
	fclose(fp);
	return 1;
}

int write_run_data(int run_number, float total_time, bool solved, int fittest) {
	
	struct population_member *curr_node = &graph[fittest];

	FILE *fp;
	fp = fopen(run_file_name, "a");

	get_tree_as_char(current_tree);

	char *solved_char = solved ? "TRUE" : "FALSE";
	char line[1024 + POP_SIZE * POP_SIZE];

	int size = get_tree_size(fittest);
	sprintf(line, "%d, %f, %s, %f, %d\n", run_number, total_time, solved_char, curr_node->total_fitness, size);
	fprintf(fp, "%s", line);
	fclose(fp);
	return 1;
}

int get_tree_size(int fittest);
int size_list[POP_SIZE];

int get_tree_size(int fittest){
	
	for (int i = 0; i < POP_SIZE; i++) {
		size_list[i] = 0;
	}

	create_size_list(fittest);
	int size = 0;

	for (int x = 0; x < POP_SIZE; x++) {
		if (size_list[x] == 1)
			size++;
	}

	return size;
}

void create_size_list(int node) {
	size_list[node] = 1;

	struct population_member *curr_node = &graph[node];

	if (curr_node->suc_1 != -1)
		create_size_list(curr_node->suc_1);
	if (curr_node->suc_2 != -1)
		create_size_list(curr_node->suc_2);

}


int main() {
	srand(time(NULL));
	int test_count = 0;
	get_time();
	sprintf(run_file_name, "RUN-log-%s.txt", file_time);
	write_run_header();
	printf("Writing All Run Data to: %s\n", run_file_name);
	do {
	test_count++;
	clock_t s_time = clock();

	create_file_name();
	write_header();
	
	double total_pop_fitness = 0;
	double total_pop_fitness_old = 0;

	double best_fitness = 0;
	double best_fitness_old = 0;
	init_pred();
	generate_init_pop();

	int smut_number = 0;

	int fittest = fittest_program();
	total_pop_fitness = get_fitness_sum();
	struct population_member *curr_node = &graph[fittest];
	best_fitness = curr_node->total_fitness;

	char_pos = 0;
	get_tree(fittest);
	current_tree[char_pos] = '\0';
	char_pos = 0;

	clock_t mut_s_time = clock();

	int attempt_number = 0;

	do {

		attempt_number++;

		fittest = fittest_program();
		struct population_member *curr_node = &graph[fittest];
		total_pop_fitness = get_fitness_sum();
		best_fitness = curr_node->total_fitness;

		if (best_fitness_old >= best_fitness && total_pop_fitness_old >= total_pop_fitness) { 
			for (int i = 0; i < POP_SIZE; i++) {
				graph[i] = graph_old[i];
			}
			total_pop_fitness = total_pop_fitness_old;
			best_fitness = best_fitness_old;
		}
		else {
			printf("\nPopulation fitness sum = %f", total_pop_fitness);

			smut_number++;

			int fittest = fittest_program();
			total_pop_fitness = get_fitness_sum();
			struct population_member *curr_node = &graph[fittest];
			printf("\nEntropy of fittest RNG = %f bits of entropy\n", curr_node->total_fitness);

			char_pos = 0;
			get_tree(fittest);
			current_tree[char_pos] = '\0';
			char_pos = 0;

			printf("\n\nSMUT COUNT %d\nATTEMPT SMUT %d\n\n", smut_number, attempt_number);
			
		}

		if (total_pop_fitness >= FIT_TARG || best_fitness >= MAX_IND_FITNESS || attempt_number > MAX_ATTEMPTS) { 
			bool solved = false;

			if (total_pop_fitness >= FIT_TARG) {
				solved = true;
				printf("\n\nTERMINATED ON TOTAL FITNESS TARGET REACHED\n");
			}
			if (best_fitness >= MAX_IND_FITNESS) {
				solved = true;
				printf("\n\nTERMINATED ON INDIVIDUAL FITNESS TARGET REACHED\n");
			}
			if (attempt_number >= MAX_ATTEMPTS) {
				printf("\n\nTERMINATED ON MAX ATTEMPT SMUT OPERATIONS\n");
			}
			clock_t mut_f_time = clock();
			double time = ((double)mut_f_time - (double)s_time) / CLOCKS_PER_SEC;

			write_data(smut_number, total_pop_fitness, (double) total_pop_fitness/POP_SIZE, fittest, solved, time);

			write_run_data(test_count, time, solved, fittest);
			//out_fittest(fittest);

			break;
		}
		else if (best_fitness_old < best_fitness || total_pop_fitness_old < total_pop_fitness){ 
			clock_t mut_f_time = clock();
			double time = ((double)mut_f_time - (double)s_time) / CLOCKS_PER_SEC;
			write_data(smut_number, total_pop_fitness, (double) total_pop_fitness/POP_SIZE, fittest, false, time);
			mut_s_time = clock();
		}

		//Mutate and Evaluate

		best_fitness_old = best_fitness;
		total_pop_fitness_old = total_pop_fitness;

		successor_mutation();

		int update_prog = 0;
		char* text = "Evaluating: ";
		for (int x = 0; x < POP_SIZE; x++) {
			int next = get_next_update();
			if (next != -1) {
				dynamic_calc(next);
				calc_fitness(next);
				update_prog++;	
				print_progress(text, update_prog, 50, update_size);
			}
			else
				break;
		}

		for (int i = 0; i < POP_SIZE; i++) {
			get_total_entropy(i);
		}

	} while(true);

	clock_t f_time = clock();
	double time = ((double)f_time - (double)s_time) / CLOCKS_PER_SEC;
	printf("\n\nExecution took %f seconds\n\n", time);
	
	} while (test_count <= RUNS);
	getch();
	return 0;
}

void out_fittest(int fittest) {
	struct population_member *progp = &graph[fittest];
	FILE *fp;
	char file_name_rand[100];
	sprintf(file_name_rand, "%s%s", "fittest.rand.out.", file_name);
	fp = fopen(file_name_rand, "a");

	for (int x = 0; x < J; x++) {
		if (x%8 == 0)
			fprintf(fp, ",%c", progp->bin_output[x]);
		else
			fprintf(fp, "%c", progp->bin_output[x]);
	}
	fprintf(fp, "\n");
	fclose(fp);
}

int get_next_update() {
	int lowest_depth = INT_MAX;
	int next_node = -1;

	for (int i = 0; i < POP_SIZE; i++) {
		if (update_list[i] == 1) {
			struct population_member *curr_node = &graph[i];
			if (curr_node->depth < lowest_depth){
				next_node = i;
				lowest_depth = curr_node->depth;
			}
		}
	}

	if (next_node != -1)
		update_list[next_node] = -1;

	return next_node;
}

double get_fitness_sum() {
	double sum = 0;
	for (int i = 0; i < POP_SIZE; i++) {
		struct population_member *curr_node = &graph[i];
		sum += curr_node->total_fitness;
	}

	return sum;
}

void init_pred() {
	for (int i = 0; i < POP_SIZE; i++) {
		struct population_member *curr_node = &graph[i];
		for (int x = 0; x < POP_SIZE; x++) {
			curr_node->pred[x] = 0;
		}
	}
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

void generate_init_pop() {
	int depth;
	for (int x = 0; x < POP_SIZE; x++) {
		struct population_member *curr_node = &graph[x];
		char token;
		if (x < NUM_TERMINALS) {
			//Adding all terminals exactly once
			token = (char) x + 1;	
			curr_node->depth = 0;
			curr_node->suc_1 = -1;
			curr_node->suc_2 = -1;
		}
		else {
			int f = random_function();
			
			token = (char) f;
			int suc_l;
			struct population_member *successor_l;
			do {
				suc_l = curr_node->suc_1 = rand() % x;
				successor_l = &graph[suc_l];
				curr_node->depth = successor_l->depth + 1;
			} while(successor_l->depth >= curr_node->depth);

			successor_l->pred[x] = 1;
			int suc_r;
			struct population_member *successor_r;
			do {
				suc_r = rand() % x; 
				successor_r = &graph[suc_r];
			} while(successor_r->depth >= curr_node->depth);

			curr_node->suc_2 = suc_r;
			successor_r->pred[x] = 1;

		}

		curr_node->node_value = token;

		char_pos = 0;
		get_tree(x);
		current_tree[char_pos] = '\0';
		char_pos = 0;

		if (x < NUM_TERMINALS)
			calc_term_output(x);
		else {
			dynamic_calc(x);
		}

		calc_fitness(x);

		get_total_entropy(x);
		char* text = "Generating Init Pop: ";
		print_progress(text, x + 1, 50, POP_SIZE);
	}
	printf("\n");
}

void dynamic_calc(int index) {

	struct population_member *curr_node = &graph[index];
	struct population_member *node_left = &graph[curr_node->suc_1];
	struct population_member *node_right = &graph[curr_node->suc_2];
	char token = *token_table[curr_node->node_value].name; 

	switch(token) {
		case '+': 
			for (int i = 0; i < J; i++) {
				curr_node->dec_output[i] = node_left->dec_output[i] + node_right->dec_output[i];
				if ((curr_node->dec_output[i] % 2) == 0)
					curr_node->bin_output[i] = '0';
				else
					curr_node->bin_output[i] = '1';
			}
			break;
		case '-': 
			for (int i = 0; i < J; i++) {
				curr_node->dec_output[i] = node_left->dec_output[i] - node_right->dec_output[i];
				if ((curr_node->dec_output[i] % 2) == 0) 
					curr_node->bin_output[i] = '0';
				else
					curr_node->bin_output[i] = '1';    

			}
			break;
		case '*': 
			for (int i = 0; i < J; i++) {
				curr_node->dec_output[i] = node_left->dec_output[i] * node_right->dec_output[i];
				if ((curr_node->dec_output[i] % 2) == 0) 
					curr_node->bin_output[i] = '0';
				else
					curr_node->bin_output[i] = '1';  
			}
			break;
		case '%': 
			for (int i = 0; i < J; i++) {
				curr_node->dec_output[i] = protected_mod(node_right->dec_output[i], node_left->dec_output[i]);
				if ((curr_node->dec_output[i] % 2) == 0) 
					curr_node->bin_output[i] = '0';
				else
					curr_node->bin_output[i] = '1';        
			}
			break;
		case '/': 
			for (int i = 0; i < J; i++) {
				curr_node->dec_output[i] = protected_div(node_right->dec_output[i], node_left->dec_output[i]);
				if ((curr_node->dec_output[i] % 2) == 0) 
					curr_node->bin_output[i] = '0';
				else
					curr_node->bin_output[i] = '1';  
			}
			break;
	}
	curr_node->bin_output[J + 1] = '\0';
}

void successor_mutation() {
	for (int x = 0; x < POP_SIZE; x++)
		graph_old[x] = graph[x];
	
	int mut = (rand() % (POP_SIZE - NUM_TERMINALS)) + NUM_TERMINALS;
	int suc = rand() % 2;

	struct population_member *mut_node = &graph[mut];

	if (suc == 0) {
		//Update old
		int current_suc_1 = mut_node->suc_1; 
		struct population_member *old_suc = &graph[current_suc_1];
		old_suc->pred[mut] = 0;

		int mut_to;

		struct population_member *mut_to_node;
		do {
			mut_to = rand() % mut;
			mut_to_node = &graph[mut_to];
		} while (mut_to == current_suc_1 && mut_to_node->depth >= mut_node->depth); //Mutating to lower depth

		mut_node->suc_1 = mut_to;
		mut_to_node->pred[mut] = 1;
	}

	else {
		//Update old
		int current_suc_2 = mut_node->suc_2; 
		struct population_member *old_suc = &graph[current_suc_2];
		old_suc->pred[mut] = 0;

		int mut_to;

		struct population_member *mut_to_node;
		do {
			mut_to = rand() % mut;
			mut_to_node = &graph[mut_to];
		} while (mut_to == current_suc_2 && mut_to_node->depth >= mut_node->depth);
		
		mut_node->suc_2 = mut_to;
		mut_to_node->pred[mut] = 1;
	}

	struct population_member *left = &graph[mut_node->suc_1];
	struct population_member *right = &graph[mut_node->suc_2];

	if (left->depth >= right->depth)
		mut_node->depth = left->depth + 1;
	else
		mut_node->depth = right->depth + 1;

	init_update_list();
	update_list[mut] = 1;
	update_size = 1;
	create_update_list(mut);
}

void init_update_list() {
	for (int x = 0; x < POP_SIZE; x++) {
		update_list[x] = -1;
	}
}

void create_update_list(int update_node) {
	struct population_member *curr_update = &graph[update_node];

	for (int i = 0; i < POP_SIZE; i++) {
		if (curr_update->pred[i] == 1) {
			if (update_list[i] != 1) {
				update_list[i] = 1;
				update_size++;
			}
			create_update_list(i);
		}
	}
}

long protected_div(long dom, long num) {
	if (dom == 0) {
		return (0);
	}
	else {
		return ((long) num/(long)dom);
	}
}

long protected_mod(long dom, long num) {
	if (dom == 0) {
		return (1);
	}
	else
		return ((long)num%(long)dom);
}

void get_tree(int index) {

	struct population_member *curr_node = &graph[index];
	current_tree[char_pos] = curr_node->node_value;
	char_pos++;
	if (token_table[curr_node->node_value].arity == 1) {
		get_tree(curr_node->suc_1);
		get_tree(curr_node->suc_2);
	}
}

void print_tree(char *code) {
		char token;
		while (token = *code++) {
				fputs(token_table[token].name, stdout);
		}
}

void get_tree_as_char(char *code) {
		char token;
		int x = 0;
		while (token = *code++) {
			tree_as_char[x] = *token_table[token].name;
			x++;
		}
		tree_as_char[x] = '\0';
}

void calc_term_output(int index) {
	//Method only called on terminals
	struct population_member *curr_node = &graph[index];
	for (int x = 0; x < J; x++) {
		current_j = x + 1;
		char_pos = 0;
		int output_for_j;
		if (*token_table[curr_node->node_value].name == 'J') {
			output_for_j = current_j;
		}
		else if (*token_table[curr_node->node_value].name == '0') {
			output_for_j = 0;
		}
		else {
			output_for_j = *token_table[curr_node->node_value].name - '0'; 
		}
		curr_node->dec_output[x] = output_for_j;

		int lsb = output_for_j % 2;
		if (lsb == 0)
			curr_node->bin_output[x] = '0';
		else
			curr_node->bin_output[x] = '1';
	}
	curr_node->bin_output[J + 1] = '\0';
}

void calc_fitness(int index) {
	struct population_member *curr_node = &graph[index];
	char *bit_seq = curr_node->bin_output; 

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
		curr_node->scalar_entropy[k] = e_scalar[k];
	}
}

void get_total_entropy(int index) {
	struct population_member *curr_node = &graph[index];
	double e_total = 0;
	curr_node->total_fitness = 0;
	for(int s = 0; s < H_MAX; s++){
		curr_node->total_fitness = curr_node->total_fitness + curr_node->scalar_entropy[s];
	}
}

int fittest_program() {
	int fittest_index = 0;
	int max_fitness = -1;
	for (int d = 0; d < POP_SIZE; d++) {
		struct population_member *curr_node = &graph[d];
		if (curr_node->total_fitness > max_fitness) {
			fittest_index = d;
			max_fitness = curr_node->total_fitness;
		}
	}

	return fittest_index;
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