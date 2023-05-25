#include "Job.h"

Job::Job (threads_collection threads, JobState state, const MapReduceClient
&client) : _threads
(threads),
_state(state), _client(client)
{
  this->_intermediate_vectors = new IntermediateVec [threads.size()];
}

Job::Job (JobState state, InputVec input_vec,
          OutputVec output_vec, const MapReduceClient &client, int
          threads_count) :
          _threads({}),
          _state(state),
          _input_elements(input_vec),
          _output_elements(output_vec),
          _client(client),
          _threads_count(threads_count)
{
  this->_intermediate_vectors = new IntermediateVec [threads_count];
}

const JobState Job::get_state ()
{
  return this->_state;
}

void Job::set_state (JobState state)
{
  this->_state = state;
}

void Job::set_threads (threads_collection threads)
{
  this->_threads = threads;
}

void Job::append_thread (thread_pair pair)
{
  this->_threads.push_back (pair);
}

const InputVec Job::get_inputs_elements ()
{
  return this->_input_elements;
}

const OutputVec Job::get_output_elements ()
{
  return this->_output_elements;
}

const MapReduceClient& Job::get_client ()
{
  return this->_client;
}

const float Job::get_percentage()
{
  return this->_state.percentage;
}

const stage_t Job::get_stage()
{
  return this->_state.stage;
}

void Job::set_percentage(float percent)
{
  this->_state = {this->get_stage(), percent};
}

void Job::set_stage (stage_t stage)
{
  this->_state = {stage, this->get_percentage()};
}

const int Job::get_threads_count ()
{
  return this->_threads_count;
}

IntermediateVec* Job::get_intermediate_vectors ()
{
  return this->_intermediate_vectors;
}

