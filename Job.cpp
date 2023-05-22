#include "Job.h"

const JobState Job::get_state ()
{
  return this->_state;
}

void Job::set_state (JobState state)
{
  this->_state = state;
}
