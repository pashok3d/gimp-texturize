/* graph.h */

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "block.h"

#define NODE_BLOCK_SIZE 512
#define ARC_BLOCK_SIZE 1024
#define NODEPTR_BLOCK_SIZE 128

class Graph {
public:
  typedef enum {
    SOURCE	= 0,
    SINK	= 1
  } termtype; /* terminals */

  typedef short captype;
  /* Type of total flow */
  typedef int flowtype;
  typedef void * node_id;

  Graph(void (*err_function)(const char *) = NULL);

  ~Graph();

  node_id add_node();

  void add_edge(node_id from, node_id to, captype cap, captype rev_cap);

  void set_tweights(node_id i, captype cap_source, captype cap_sink);

  void add_tweights(node_id i, captype cap_source, captype cap_sink);

  termtype what_segment(node_id i);

  flowtype maxflow();

private:
  struct arc_st;

  /* node structure */
  typedef struct node_st {
    arc_st			*first;		/* first outcoming arc */

    arc_st			*parent;	/* node's parent */
    node_st			*next;		/* pointer to the next active node
									   (or to itself if it is the last node in the list) */
    int				TS;			/* timestamp showing when DIST was computed */
    int				DIST;		/* distance to the terminal */
    short			is_sink;	/* flag showing whether the node is in the source or in the sink tree */

    captype			tr_cap;		/* if tr_cap > 0 then tr_cap is residual capacity of the arc SOURCE->node
									   otherwise         -tr_cap is residual capacity of the arc node->SINK */
  } node;

  /* arc structure */
  typedef struct arc_st {
    node_st			*head;		/* node the arc points to */
    arc_st			*next;		/* next arc with the same originating node */
    arc_st			*sister;	/* reverse arc */

    captype			r_cap;		/* residual capacity */
  } arc;

  /* 'pointer to node' structure */
  typedef struct nodeptr_st {
    node_st			*ptr;
    nodeptr_st		*next;
  } nodeptr;

  Block<node>			*node_block;
  Block<arc>			*arc_block;
  DBlock<nodeptr>		*nodeptr_block;

  void	(*error_function)(const char *);/* this function is called if a error occurs,
                                           with a corresponding error message
                                           (or exit(1) is called if it's NULL) */

  flowtype			flow;		/* total flow */

  node				*queue_first[2], *queue_last[2];	/* list of active nodes */
  nodeptr			*orphan_first, *orphan_last;		/* list of pointers to orphans */
  int				TIME;					/* monotonically increasing global counter */

  void set_active(node *i);
  node *next_active();

  void maxflow_init();
  void augment(arc *middle_arc);
  void process_source_orphan(node *i);
  void process_sink_orphan(node *i);
};

#endif
