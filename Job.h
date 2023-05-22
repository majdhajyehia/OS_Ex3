//
// Created by majd.hajyehia on 5/22/23.
//

#ifndef RESOURCES_JOB_H
#define RESOURCES_JOB_H
#include "MapReduceFramework.h"


class Job{
private:
    JobState _state;
public:
  const JobState get_state();
  void set_state(JobState state);
};

#endif //RESOURCES_JOB_H
