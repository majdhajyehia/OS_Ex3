#include "MapReduceFramework.h"
#include "Job.h"
#include <pthread.h>
#include <atomic>


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

void* startRoutine(void* arg)
{
  ThreadContext* tc = (ThreadContext*) arg;

  for (int i = 0; i < 1000; ++i) {
    // old_value isn't used in this example, but will be necessary
    // in the exercise
    int old_value = (*(tc->atomic_counter))++;
    (void) old_value;  // ignore not used warning
    (*(tc->bad_counter))++;
  }
  return 0;
}

JobHandle startMapReduceJob(const MapReduceClient& client,
                            const InputVec& inputVec, OutputVec& outputVec,
                            int multiThreadLevel)
{
  std::atomic<int> atomic_counter(0);
  ThreadContext thread_context = {&atomic_counter};
  threads_collection job_threads;
  job_threads.reserve (multiThreadLevel);
  for(int i = 0; i < multiThreadLevel; ++i)
  {
    pthread_t* new_thread = new pthread_t;
    job_threads.push_back (std::make_pair (new_thread, thread_context));
    // TODO: Complete the startRoutine Implementation
    pthread_create (new_thread, NULL, startRoutine, &thread_context);
  }
  Job *new_job = new Job(job_threads, {UNDEFINED_STAGE, 0});
  return new_job;
}

void waitForJob(JobHandle job)
{

}
