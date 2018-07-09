// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of 
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "ligra.h"
#include "sched.h"
#include "pthread.h"
struct BFS_F {
	uintE* Parents;
	BFS_F(uintE* _Parents) : Parents(_Parents) {}
	inline bool update (uintE s, uintE d) { //Update
		if(Parents[d] == UINT_E_MAX) { Parents[d] = s; return 1; }
		else return 0;
	}
	inline bool updateAtomic (uintE s, uintE d){ //atomic version of Update
		return (CAS(&Parents[d],UINT_E_MAX,s));
	}
	//cond function checks if vertex has been visited yet
	inline bool cond (uintE d) { return (Parents[d] == UINT_E_MAX); } 
};

struct thread_info{
	int tid;
	int thread_num;
	long vertices;
	uintE* Parents;
};

typedef struct thread_info t_info;

inline void setaffinity_oncpu(int cpu)
{
	cpu_set_t cpu_mask;
	int err;

	CPU_ZERO(&cpu_mask);
	CPU_SET(cpu, &cpu_mask);

	err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
	if (err) {
		perror("sched_setaffinity");
		exit(1);
	}

}


inline void *BFS_worker(void *argv )
{       
	t_info *t = (t_info*) argv;
	int thread_num = t->thread_num;
	int id = t->tid;
	long vertices = t->vertices;  // Read thread's private info 
	uintE* Parents;
	Parents = t->Parents;
	

	int chunk;
	chunk = vertices / thread_num; 

	long start_vertex; 
	long end_vertex;

	start_vertex = chunk*id;
	end_vertex = chunk*id + chunk - 1; // Calculate chunk and vertices 
	
	
	for(long i=start_vertex; i <= end_vertex; i++) Parents[i] = UINT_E_MAX;

      
	fprintf(stdout,"[%d] Start = %d End = %d \n ", id, start_vertex , end_vertex);	
		
	

	setaffinity_oncpu(id);

}



inline pthread_t * spawn_threads(int thread_num, int n, uintE* Parents)
{

	pthread_t * thread =(pthread_t *) malloc(sizeof(pthread_t)*thread_num);
	int i;
	t_info* t; 
	t = (t_info *)malloc(sizeof(t_info) * thread_num);
	for (i = 0; i < thread_num; i++) {
		//printf("t address = %p\n", &t);
		t[i].tid = i;
		t[i].thread_num = thread_num;
		t[i].vertices = n;
		t[i].Parents = Parents;
		int ret = pthread_create(&thread[i], NULL,BFS_worker,(void*) &t[i]);
		if(ret != 0) {
			printf ("Create pthread error!\n");
			exit (1);
		}
	}
	return thread;
}

inline void join_threads(pthread_t* threads, int thread_num)
{
	int i;
	for (i = 0; i < thread_num; i++) pthread_join(threads[i],NULL);

}

template <class vertex>
void Compute(graph<vertex>& GA, commandLine P) {
	long start = P.getOptionLongValue("-r",0);
	long n = GA.n;

	//creates Parents array, initialized to all -1, except for start
	uintE* Parents = newA(uintE,n);
	pthread_t* t = spawn_threads(4,Parents);
	join_threads(t,4);
	//parallel_for(long i=0;i<n;i++) Parents[i] = UINT_E_MAX;
	Parents[start] = start;
	vertexSubset Frontier(n,start); //creates initial frontier
	while(!Frontier.isEmpty()){ //loop until frontier is empty
		vertexSubset output = edgeMap(GA, Frontier, BFS_F(Parents));    
		Frontier.del();
		Frontier = output; //set new frontier
	} 
	Frontier.del();
	free(Parents); 
}










