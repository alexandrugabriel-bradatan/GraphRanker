#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

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
static graph *GRAPH = NULL;

static inline uint read_num(void);
static inline int start_main_loop(void);
static inline enum cmd eval_cmd(void);
static inline int add(void);
static inline int topk(void);

static inline void graph_create(void);
static void graph_parse(uint index); // io-bound
static uint graph_next_neighbour(uint node, uint *w); // static vars
static inline ulong graph_sum(void);
static inline void graph_rank(void);

static inline void ranking_create(void);
static inline void ranking_swap(uint i, uint j);
static void ranking_max_heapify(uint i); // recursive
static inline void ranking_bubble_up(uint elem);
static inline void ranking_insert(ulong key, uint val);
static inline void ranking_pop(void);
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

/* "Bubble up" the ranking's heap element in position elem */
inline void ranking_bubble_up(uint elem) {
	for (uint i = elem;
			i > 0 && RANKING->heap[heap_parent(i)].key < RANKING->heap[i].key;
			i = heap_parent(i)) {
		ranking_swap(i, heap_parent(i));
	}
}

/* Pop node with the largest key */
inline void ranking_pop(void) {
		ranking_swap(0, RANKING->len - 1);
		RANKING->len--;
		ranking_max_heapify(0);
}

/*
 * Insert one item into the ranking if it allows it, otherwise pop the node
 * with the largest key if the given key is smaller than it
 */
inline void ranking_insert(ulong key, uint val) {
	if (RANKING->len < N_RANK) {
		RANKING->len++;
		RANKING->heap[RANKING->len - 1].key = key;
		RANKING->heap[RANKING->len - 1].val = val;
		ranking_bubble_up(RANKING->len - 1);
	} else if (key < RANKING->heap[0].key) {
		RANKING->len++;
		RANKING->heap[RANKING->len - 1].key = key;
		RANKING->heap[RANKING->len - 1].val = val;
		ranking_bubble_up(RANKING->len - 1);
		ranking_pop();
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

	if (PQUEUE->len > N_NODES)
		abort();
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

/* Crate new graph */
inline void graph_create(void) {
	GRAPH = malloc(sizeof(graph));
}

/* Parse new graph from stdin */
void graph_parse(uint index) {
	GRAPH->index = index;
	GRAPH->score = 0;
	GRAPH->nodes = malloc(N_NODES * sizeof(node));
	for (uint i = 0; i < N_NODES; i++) {
		GRAPH->nodes[i].edges = malloc(N_NODES * sizeof(uint));
		for (uint j = 0; j < N_NODES; j++)
			GRAPH->nodes[i].edges[j] = read_num();
	}
}

/*
 * Works in a similar maner to strtok. For every iteration the next neighbour's
 * index and the corresponding edge weight are returned. If there are no more
 * neighbours, N_NODES is returned.
 */
uint graph_next_neighbour(uint node, uint *w) {
	static uint cur_node = 0;
	static uint old_neighbour = 0;

	if (node != cur_node) {
		cur_node = node;
		old_neighbour = 0;
	}

	while (GRAPH->nodes[cur_node].edges[old_neighbour] == 0 &&
			old_neighbour < N_NODES)
		old_neighbour++;

	if (old_neighbour < N_NODES)
		*w = GRAPH->nodes[cur_node].edges[old_neighbour];

	return old_neighbour++;
}

inline ulong graph_sum(void) {
	ulong r = 0;
	for (uint i = 0; i < N_NODES; i++) {
		if (GRAPH->nodes[i].dist != ULONG_MAX)
			r += GRAPH->nodes[i].dist;
	}
	return r;
}

inline void graph_rank(void) {
	uint min_node, neighbour, neighbour_weight;
	ulong min_p, new_p;

	pqueue_clear();

	GRAPH->nodes[0].dist = 0;
	pqueue_enqueue(0, 0);
	for (uint i = 1; i < N_NODES; i++) {
		pqueue_enqueue(i, ULONG_MAX);
		GRAPH->nodes[i].dist = ULONG_MAX;
	}

	while (!pqueue_empty()) {
		min_node = pqueue_unqueue();
		min_p = GRAPH->nodes[min_node].dist;
		if (min_p == ULONG_MAX)
			break;
		for (neighbour = graph_next_neighbour(min_node, &neighbour_weight);
				neighbour < N_NODES;
				neighbour = graph_next_neighbour(min_node, &neighbour_weight)) {
			new_p = min_p + neighbour_weight;
			if (GRAPH->nodes[neighbour].dist > new_p) {
				GRAPH->nodes[neighbour].dist = new_p;
				pqueue_decrease_priority(neighbour, new_p);
			}
		}
	}

	GRAPH->score = graph_sum();
}

/* AggiungiGrafo command */
int add(void) {
	static uint cur_index = 0;

	graph_parse(cur_index);
	graph_rank();
	ranking_insert(GRAPH->score, GRAPH->index);
	cur_index++;
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
	graph_create();
	pqueue_create();
	return start_main_loop();
}
