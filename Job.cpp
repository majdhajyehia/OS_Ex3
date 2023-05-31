#include "Job.h"
/**
 * can this constructor be deleated, is it used?
 */
//Job::Job (threads_collection threads, JobState state, const MapReduceClient
//&client) : _threads
//(threads),
//_state(state), _client(client)
//{
//  this->_intermediate_vectors = new IntermediateVec [threads.size()];
//}
/**
 *
 * @param state the state of the job should be init as undifiend
 * @param input_vec the vector of inputs from the client
 * @param output_vec empty vec to return to client
 * @param client client map reduce
 * @param threads_count the amount of threads to run
 */
Job::Job (JobState state, InputVec input_vec,
          OutputVec output_vec, const MapReduceClient &client, int
          threads_count) :
          _threads({}),
          _state(state),
          _input_elements(input_vec),
          _output_elements(output_vec),
          _client(client),
          _threads_count(threads_count),
          barrier(Barrier(threads_count)), //added berrier creation
          shuffeld_vector_mutex(PTHREAD_MUTEX_INITIALIZER), //added mutex creation to lock the intermidiate vector
          output_vector_mutex(PTHREAD_MUTEX_INITIALIZER) //added mutex creation to lock the output vector
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
    stage_t current_stage = get_stage();
    if (current_stage==UNDEFINED_STAGE)
    {
        _state.percentage=0.0;
    }
    if(current_stage==MAP_STAGE)
    {
        _state.percentage = float(_threads.back().second.input_elements->load()/_input_elements.size());
    }
    if(current_stage==SHUFFLE_STAGE)
    {
        if(_intermidiate_elements_count == 0)
        {
            load_intermidiate_elements_count();
        }
        if (_intermidiate_elements_count == 0)
        {
            _state.percentage =0;
        }
        else {
            _state.percentage = float(_threads.back().second.shuffle_atomic->load() / _intermidiate_elements_count);
        }
    }
    if (current_stage == REDUCE_STAGE)
    {
        //might need to check for reduced instead of intermidiate but i think its ok
        _state.percentage = float(_threads.back().second.reduce_atomic->load() / _intermidiate_elements_count);
    }
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

void Job::set_intermediate_vectors(IntermediateVec* intermediate_vectors)
{
    this->_intermediate_vectors = intermediate_vectors;
}

intermediate_unique_k2_vector* Job::get_unique_k2_keys ()
{
  return this->_unique_k2_keys;
}
/**
 * loads the number of elements in all intermidaiate vectors for precantage calculation
 */
void Job::load_intermidiate_elements_count() {
    int count = 0;
    for (int i = 0; i<_intermediate_vectors->size(); i++)
    {
        count = count + _intermediate_vectors[i].size();
    }
    _intermidiate_elements_count = count;
}
