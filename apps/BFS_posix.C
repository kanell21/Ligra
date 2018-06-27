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
	thread_info(int _tid, int _thread_num) : tid(_tid), thread_num(_thread_num) {}
};

typedef struct thread_info t_info;

template <class vertex>
void Compute(graph<vertex>& GA, commandLine P) {
	long start = P.getOptionLongValue("-r",0);
	long n = GA.n;
	//creates Parents array, initialized to all -1, except for start
	uintE* Parents = newA(uintE,n);
	parallel_for(long i=0;i<n;i++) Parents[i] = UINT_E_MAX;
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

void *BFS_worker(void *argv )
{
	t_info *t = (t_info*) argv;
	int thread_num = t->thread_num;
	int id = t->tid;
	printf("Tid = ");

}



template <class vertex>
void spawn_threads(int thread_num)
{
	
	pthread_t * thread = malloc(sizeof(pthread_t)*thread_num);
	int i;
	t_info *temp;
	for (i = 0; i < thread_num; i++) {
		t_info t(i,thread_num);
		temp = t;
		int ret = pthread_create(&thread[i], NULL,BFS_worker,(void*) temp);
		if(ret != 0) {
			printf ("Create pthread error!\n");
			exit (1);
		}
	}
}














