/*
Copyright (C) 2011 Melissa Gymrek <mgymrek@mit.edu>

This file is part of lobSTR.

lobSTR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

lobSTR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with lobSTR.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SRC_MULTITHREADDATA_H__
#define SRC_MULTITHREADDATA_H__

#include <err.h>
#include <pthread.h>
#include <semaphore.h>

#include <list>
#include <string>

#include "src/ReadPair.h"

template<class ITEM>
class ProtectedList {
 private:
  std::list <ITEM> items;
  pthread_mutex_t list_access;
  sem_t empty_slots;
  sem_t full_slots;
  size_t num_items;
  int slots;

 public:
  explicit ProtectedList(int _slots) :
  empty_slots(-1),
  full_slots(-1),
  num_items(0),
    slots(_slots) {
      if (pthread_mutex_init(&list_access, NULL)!=0)
	err(1,"ProtectedList::ctor(): pthread_mutex_init() failed");
      if (sem_init(&empty_slots, 0, _slots)!=0)
	err(1,"ProtectedList::ctor(): sem_init(&empty_slots) failed");
      if (sem_init(&full_slots, 0, 0)!=0)
	err(1,"ProtectedList::ctor(): sem_init(&full_slots) failed");

      num_items = 0;
    }

  void put(ITEM item) {
    if (sem_wait(&empty_slots)!=0)
	err(1,"ProtectedList::put(): sem_wait(&empty_slots) failed");

    // Lock the list access
    if (pthread_mutex_lock(&list_access)!=0)
	err(1,"ProtectedList::ctor(): pthread_mutex_lock() failed");

    // Add this record to the list
    items.push_back(item);

    // Release the lock
    if (pthread_mutex_unlock(&list_access)!=0)
	err(1,"ProtectedList::ctor(): pthread_mutex_unlock() failed");

    num_items++;

    // Flag consumer threads
    if (sem_post(&full_slots)!=0)
	err(1,"ProtectedList::put(): sem_post(&full_slots) failed");
  }

  ITEM get() {
    // Wait for a semaphore to be avialble
    if (sem_wait(&full_slots) != 0)
	err(1,"ProtectedList::Get(): sem_wait(&full_slots) failed");

    // Lock the list access
    if (pthread_mutex_lock(&list_access)!=0)
	err(1,"ProtectedList::ctor(): pthread_mutex_lock() failed");

    // Get an element from the list, and remove it

    // Should never happen...
    if (items.empty())
      errx(1, "Internal error: Multithreaded " \
           "Protected list is empty (after getting a semaphore)");

    ITEM item = items.front();
    items.pop_front();
    num_items--;

    // Release the lock
    if (pthread_mutex_unlock(&list_access)!=0)
	err(1,"ProtectedList::ctor(): pthread_mutex_unlock() failed");
    if (sem_post(&empty_slots)!=0)
	err(1,"ProtectedList::get(): sem_post(&empty_slots) failed");
    return item;
  }

  void wait_for_all_slots() {
    for (int i = 0; i < slots; ++i) {
      if (sem_wait(&empty_slots)!=0)
	err(1,"ProtectedList::wait_for_all_slots(): sem_wait(&empty_slots) failed");
    }
  }
};

class MultithreadData {
 private:
  int slots;
  ProtectedList<ReadPair*> items_to_process;
  ProtectedList<ReadPair*> items_to_output;

  pthread_mutex_t counter_mutex;
  size_t input_count;
  size_t output_count;

 public:
  explicit MultithreadData(int _slots);

  /* From the READER(Producer) to the Satellite
     processing threads (consumer) */
  void post_new_input_read(ReadPair* pRecord);

  /* Used in the Satellite processing threads */
  ReadPair* get_new_input();

  void wait_for_completed_input_processing();

  /* From the Satellite processing threads to the output-writer thread */
  void post_new_output_read(ReadPair *pRecord);

  /* Used in the Output-Writer thread */
  ReadPair* get_new_output();
  void wait_for_completed_output_processing();
  void increment_input_counter();
  void increment_output_counter();
  bool input_output_counters_equal();
};

#endif  // SRC_MULTITHREADDATA_H__
