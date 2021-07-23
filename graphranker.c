#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define heap_parent(i) ((i - 1) / 2)

typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct {
	ulong dist;
	uint *edges;
} node;

typedef struct {
	uint   index;
	ulong  score;
	node  *nodes;
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

typedef struct {
	uint  key;
	ulong p;
} pqueue_node;

/*
 * The priority queue is implemented as a min-heap. Each element has a unique
 * key that corresponds to the index of the element's position in the min-hep.
 * The keys' values are stored inside 'tokens', an array index by said keys. We
 * will use the node indexes as keys, so the queue can hold up to N_NODES
 * elements and is optimized for this.
 */
typedef struct {
	uint  		 len;
	uint  		*tokens;
	pqueue_node *heap;
} pqueue;

enum cmd {ADD, TOPK, END};

static uint N_NODES;
static uint N_RANK;
static ranking *RANKING = NULL;
static pqueue *PQUEUE = NULL;

static inline uint read_num(void);
static inline int start_main_loop(void);
static inline enum cmd eval_cmd(void);
static inline int add(void);
static inline int topk(void);

static graph *graph_create(uint index); // io-bound
static inline void graph_destroy(graph *g);

static inline void ranking_create(void);
static inline void ranking_insert(ulong key, uint val);
static inline void ranking_swap(uint i, uint j);
static void ranking_max_heapify(uint i); // recursive
static inline void ranking_walk(void);

static inline void pqueue_create(void);
static inline bool pqueue_empty(void);
static inline void pqueue_enqueue(uint key, ulong p);
static void pqueue_min_heapify(uint i); // recursive
static inline void pqueue_swap(uint i, uint j);
static inline void pqueue_bubble_up(uint elem);
static inline uint pqueue_unqueue(void);
static inline void pqueue_decrease_priority(uint key, ulong new_p);
static inline void pqueue_clear(void);

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

/* Create the priority queue */
inline void pqueue_create(void) {
	PQUEUE = calloc(1, sizeof(pqueue));
	PQUEUE->tokens = malloc(N_NODES * sizeof(uint));
	PQUEUE->heap = malloc(N_NODES * sizeof(pqueue_node));
}

/* Swap the queue's i-th and j-th heap elements */
inline void pqueue_swap(uint i, uint j) {
	pqueue_node tmp;
	uint tmp_tok;

	tmp_tok = PQUEUE->tokens[PQUEUE->heap[i].key];
	PQUEUE->tokens[PQUEUE->heap[i].key] = PQUEUE->tokens[PQUEUE->heap[j].key];
	PQUEUE->tokens[PQUEUE->heap[j].key] = tmp_tok;

	tmp = PQUEUE->heap[i];
	PQUEUE->heap[i] = PQUEUE->heap[j];
	PQUEUE->heap[j] = tmp;
}

/* "Bubble up" the queue's heap element in position elem */
inline void pqueue_bubble_up(uint elem) {
	for (uint i = elem;
			i > 0 && PQUEUE->heap[heap_parent(i)].p > PQUEUE->heap[i].p;
			i = heap_parent(i))
		pqueue_swap(i, heap_parent(i));
}

/* Add node with key 'key' to the queue with priority p */
inline void pqueue_enqueue(uint key, ulong p) {
	PQUEUE->heap[PQUEUE->len].p = p;
	PQUEUE->heap[PQUEUE->len].key = key;
	PQUEUE->tokens[key] = PQUEUE->len;
	PQUEUE->len++;

	pqueue_bubble_up(PQUEUE->len - 1);
}

/* Equivalent to ranking_max_heapify, only with min-heaps */
void pqueue_min_heapify(uint i) {
	uint pos_min, l, r;

	l = 2 * i;
	r = 2 * i + 1;
	if (l < PQUEUE->len && PQUEUE->heap[l].p < PQUEUE->heap[i].p)
		pos_min = l;
	else
		pos_min = i;
	if (r < PQUEUE->len && PQUEUE->heap[r].p < PQUEUE->heap[i].p)
		pos_min = r;
	if (pos_min != i) {
		pqueue_swap(i, pos_min);
		pqueue_min_heapify(pos_min);
	}
}

/* Pop node with the smallest priority */
inline uint pqueue_unqueue(void) {
	uint old_key = PQUEUE->heap[0].key;

	pqueue_swap(0, PQUEUE->len - 1);
	PQUEUE->len--;
	pqueue_min_heapify(0);

	return old_key;
}

/* Decrease the priority of the node corresponding to key to p */
inline void pqueue_decrease_priority(uint key, ulong new_p) {
	PQUEUE->heap[PQUEUE->tokens[key]].p = new_p;
	pqueue_bubble_up(PQUEUE->tokens[key]);
}

inline bool pqueue_empty(void) {
	return PQUEUE->len == 0;
}

/* Reset queue without freeing memory */
inline void pqueue_clear(void) {
	PQUEUE->len = 0;
}

/* Create graph object */
graph *graph_create(uint index) {
	uint i, j;
	graph *g = malloc(sizeof(graph));

	g->score = 0;
	g->nodes = malloc(N_NODES * sizeof(node));
	for (uint i = 0; i < N_NODES; i++) {
		g->nodes[i].edges = malloc(N_NODES * sizeof(uint));
		for (uint j = 0; j < N_NODES; j++)
			g->nodes[i].edges[j] = read_num();
	}
	g->index = index;

	return g;
}

/* Destroy graph object */
inline void graph_destroy(graph *g) {
	for (uint i = 0; i < N_NODES; i++) {
		free(g->nodes[i].edges);
	}
	free(g->nodes);
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
	pqueue_create();
	return start_main_loop();
}
