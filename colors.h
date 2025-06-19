#ifndef COLORS_H
#define COLORS_H

#define MAX_COLORS 0xFFFFFFFFFFFFFFFFULL
#define MAX_SIZE 64

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t colors_t;

/* return a color with '1' on all bits within range of given size */
colors_t colors_full(const size_t size);

/* return a color with 0 on all bits */
colors_t colors_empty (void);

/* return a color with a '1' set to the given index and '0' for the others */
colors_t colors_set(const size_t color_id);

/* return a color with a '1' set to the index of a given color */ 
colors_t colors_add(const colors_t colors, const size_t color_id);

/* return a color with a '0' set to the index of a given color */ 
colors_t colors_discard(const colors_t colors, const size_t color_id);

/* check if the color index is set to '1' in the given color */
bool colors_is_in(const colors_t colors, const size_t color_id);

/* return a bitwise negation the given color */
colors_t colors_negate(const colors_t colors);

/* return the intersection of two given colors */
colors_t colors_and(const colors_t colors1, const colors_t colors2);

/* return the union of two given colors */
colors_t colors_or(const colors_t colors1, const colors_t colors2);

/* return the exclusive union of two given colors */
colors_t colors_xor(const colors_t colors1, const colors_t colors2);

/* return color1 less color2 */
colors_t colors_subtract(const colors_t colors1, const colors_t colors2);

/* check if two colors are equal */
bool colors_is_equal(const colors_t colors1, const colors_t colors2);

/* check if colors1 is included in color2 */
bool colors_is_subset(const colors_t colors1, const colors_t colors2);

/* check if the given color only has one bit set to '1' */
bool colors_is_singleton(const colors_t colors);

/* return a color which is the least significant bit of a given color */
colors_t colors_rightmost(const colors_t colors);

/* return the number of bit set to '1' of a given color */
size_t colors_count(const colors_t colors);

/* return a color which is the most significant bit of a given color */
colors_t colors_leftmost(const colors_t colors);

/* return a singleton taken randomly from the given color */
colors_t colors_random(const colors_t colors);

/* check if the given subgrid is consistent */
bool subgrid_consistency(colors_t **subgrid, const size_t size);

/* check if the heuristics have modified the given subgrid */
bool subgrid_heuristics(colors_t **subgrid, const size_t size);

#endif
