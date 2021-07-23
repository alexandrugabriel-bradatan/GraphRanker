#include <stdio.h>
#include <stdlib.h>

#define heap_parent(i) ((i - 1) / 2)

typedef unsigned int uint;

typedef struct {
	uint   index;
	long   sum;
	uint **mat;
} graph;

typedef struct {
	ulong key;
	uint  val;
} ranking_node;

/* The ranking is implemented using a max-heap with the graph indexes as keys */
typedef struct {
	uint          len;
	ranking_node *heap;
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

static inline void ranking_create(void);
static inline void ranking_insert(ulong key, uint val);
static inline void ranking_swap(uint i, uint j);
static void ranking_max_heapify(uint i); // recursive
static inline void ranking_walk(void);

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

/* Create the ranking object */
inline void ranking_create(void) {
	RANKING = malloc(sizeof(ranking));
	RANKING->len = 0;
	RANKING->heap = malloc((N_RANK + 1) * sizeof(ranking_node));
}

/* Swap ranking's i-th and j-th heap elements */
inline void ranking_swap(uint i, uint j) {
	ranking_node tmp;

	tmp = RANKING->heap[i];
	RANKING->heap[i] = RANKING->heap[j];
	RANKING->heap[j] = tmp;
}

/*
 * Create a max-heap with root at position i assuming the children of the
 * i-th node are both sub-max-heaps.
 */
void ranking_max_heapify(uint i) {
	uint pos_max, l, r;

	l = 2 * i;
	r = 2 * i + 1;
	if (l < RANKING->len && RANKING->heap[l].key > RANKING->heap[i].key)
		pos_max = l;
	else
		pos_max = i;
	if (r < RANKING->len && RANKING->heap[r].key > RANKING->heap[i].key)
		pos_max = r;
	if (pos_max != i) {
		ranking_swap(i, pos_max);
		ranking_max_heapify(pos_max);
	}
}

/*
 * Insert one item into the ranking if it allows it, otherwise pop the node
 * with the largest key
 */
inline void ranking_insert(ulong key, uint val) {
	if (key > RANKING->heap[0].key)
		return;

	RANKING->len++;
	RANKING->heap[RANKING->len - 1].key = key;
	RANKING->heap[RANKING->len - 1].val = val;

	for (uint i = RANKING->len - 1;
			i > 0 && RANKING->heap[heap_parent(i)].key < RANKING->heap[i].key;
			i = heap_parent(i)) {
		ranking_swap(i, heap_parent(i));
	}

	if (RANKING->len > N_RANK) {
		ranking_swap(0, RANKING->len - 1);
		RANKING->len--;
		ranking_max_heapify(0);
	}
}

inline void ranking_walk(void) {
	if (RANKING->len == 0)
		return;

	for (uint i = 0; i < RANKING->len - 1; i++)
		printf("%d ", RANKING->heap[i].val);
	printf("%d", RANKING->heap[RANKING->len - 1].val);
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
inline int topk(void) {
	ranking_walk();
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
	ranking_create();
	return start_main_loop();
}
