#include "MapReduceFramework.h"
#include "Job.h"

void getJobState(JobHandle job, JobState* state)
{
  Job *_job = (Job*) job;
  _job->set_state (state);
}

void closeJobHandle(JobHandle job)
{
  waitForJob (job);
  free(job);
}