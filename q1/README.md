This code has uses 3 different implementations of sorting with the main sort being merge sort and selection sort being used when the number of elements we are sorting in the current step being less than 5.

The 3 different sorts are normal mergesort, mergesort using processes and mergesort with threads.

Normal mergesort is a classic/normal implementation of mergesort.

Mergesort with processes is an implementation of merge sort that uses processes for every iteration. Two child processes are created for the left and right halves of the array we are currently running the function on. The process waits for these two child processes to finish executing until they continue onto the next iteration of the recursion.

Mergesort with threads is an implementation of mergesort that uses threads for every iteration. Two threads are created for the left and the right halves of the array we are currently running the function on. The thread waits until the 2 threads that were created finish executing and then continues onto the next iteration of the recursion.

By running the program, we observe that the threaded and process implementations of mergesort take a lot more time than the normal mergesort, around 100 times more. The reason for this is because of the number of processes and threads we create to implement the sort for the latter two mergesorts. A large number of threads and processes will have to be created and executed and a majority of them will be running a very small computation such that the cost of creating these threads and processes much higher than the benefit of using concurrent processes.