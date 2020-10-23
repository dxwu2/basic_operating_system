/* filesystem.h - Definitions for various structures and variables used in our filesystem */

#include "types.h"

/* macros */


/* index node struct */
typedef struct index_node {
    uint32_t length_in_B;

} index_node_t;

/* the filesystem */

/* local functions */
void init_file_system(void);