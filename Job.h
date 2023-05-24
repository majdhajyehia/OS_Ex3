//
// Created by majd.hajyehia on 5/22/23.
//

#ifndef RESOURCES_JOB_H
#define RESOURCES_JOB_H
#include "MapReduceFramework.h"
#include <pthread.h>
#include <vector>
#include <atomic>

typedef struct {
    std::atomic<int>* atomic_counter;
    int* bad_counter;
} ThreadContext;

typedef std::vector<std::pair<pthread_t*, ThreadContext>> threads_collection;



class Job{
private:
    JobState _state;
    threads_collection _threads;
public:
  Job(threads_collection threads, JobState state); // Constructor
  const JobState get_state();
  void set_state(JobState state);

};

#endif //RESOURCES_JOB_H
