#include <assert.h> // assert
#include <math.h>   // q, ceil
#include <stdio.h>  // printf
#include <stdlib.h> // malloc

#include "cms.h"
#include "contrib/murmurhash2.h"

#define min(a, b)                                                                                  \
    ({                                                                                             \
        __typeof__(a) _a = (a);                                                                    \
        __typeof__(b) _b = (b);                                                                    \
        _a < _b ? _a : _b;                                                                         \
    })

#define BIT64 64
#define CMS_HASH(item, itemlen, i) MurmurHash2(item, itemlen, i)

CMSketch *NewCMSketch(size_t width, size_t depth, uint64_t max) {
    assert(width > 0);
    assert(depth > 0);

    CMSketch *cms = CMS_CALLOC(1, sizeof(CMSketch));

    cms->width = width;
    cms->depth = depth;
    cms->counter = 0;
    if (max <= 0xff) {
        cms->byte = 1;
    } else if (max <= 0xffff) {
        cms->byte = 2;
    } else if (max <= 0xffffffff) {
        cms->byte = 4;
    } else {
        cms->byte = 8;
    }
    cms->array = CMS_CALLOC(width * depth, cms->byte);

    return cms;
}

void CMS_DimFromProb(double error, double delta, size_t *width, size_t *depth) {
    assert(error > 0 && error < 1);
    assert(delta > 0 && delta < 1);

    *width = ceil(2 / error);
    *depth = ceil(log10f(delta) / log10f(0.5));
}

void CMS_Destroy(CMSketch *cms) {
    assert(cms);

    CMS_FREE(cms->array);
    cms->array = NULL;

    CMS_FREE(cms);
}

size_t CMS_IncrBy(CMSketch *cms, const char *item, size_t itemlen, size_t value) {
    assert(cms);
    assert(item);

    size_t minCount = (size_t)-1;

    for (size_t i = 0; i < cms->depth; ++i) {
        uint32_t hash = CMS_HASH(item, itemlen, i);
        CMS_INCR(cms, (hash % cms->width) + (i * cms->width), value);
        minCount = min(minCount, CMS_GET(cms, (hash % cms->width) + (i * cms->width)));
    }
    cms->counter += value;
    return minCount;
}

size_t CMS_Query(CMSketch *cms, const char *item, size_t itemlen) {
    assert(cms);
    assert(item);

    size_t minCount = (size_t)-1;

    for (size_t i = 0; i < cms->depth; ++i) {
        uint32_t hash = CMS_HASH(item, itemlen, i);
        minCount = min(minCount, CMS_GET(cms, (hash % cms->width) + (i * cms->width)));
    }
    return minCount;
}

void CMS_Merge(CMSketch *dest, size_t quantity, const CMSketch **src, const long long *weights) {
    assert(dest);
    assert(src);
    assert(weights);

    size_t itemCount = 0;
    size_t cmsCount = 0;
    size_t width = dest->width;
    size_t depth = dest->depth;

    for (size_t i = 0; i < depth; ++i) {
        for (size_t j = 0; j < width; ++j) {
            itemCount = 0;
            for (size_t k = 0; k < quantity; ++k) {
                itemCount += CMS_GET(src[k], (i * width) + j);
            }
            CMS_SET(dest, (i * width) + j, itemCount);
        }
    }

    for (size_t i = 0; i < quantity; ++i) {
        cmsCount += src[i]->counter * weights[i];
    }
    dest->counter = cmsCount;
}

void CMS_INCR(CMSketch *cms, u_int64_t index, uint64_t delta) {
    if (cms->byte == 1) {
        u_int8_t *force = (u_int8_t *)cms->array;
        force[index] += delta;
    } else if (cms->byte == 2) {
        u_int16_t *force = (u_int16_t *)cms->array;
        force[index] += delta;
    } else if (cms->byte == 4) {
        u_int32_t *force = (u_int32_t *)cms->array;
        force[index] += delta;
    } else {
        u_int64_t *force = (u_int64_t *)cms->array;
        force[index] += delta;
    }
}

void CMS_SET(CMSketch *cms, u_int64_t index, uint64_t num) {
    if (cms->byte == 1) {
        u_int8_t *force = (u_int8_t *)cms->array;
        force[index] = num;
    } else if (cms->byte == 2) {
        u_int16_t *force = (u_int16_t *)cms->array;
        force[index] = num;
    } else if (cms->byte == 4) {
        u_int32_t *force = (u_int32_t *)cms->array;
        force[index] = num;
    } else {
        u_int64_t *force = (u_int64_t *)cms->array;
        force[index] = num;
    }
}

u_int64_t CMS_GET(const CMSketch *cms, u_int64_t index) {
    if (cms->byte == 1) {
        u_int8_t *force = (u_int8_t *)cms->array;
        return force[index];
    } else if (cms->byte == 2) {
        u_int16_t *force = (u_int16_t *)cms->array;
        return force[index];
    } else if (cms->byte == 4) {
        u_int32_t *force = (u_int32_t *)cms->array;
        return force[index];
    } else {
        u_int64_t *force = (u_int64_t *)cms->array;
        return force[index];
    }
}

void CMS_MergeParams(mergeParams params) {
    CMS_Merge(params.dest, params.numKeys, (const CMSketch **)params.cmsArray,
              (const long long *)params.weights);
}

/************ used for debugging *******************
void CMS_Print(const CMSketch *cms) {
    assert(cms);

    for (int i = 0; i < cms->depth; ++i) {
        for (int j = 0; j < cms->width; ++j) {
            printf("%d\t", cms->array[(i * cms->width) + j]);
        }
        printf("\n");
    }
    printf("\tCounter is %lu\n", cms->counter);
} */