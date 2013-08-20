/*
 * Harry - A Tool for Measuring String Similarity
 * Copyright (C) 2013 Konrad Rieck (konrad@mlsec.org)
 * --
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.  This program is distributed without any
 * warranty. See the GNU General Public License for more details. 
 */
#include "config.h"
#include "common.h"
#include "harry.h"
#include "util.h"

#include "kern_wdegree.h"

/**
 * @addtogroup measures
 * <hr>
 * <em>kern_wdegree</em>: Weighted-degree kernel
 * @{
 */

/* External variables */
extern config_t cfg;

/* Local variables */
int degree = 3;         /**< Degree of kernel */
int shift = 0;          /**< Shift of kernel */

/**
 * Initializes the similarity measure
 */
void kern_wdegree_config()
{
    config_lookup_int(&cfg, "measures.kern_wdegree.degree", &degree);
    config_lookup_int(&cfg, "measures.kern_wdegree.shift", &shift);
}

/**
 * Weighting function for matching blocks. 
 * Sonnenburg, Raetsch, Schoelkopf: Large scale genomic sequence SVM
 * classifiers.  ICML 2005: 848-855
 * @param len length of block
 * @param degree of kernel
 * @return weighting
 */
static float weight(float len, int degree)
{
    assert(len > 0 && degree > 0);

    if (len <= degree) {
        float w = len * (-len * len + 3 * degree * len + 3 * degree + 1);
        return w / (3 * degree * (degree + 1));
    } else {
        return (3 * len - degree + 1) / 3;
    }
}

/**
 * Implementation of weighted-degree kernel in block mode.
 * @param x Symbols of string x
 * @param y Symbols of string y
 * @param len Length of region to match
 * @return kernel value
 */
static float kern_wdegree(sym_t *x, sym_t *y, int len)
{
    int i, start;
    float k = 0;

    for (i = 0, start = -1; i < len; i++) {
        /* Identify matching region */
        if (x[i] == y[i]) {
            if (start == -1)
                start = i;
            continue;
        }

        /* No match found continue */
        if (start == -1)
            continue;

        k += weight(i - start, degree);
        start = -1;
    }

    if (start != -1)
        k += weight(i - start, degree);

    return k;
}

/**
 * Compute the weighted-degree kernel with shift. If the strings have
 * unequal size, the remaining symbols of the longer string are ignored (in
 * accordance with the kernel definition)
 * @param x first string 
 * @param y second string
 * @return weighted-degree kernel
 */
float kern_wdegree_compare(str_t x, str_t y)
{
    float k = 0;
    int s, len;

    /* Loop over shifts */
    for (s = -shift; s <= shift; s++) {
        if (s <= 0) {
            len = fmax(fmin(x.len, y.len + s), 0);
            k += kern_wdegree(x.str.s, y.str.s - s, len);
        } else {
            len = fmax(fmin(x.len - s, y.len), 0);
            k += kern_wdegree(x.str.s + s, y.str.s, len);
        }
    }

    return k;
}

/** @} */
