#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int            index;
	long           sum;
	unsigned int **mat;
} graph;

typedef struct {
	int     size;
	graph **heap;
} ranking;

enum cmd {ADD, TOPK, END};

static unsigned int N_NODES;
static unsigned int N_RANK;
static ranking *RANKING = NULL;

static inline unsigned int read_num(void);
static inline int start_main_loop(void);
static inline enum cmd eval_cmd(void);
static int add(void);
static int topk(void);

static inline ranking *ranking_create(void);

/* reads one ASCII number from stdin and converts it into a uint */
inline unsigned int read_num(void) {
	char c;
	char num_buf[11];
	u_int8_t i;

	for (i = 0;
			(c = fgetc(stdin)) != ',' && c != '\n' && c != ' ' && c != EOF; i++)
		num_buf[i] = c;
	num_buf[i] = '\0';

	unsigned int ret = 0;
	for (i = 0; num_buf[i] != '\0'; i++)
		ret = 10 * ret + (num_buf[i] - '0');
	return ret;
}

/*
 * Reads a line from stdin and translates the command into a cmd enum.
 * Hypothesis: user input is always correct!
 */
inline enum cmd eval_cmd(void) {
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

/* AggiungiGrafo command */
int add(void) {
	/* TODO  */
	return 0;
}

/* TopK command */
int topk(void) {
	if (RANKING == NULL)
		return 0;
	int i;
	for (i = 0; i < N_RANK - 1; i++)
		printf("%d ", RANKING->heap[i]->index);
	printf("%d", RANKING->heap[i]->index);
	return 0;
}

/* Executes repl */
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
