//
// Created by majd.hajyehia on 5/22/23.
//

#ifndef RESOURCES_JOB_H
#define RESOURCES_JOB_H
#include "MapReduceFramework.h"
#include <pthread.h>
#include <vector>
#include <atomic>
#include <functional>
#include "Barrier.h"

typedef struct {
    std::atomic<int>* input_elements;
    std::atomic<int>* shuffle_atomic;
    JobHandle job_handle;
    int thread_id;
    unsigned long unique_k2_keys_count;
} ThreadContext;

typedef std::pair<pthread_t*, ThreadContext> thread_pair;
typedef std::vector<thread_pair> threads_collection;
typedef std::vector<K2*> intermediate_unique_k2_vector;


class Job{
private:
    JobState _state;
    threads_collection _threads;
    InputVec _input_elements;
    OutputVec _output_elements;
    IntermediateVec *_intermediate_vectors;
    const MapReduceClient &_client;
    int _threads_count;
    intermediate_unique_k2_vector* _unique_k2_keys;

public:
  Job(threads_collection threads, JobState state, const MapReduceClient
  &client);
  // Constructor
  Job(JobState state, InputVec input_vec, OutputVec
  output_vec, const MapReduceClient &client, int threads_count);
  const JobState get_state();
  void set_state(JobState state);
  void set_threads(threads_collection threads);
  void append_thread(thread_pair pair);
  const InputVec get_inputs_elements();
  const OutputVec get_output_elements();
  const MapReduceClient& get_client();
  const float get_percentage();
  const stage_t get_stage();
  void set_percentage(float percent);
  void set_stage(stage_t stage);
  const int get_threads_count();
  IntermediateVec* get_intermediate_vectors();
  Barrier barrier;
  intermediate_unique_k2_vector* get_unique_k2_keys();
  std::vector<IntermediateVec> shuffeld_vec;
  pthread_mutex_t shuffeld_vector_mutex;
  pthread_mutex_t output_vector_mutex;

  void set_intermediate_vectors(IntermediateVec *intermediate_vectors);
};

#endif //RESOURCES_JOB_H
