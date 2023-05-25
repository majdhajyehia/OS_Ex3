#include "MapReduceFramework.h"
#include "Job.h"
#include <pthread.h>
#include <atomic>
#include <algorithm>


void getJobState(JobHandle job, JobState* state)
{
  Job *_job = (Job*) job;
  _job->set_state (*state);
}

void closeJobHandle(JobHandle job)
{
  waitForJob (job);
  // Delete Job
}

bool pair_check_less(const IntermediatePair &p1, const IntermediatePair &p2){
  return *p1.first < *p2.first;
}

void* thread_logic(void* arg)
{
  ThreadContext* tc = (ThreadContext*) arg;
  Job *thread_job = (Job *)tc->job_handle;
  MapReduceClient &client = (MapReduceClient &) thread_job->get_client();
  InputVec input_vec = thread_job->get_inputs_elements();
  OutputVec output_vec = thread_job->get_output_elements();
  int thread_id = tc->thread_id;
  // BEGIN: Mapping Phase
  unsigned long old_value = 0;
  while((old_value = *(tc->input_elements)++) < input_vec.size())
  {
    client.map (input_vec[old_value].first,
                input_vec[old_value].second, tc);
  }
  // END: Mapping Phase
  // BEGIN: Sorting Phase & Preparation
  IntermediateVec *intermediate_vectors =
      thread_job->get_intermediate_vectors();
  if(! intermediate_vectors->empty())
  {
    std::sort(intermediate_vectors[thread_id].begin(),
              intermediate_vectors[thread_id].end(), pair_check_less );
    // TODO: Prepare the Keys for Shuffling and continue to the Shuffling Phase
  }
  // END: Sorting Phase

  thread_job->barrier.barrier();

  return 0;
}

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel)
{
  std::atomic<int> atomic_counter(0);
  Job *new_job = new Job({UNDEFINED_STAGE, 0}, inputVec, outputVec, client,
                         multiThreadLevel);
  threads_collection job_threads;
  job_threads.reserve (multiThreadLevel);
  for(int i = 0; i < multiThreadLevel; ++i)
  {
    ThreadContext thread_context = {&atomic_counter, new_job, i};
    pthread_t* new_thread = new pthread_t;
    job_threads.push_back (std::make_pair (new_thread, thread_context));
    pthread_create (new_thread, NULL, thread_logic, &thread_context);
  }
  new_job->set_threads (job_threads);
  for(auto it = job_threads.begin(); it != job_threads.end(); ++it)
  {
    pthread_join (*(it->first), NULL);
  }
  return new_job;
}

void waitForJob(JobHandle job)
{

}

void emit2 (K2* key, V2* value, void* context)
{
  ThreadContext *tc = (ThreadContext*) context;
  Job *job = (Job *) tc->job_handle;
  int thread_id = tc->thread_id;
  job->get_intermediate_vectors()[thread_id].push_back (IntermediatePair
  (key, value));
}

void emit3 (K3* key, V3* value, void* context)
{
  ThreadContext *tc = (ThreadContext*) context;
  Job *job = (Job *) tc->job_handle;
  int thread_id = tc->thread_id;
  // TODO: Complete the logic here + Use Mutexes
}