#include "Job.h"

Job::Job (threads_collection threads, JobState state) : _threads(threads),
_state(state)
{}

const JobState Job::get_state ()
{
  return this->_state;
}

void Job::set_state (JobState state)
{
  this->_state = state;
}