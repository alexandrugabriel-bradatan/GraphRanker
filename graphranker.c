#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;

typedef struct {
	uint   index;
	long   sum;
	uint **mat;
} graph;

typedef struct {
	uint     size;
	graph **heap;
} ranking;

enum cmd {ADD, TOPK, END};

static uint N_NODES;
static uint N_RANK;
static ranking *RANKING = NULL;

static inline uint read_num(void);
static inline int start_main_loop(void);
static inline enum cmd eval_cmd(void);
static inline int add(void);
static inline int topk(void);

static graph *graph_create(uint index);
static void graph_destroy(graph *g);

static inline ranking *ranking_create(void);

/* Read one ASCII number from stdin and convert it into a uint */
uint read_num(void) {
	char c;
	char num_buf[11];
	u_int8_t i;

	for (i = 0;
			(c = fgetc(stdin)) != ',' && c != '\n' && c != ' ' && c != EOF; i++)
		num_buf[i] = c;
	num_buf[i] = '\0';

	uint ret = 0;
	for (i = 0; num_buf[i] != '\0'; i++)
		ret = 10 * ret + (num_buf[i] - '0');
	return ret;
}

/*
 * Read a line from stdin and translate the command into a cmd enum.
 * Hypothesis: user input is always correct!
 */
enum cmd eval_cmd(void) {
	char cmd[15];
	if (fgets(cmd, 15, stdin))
		return cmd[0] == 'A' ? ADD : TOPK;
	else
		return END;
}

/*
 * Create ranking object
 * Note: since we will be having only one ranking, there is no need to destroy
 * it
 */
inline ranking *ranking_create(void) {
	ranking *r = malloc(sizeof(ranking));
	r->size = 0;
	r->heap = malloc(N_RANK * sizeof(graph*));
	return r;
}

/* Create graph object */
graph *graph_create(uint index) {
	uint i, j;
	graph *g = malloc(sizeof(graph));

	g->mat = malloc(N_NODES * sizeof(unsigned int*));
	for (i = 0; i < N_NODES; i++)
		g->mat[i] = malloc(N_NODES * sizeof(unsigned int));

	for (i = 0; i < N_NODES; i++)
		for (j = 0; j < N_NODES; j++)
			g->mat[i][j] = read_num();

	g->index = index;

	return g;
}

/* Destroy graph object */
void graph_destroy(graph *g) {
	for (int i = 0; i < N_NODES; i++)
		free(g->mat[i]);
	free(g->mat);
	free(g);
}

/* AggiungiGrafo command */
int add(void) {
	/* TODO  */
	return 0;
}

/* TopK command */
int topk(void) {
	if (RANKING == NULL)
		return 0;
	uint i;
	for (i = 0; i < N_RANK - 1; i++)
		printf("%d ", RANKING->heap[i]->index);
	printf("%d", RANKING->heap[i]->index);
	return 0;
}

/* Execute repl */
inline int start_main_loop(void) {
	enum cmd cur_cmd;
	int out;

	while(!feof(stdin)) {
		cur_cmd = eval_cmd();
		switch(cur_cmd) {
			case ADD:
				out = add();
				break;
			case TOPK:
				out = topk();
				break;
			case END:
				break;
		}
		if (out != 0) return out;
	}
	return 0;
}

int main(void) {
	N_NODES = read_num();
	N_RANK = read_num();
	return start_main_loop();
}
