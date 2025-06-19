
#include <colors.h>
#include <time.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

colors_t colors_full(const size_t size)
{
  if (size == 0)
    return 0;

  if(size > MAX_SIZE)
    return MAX_COLORS;

  return MAX_COLORS >> (MAX_SIZE - size);
}

colors_t colors_empty (void)
{
  return 0;
}

colors_t colors_set(const size_t color_id)
{
  if(color_id > MAX_SIZE)
    return colors_empty();

  return (colors_empty() + 1) << color_id;

}

colors_t colors_add(const colors_t colors, const size_t color_id)
{
  if(color_id > MAX_SIZE)
    return colors;

  return colors | colors_set(color_id);
}

colors_t colors_discard(const colors_t colors, const size_t color_id)
{
  return colors & (~colors_set(color_id));
}

bool colors_is_in(const colors_t colors, const size_t color_id)
{
  if(color_id > MAX_SIZE)
    return false;

  return (colors_set(color_id) & colors) != 0;
}

colors_t colors_negate(const colors_t colors)
{
  return ~colors;
}

colors_t colors_and(const colors_t colors1, const colors_t colors2)
{
  return colors1 & colors2;
}

colors_t colors_or(const colors_t colors1, const colors_t colors2)
{
  return colors1 | colors2;
}

colors_t colors_xor(const colors_t colors1, const colors_t colors2)
{
  return colors1 ^ colors2;
}

colors_t colors_subtract(const colors_t colors1, const colors_t colors2)
{
  return colors1 & ~colors2;
}

bool colors_is_equal(const colors_t colors1, const colors_t colors2)
{
  return colors1 == colors2;
}

bool colors_is_subset(const colors_t colors1, const colors_t colors2)
{
  return (colors1 & colors2) == colors1;
}

bool colors_is_singleton(const colors_t colors)
{
  return colors && !(colors & (colors - 1));
}

colors_t colors_rightmost(const colors_t colors)
{
  if (colors == 0)
    return colors;

  return colors & ~(colors - 1);
}

size_t colors_count(const colors_t colors)
{
  colors_t i = colors;
  i = i - ((i >> 1) & 0x5555555555555555);
  i = (i & 0x3333333333333333) + ((i >> 2) & 0x3333333333333333);
  return (((i + (i >> 4)) & 0x0F0F0F0F0F0F0F0F) * 0x0101010101010101) >> 56;
}

colors_t colors_leftmost(const colors_t colors)
{
  colors_t  n = colors;
  size_t r = 0;

  if (colors == 0)
    return 0;

  while (n >>= 1)
    r = r + 1;

  return colors_set(r);
}

static void prng_init(time_t seed)
{
  static bool seed_initialized = false;

  if (!seed_initialized)
  {
    srandom(seed);
    seed_initialized = true;
  }
}

colors_t colors_random(const colors_t colors)
{
  colors_t random_color = colors;
  prng_init(time(NULL) * getpid());
  int nb_colors = colors_count(colors);
  long r = random();

  if (!nb_colors)
    return 0;

  r = r % nb_colors;
  for (int i = 0; i < r; i = i + 1)
    random_color = random_color ^ colors_rightmost(random_color);
  
  random_color = colors_rightmost(random_color);
  
  return random_color;
}

bool subgrid_consistency(colors_t **subgrid, const size_t size)
{
  colors_t singleton_pool = colors_empty();
  colors_t multicolor_pool = colors_empty();

  for (size_t i = 0; i < size; i = i + 1)
  {
    if (!*subgrid[i])
      return false;

    if (colors_is_singleton(*subgrid[i]))
    {
      if (colors_and(singleton_pool, *subgrid[i]))
        return false;

      singleton_pool = colors_or(singleton_pool, *subgrid[i]);
    }
    else
      multicolor_pool = colors_or(multicolor_pool, *subgrid[i]);

  }
  
  multicolor_pool = colors_or(multicolor_pool, singleton_pool);
  
  return colors_is_equal(multicolor_pool, colors_full(size));
   
}

static bool cross_hatching(colors_t **subgrid, const size_t size)
{
  colors_t singleton_pool = colors_empty();
  colors_t control = colors_empty();
  bool alteration = false;

  if (!subgrid)
    return false;

  for (size_t i = 0; i < size; i = i + 1)
    if (colors_is_singleton(*subgrid[i]))
      singleton_pool = colors_or(singleton_pool, *subgrid[i]);

  for (size_t i = 0; i < size; i = i + 1)
  {
    control = *subgrid[i];
    if (!colors_is_singleton(*subgrid[i]))
      *subgrid[i] = colors_subtract(*subgrid[i], singleton_pool);

    alteration = alteration || !colors_is_equal(*subgrid[i], control);
  }

  return alteration;
}

static bool lone_number(colors_t **subgrid, const size_t size)
{
  colors_t suspect = colors_empty();
  size_t occ = 0;
  colors_t control = colors_empty();
  bool alteration = false;

  if (!subgrid)
    return false;

  for (size_t i = 0; i < size; i = i + 1)
  {
    occ = 0;
    suspect = colors_set(i);
    for (size_t j = 0; j < size; j = j + 1)
      if (colors_and(*subgrid[j], suspect))
        occ = occ + 1;

    if (occ == 1)
      for (size_t j = 0; j < size; j = j + 1)
      {
        control = *subgrid[i];
        if (colors_and(*subgrid[j], suspect))
          *subgrid[j] = colors_and(*subgrid[j], suspect);

        alteration = alteration || !colors_is_equal(*subgrid[i], control);
      }
  }

  return alteration;
}

static bool naked_subset(colors_t **subgrid, const size_t size)
{
  colors_t suspect = colors_empty();
  size_t suspect_size = 0;
  size_t occ = 0;
  colors_t control = colors_empty();
  bool alteration = false;

  if (!subgrid)
    return false;

  for (size_t i = 0; i < size; i = i + 1)
  {
    occ = 0;
    suspect = *subgrid[i] ;
    suspect_size = colors_count(suspect);
    for (size_t j = 0; j < size; j = j + 1)
      if (colors_is_equal(suspect, *subgrid[j]))
        occ = occ + 1;

    if (occ == suspect_size)
      for (size_t j = 0; j < size; j = j + 1)
      {
        control = *subgrid[i];
        if (!colors_is_equal(suspect, *subgrid[j]))
          *subgrid[j] = colors_subtract(*subgrid[j], suspect);

        alteration = alteration || !colors_is_equal(*subgrid[i], control);
      }
  }

  return alteration;
}

static bool hidden_subset(colors_t **subgrid, const size_t size)
{
  colors_t suspect = colors_empty();
  size_t suspect_size = 0;
  size_t occ = 0;
  colors_t control = colors_empty();
  bool alteration = false;

  if (!subgrid)
    return false;

  for (size_t i = 0; i < size; i = i + 1)
  {
    occ = 0;
    suspect = *subgrid[i] ;
    suspect_size = colors_count(suspect);
    for (size_t j = 0; j < size; j = j + 1)
      if (colors_and(suspect, *subgrid[j]))
        occ = occ + 1;

    if (occ == suspect_size)
      for (size_t j = 0; j < size; j = j + 1)
      {
        control = *subgrid[i];
        if (colors_and(suspect, *subgrid[j]))
          *subgrid[j] = colors_and(*subgrid[j], suspect);

        alteration = alteration || !colors_is_equal(*subgrid[i], control);
      }
  }

  return alteration;
}

bool subgrid_heuristics(colors_t **subgrid, const size_t size)
{
  bool alteration = false;

  if (!subgrid)
    return false;

  alteration = alteration || cross_hatching(subgrid, size);
  alteration = alteration || lone_number(subgrid, size);
  alteration = alteration || naked_subset(subgrid, size);
  alteration = alteration || hidden_subset(subgrid, size);
  
  return alteration;
}
