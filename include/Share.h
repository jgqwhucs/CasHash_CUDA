#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cuda_runtime.h>
#include <string>

const int kDimSiftData = 128; // the number of dimensions of SIFT feature
const int kDimHashData = 128; // the number of dimensions of Hash code
const int kBitInCompHash = 64; // the number of Hash code bits to be compressed; in this case, use a <uint64_t> variable to represent 64 bits
const int kDimCompHashData = kDimHashData / kBitInCompHash; // the number of dimensions of CompHash code
const int kMinMatchListLen = 16; // the minimal list length for outputing SIFT matching result between two images
const int kMaxCntPoint = 1000000; // the maximal number of possible SIFT points; ensure this value is not exceeded in your application

const int kCntBucketBit = 8; // the number of bucket bits
const int kCntBucketGroup = 6; // the number of bucket groups
const int kCntBucketPerGroup = 1 << kCntBucketBit; // the number of buckets in each group

const int kCntCandidateTopMin = 6; // the minimal number of top-ranked candidates
const int kCntCandidateTopMax = 10; // the maximal number of top-ranked candidates

typedef int SiftData_t;
typedef int* SiftDataPtr; // SIFT feature is represented with <int> type
typedef uint8_t* HashDataPtr; // Hash code is represented with <uint8_t> type; only the lowest bit is used
typedef uint64_t* CompHashDataPtr; // CompHash code is represented with <uint64_t> type
typedef int* BucketElePtr; // index list of points in a specific bucket

typedef struct {
    int cntPoint; // the number of SIFT points
    std::string keyFilePath;
    SiftDataPtr siftDataMatrix; // [cntPoint x 128] Matrix, storing all sift vectors one-off

    /* not currently used: */
    HashDataPtr* hashDataPtrList; // Hash code for each SIFT point
    CompHashDataPtr* compHashDataPtrList; // CompHash code for each SIFT point
    uint16_t* bucketIDList[kCntBucketGroup]; // bucket entries for each SIFT point
    int cntEleInBucket[kCntBucketGroup][kCntBucketPerGroup]; // the number of SIFT points in each bucket
    BucketElePtr bucketList[kCntBucketGroup][kCntBucketPerGroup]; // SIFT point index list for all buckets
}   ImageDataHost; // all information needed for an image to perform CasHash-Matching

typedef struct {
    int cntPoint;
    SiftDataPtr siftDataMatrix;
    size_t siftDataMatrixPitch;

    HashDataPtr* hashDataPtrList;
    CompHashDataPtr* compHashDataPtrList;
    uint16_t* bucketIDList[kCntBucketGroup];
    int cntEleInBucket[kCntBucketGroup][kCntBucketPerGroup];
    BucketElePtr bucketList[kCntBucketGroup][kCntBucketPerGroup];
} ImageDataDevice;

#define CUDA_CHECK_ERROR                                                         \
    do {                                                                         \
        const cudaError_t err = cudaGetLastError();                              \
        if (err != cudaSuccess) {                                                \
            const char *const err_str = cudaGetErrorString(err);                 \
            std::cerr << "Cuda error in " << __FILE__ << ":" << __LINE__ - 1     \
                      << ": " << err_str << " (" << err << ")" << std::endl;     \
            exit(EXIT_FAILURE);                                                  \
        }                                                                        \
    } while(0)

template< typename T >
void check(T result, char const *const func, const char *const file, int const line)
{
    if (result)
    {
        fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n",
                file, line, static_cast<unsigned int>(result), cudaGetErrorString(result), func);
        cudaDeviceReset();
        // Make sure we call CUDA Device Reset before exiting
        exit(EXIT_FAILURE);
    }
}
#define CUDA_CATCH_ERROR(val) check ( (val), #val, __FILE__, __LINE__)
