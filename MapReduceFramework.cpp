#include "MapReduceFramework.h"
#include "Job.h"
#include <pthread.h>
#include <atomic>
#include <algorithm>
#include <iterator>


void getJobState (JobHandle job, JobState *state)
{
  Job *_job = (Job *) job;
  state->stage=_job->get_stage();
  state->percentage=_job->get_percentage();
}

void closeJobHandle (JobHandle job)
{
  waitForJob (job);
  delete ((Job *) job);
  // Delete Job
}

bool pair_check_equals (const K2 *key1, const K2 *key2)
{
  return !((*key1 < *key2) || (*key2 < *key1));
}

bool pair_check_less (const IntermediatePair &p1, const IntermediatePair &p2)
{
  return *p1.first < *p2.first;
}

void *thread_logic (void *arg)
{
  ThreadContext *tc = (ThreadContext *) arg;
  Job *thread_job = (Job *) tc->job_handle;
  MapReduceClient &client = (MapReduceClient &) thread_job->get_client ();
  InputVec input_vec = thread_job->get_inputs_elements ();
  OutputVec output_vec = thread_job->get_output_elements ();
  int thread_id = tc->thread_id;
  // BEGIN: Mapping Phase
  thread_job->set_stage(MAP_STAGE);
  unsigned long old_value = 0;
/***
 * if theres a syncronization proplem with map might need to add atomic wait.
 */
  while ((old_value = *(tc->input_elements)++) < input_vec.size ())
  {
    client.map (input_vec[old_value].first,
                input_vec[old_value].second, tc);
  }
  // END: Mapping Phase
  // BEGIN: Sorting Phase & Preparation
  IntermediateVec *intermediate_vectors =
      thread_job->get_intermediate_vectors ();
  intermediate_unique_k2_vector *unique_k2_vectors =
      thread_job->get_unique_k2_keys ();
  if (!intermediate_vectors->empty ())
  {
    std::sort (intermediate_vectors[thread_id].begin (),
               intermediate_vectors[thread_id].end (), pair_check_less);
    // The Shuffling Phase goes here, nothing too interesting
    // Grouping pairs by key
    std::transform (
        intermediate_vectors[thread_id].begin (),
        intermediate_vectors[thread_id].end (),
        std::back_inserter (unique_k2_vectors[thread_id]),
        [] (IntermediatePair &pair)
        { return pair.first; });
    auto it = std::unique (unique_k2_vectors[thread_id].begin (),
                           unique_k2_vectors[thread_id].end (),
                           pair_check_equals);
    unique_k2_vectors[thread_id].resize ((unsigned long) std::distance
        (unique_k2_vectors[thread_id].begin (), it));
  }
  /***
   * just making sure, this is sorting back into intermidate vectors and uniq k2 is just the uniqe keys we never append the values of reappiring keys here.
   */
  // END: Sorting Phase

  thread_job->barrier.barrier ();

  // BEGIN: Shuffling Phase
  /**
   * great, it should block untill all threads finish map and only one shall do the shuffle
   */
  int initial_value = (*(tc->shuffle_atomic))++;
  if (initial_value == 0)
  {
    thread_job->set_stage(SHUFFLE_STAGE);
    thread_job->load_intermidiate_elements_count();
    intermediate_unique_k2_vector unique_keys_copy;
    for (int i = 0; i < thread_job->get_threads_count (); ++i)
    {
      // Spreading the keys
      std::copy (unique_k2_vectors[i].begin (), unique_k2_vectors[i].end (),
                 std::back_inserter (unique_keys_copy));
    }
    auto it = std::unique (unique_keys_copy.begin (), unique_keys_copy.end (),
                           pair_check_equals);
    unique_keys_copy.resize ((unsigned long) std::distance (
        unique_keys_copy.begin (), it)
    );
    tc->unique_k2_keys_count = unique_keys_copy.size ();
    while (!unique_keys_copy.empty ())
    {
      K2 *key = unique_keys_copy.back ();
      unique_keys_copy.pop_back ();
      auto new_keys_vec = IntermediateVec ();
      for (int i = 0; i < thread_job->get_intermediate_vectors ()->size ();
           i++)
      {
          //added the [i] int the key check
        while ((!thread_job->get_intermediate_vectors ()[i].empty ()) &&
               pair_check_equals (thread_job->get_intermediate_vectors ()[i].back ().first,
                                  key))
        {
          tc->shuffle_atomic++;
          new_keys_vec.push_back (thread_job->get_intermediate_vectors ()[i]
                                      .back ());
          thread_job->get_intermediate_vectors ()[i].pop_back ();
        }
      }
        //place the new_keys_vec into the shuffeld vec
        thread_job->shuffeld_vec.push_back(new_keys_vec);
    }
  }
    // END: Shuffling Phase
  // Wait for the Shuffling thread to finish
  thread_job->barrier.barrier();
  // BEGIN: reducings Phase
  thread_job->set_stage(REDUCE_STAGE);
  IntermediateVec current_vec;
  int reduce_counter;
  while(true)
  {
      pthread_mutex_lock(&thread_job->shuffeld_vector_mutex);
      if (thread_job->shuffeld_vec.empty())
      {
          pthread_mutex_unlock(&thread_job->shuffeld_vector_mutex);
          break;
      }
      current_vec = thread_job->shuffeld_vec.back();
      reduce_counter = current_vec.size();
      thread_job->shuffeld_vec.pop_back();
      pthread_mutex_unlock(&thread_job->shuffeld_vector_mutex);
      client.reduce(&current_vec,tc);
      tc->reduce_atomic->fetch_add(reduce_counter);
  }
    // END reducing phase

  /**
   * shouldn't the reduce part go here instead of return?
   */
  return 0;
}

JobHandle startMapReduceJob (const MapReduceClient &client,
                             const InputVec &inputVec, OutputVec &outputVec,
                             int multiThreadLevel)
{
  std::atomic<int>* input_elements(0);
  std::atomic<int>* shuffle_atomic(0);
  std::atomic<int>* reduce_atomic(0);
  Job *new_job = new Job ({UNDEFINED_STAGE, 0}, inputVec, outputVec, client,
                          multiThreadLevel);
  threads_collection job_threads;
  job_threads.reserve (multiThreadLevel);
  for (int i = 0; i < multiThreadLevel; ++i)
  {
    ThreadContext thread_context = {
        input_elements,
        shuffle_atomic,
        reduce_atomic,
        new_job,
        i,
        0};
    pthread_t *new_thread = new pthread_t;
    job_threads.push_back (std::make_pair (new_thread, thread_context));
    pthread_create (new_thread, NULL, thread_logic, &thread_context);
  }
  new_job->set_threads (job_threads);
//  for (auto it = job_threads.begin (); it != job_threads.end (); ++it)
//  {
//    pthread_join (*(it->first), NULL);//
//  }
  return new_job;
}

void waitForJob (JobHandle job)//TODO add check for multiple wait for
{
    threads_collection job_threads = ((Job *) job)->get_threads();
    for (auto it = job_threads.begin (); it != job_threads.end (); ++it)
    {
        pthread_join (*(it->first), NULL);
    }
}

void emit2 (K2 *key, V2 *value, void *context)
{
  ThreadContext *tc = (ThreadContext *) context;
  Job *job = (Job *) tc->job_handle;
  int thread_id = tc->thread_id;
  job->get_intermediate_vectors ()[thread_id].push_back (IntermediatePair
                                                             (key, value));
}

void emit3 (K3 *key, V3 *value, void *context)
{
  ThreadContext *tc = (ThreadContext *) context;
  Job *job = (Job *) tc->job_handle;
  int thread_id = tc->thread_id;
  pthread_mutex_lock(&job->output_vector_mutex);
  OutputVec (job->get_output_elements()).push_back (OutputPair (key, value));
  pthread_mutex_unlock(&job->output_vector_mutex);
  // this should work
}