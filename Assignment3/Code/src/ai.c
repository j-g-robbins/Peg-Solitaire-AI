#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"


void copy_state(state_t* dst, state_t* src){
	
	//Copy field
	memcpy( dst->field, src->field, SIZE*SIZE*sizeof(int8_t) );

	dst->cursor = src->cursor;
	dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution( node_t* solution_node ){
	node_t* n = solution_node;
	while( n->parent != NULL ){
		copy_state( &(solution[n->depth]), &(n->state) );
		solution_moves[n->depth-1] = n->move;
		n = n->parent;
	}
	solution_size = solution_node->depth;
}


node_t* create_init_node( state_t* init_state ){
	node_t * new_n = (node_t *) malloc(sizeof(node_t));
	new_n->parent = NULL;	
	new_n->depth = 0;
	copy_state(&(new_n->state), init_state);
	return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t* applyAction(node_t* n, position_s* selected_peg, move_t action ){

    node_t* new_node = NULL;

	//FILL IN MISSING CODE
	new_node = (node_t *) malloc(sizeof(node_t));
	new_node->parent = n;
	new_node->depth = n->depth + 1;
	new_node->move = action;
	copy_state(&(new_node->state), &(n->state));
    execute_move_t( &(new_node->state), &(new_node->state.cursor), action );
	
	return new_node;

}

/** 
 * Safely malloc an array of nodes given a size
*/
node_t** malloc_node_array(int size) {
	node_t** explored = malloc(sizeof(node_t*) * size);
	assert(explored != NULL);
	return explored;
}


/** 
 * Free array of nodes
*/
void free_node_array(node_t** array, int size) {
	for (int i=1; i < size; i++) {
		free(array[i]);
	}
	free(array);
}


/**
 * Find a solution path as per algorithm description in the handout
 */

void find_solution( state_t* init_state  ){
	

	HashTable table;

	// Choose initial capacity of PRIME NUMBER 
	// Specify the size of the keys and values you want to store once 
	ht_setup( &table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

	// Initialize Stack
	initialize_stack();

	//Add the initial node
	node_t* n = create_init_node( init_state );
	
	// Push node to stack	
	stack_push(n);
	int remaining_pegs = num_pegs(&(n->state));
	
	// Initialise explored nodes array
	int size = 10, a = 1;
	node_t** explored = malloc_node_array(size);
	
	// Iterate through stack while not empty
	while (!is_stack_empty()) {
		n = stack_top();
		stack_pop();
		
		// Add node to explored array, resize if necessary
		if (a == size) {
			size *= 2;
			explored = realloc(explored, sizeof(node_t*) * size);
			assert(explored != NULL);
		}
		explored[a++] = n;
		expanded_nodes += 1;
		
		// Update solution if current state is a better solution
		if (num_pegs(&(n->state)) < remaining_pegs) {
			save_solution(n);
			remaining_pegs = num_pegs(&(n->state));
		}
		
		// Iterate all possible moves
		for (int i = 0; i < SIZE; i++) {
			n->state.cursor.x = i;
			for (int j = 0; j < SIZE; j++) {
				n->state.cursor.y = j;
				for (int k = 0; k < 4; k++) {
					
					// Generate child nodes for valid moves
					if (can_apply(&(n->state), &(n->state.cursor), k)) {
						node_t* new_node = applyAction(n, &(n->state.cursor), k);						
						generated_nodes += 1;
						
						// If peg solitaire is solved with new state, update the solution
						if (won(&(new_node->state))) {
							save_solution(new_node);
							remaining_pegs = num_pegs(&(new_node->state));
							
							// Free memory
							free(new_node);
							free_node_array(explored, a);
							ht_destroy(&table);
							free_stack();
							
							return;
						}
						
						// Insert unique nodes into hash table and stack, i.e. Depth-First-Search
						// Strategy, else free duplicate nodes
						if (!ht_contains(&table, &(new_node->state))) {
							ht_insert(&table, &(new_node->state), &(new_node->state));
							stack_push(new_node);
						} else {
							free(new_node);
						}
					}
				}
			}
		}
		
		// Budget has been exhausted, free memory and return
		if (expanded_nodes >= budget) {
			ht_destroy(&table);
			free_node_array(explored, a);
			free_stack();
			
			return;
		}
	}
}