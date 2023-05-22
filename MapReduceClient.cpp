#include "MapReduceClient.h"
#include "MapReduceFramework.h"

#define MAPPING_TIMES 10

virtual void map(const K1* key, const V1* value, void* context) const = 0
{
  for(int i = 0; i < MAPPING_TIMES; ++i)
  {

  }
}

virtual void reduce(const IntermediateVec* pairs, void* context) const = 0
{

}