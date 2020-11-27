To Run
```
gcc q3.c -lpthread
./a.out
```
In this code I have used 4 semaphores, one for electric stages, one for acoustic stages, one for coordinators and one for musicians.

The electric stages and acoustic stages semaphores are used to ensure that only one performance is occuring at a time on each stage.

The coordinator semaphore ensures that each coordinator is giving tshirts to only one performer at a time.

The musicians semaphore ensures that a singer may join a performer if they wish to.

I have used mutexes as well, one for each player (singers and musicians) and one for each stage.

My functions have been created such that every different type of performer has a separate function.

Functions

bass_or_vio - Bassists and violinists can only perform on one type of stage, so they have one thread that uses a semaphore to wait until a stage opens up for them to perform or they leave due to impatience. The musicians name is then linked to the stage via the stage struct. Then there is a sleep for a random amount of time (according to the given time conditions) for the performance. If a singer joins the performer, 2 threads will be created for the purpose of the 2 performers collecting tshirts (1 tshirt in the case that a singer does not join the musician)

singer_split - This function exists for singer, which can perform on either electric or acoustic or stages that are already occupied by a performing musician. The function creates 3 threads that wait to join an acoustic solo, electric solo or a solo musician. We ensure that only one of these threads are performed and the other 2 kill themselves.

singer_on_electric and singer_on_acoustic- These functions takes care of the case when only a singer is performing on an electric stage. It sleeps for the length of the performance and then creates a thread and passes it through the tshirt function so that the singer may receive their tshirt.

singer_together - This function exists as a singer is allowed to perform with a currently performing musician. If a singer decides to perform along with a musician, the function prints that the singer does so and increases the performance time by 2 seconds.

two_on_stage - Pianists and guitarists are allowed to perform on either acoustic or electric stage, so this function creates 2 threads that wait for either an electric or acoustic stage.

choose_electric and choose_acoustic - These functions takes care of the case when there is a musician that is allowed to perform on either electric or acoustic stage (i.e. a pianist or guitarist). It then executes a sleep for the performance time and creates a thread at the end of the performance time for the tshirt function. If a singer decides to perform along with the musician, 2 threads will be created and passed through the tshirt function for both the singer and the musician so that they may get their tshirts.

tshirt - This function executes the giving of tshirts to the performers by the coordinators. It executes a sleep for 2 seconds for the action of the coordinator giving the tshirt to the performer.

At the end of the execution of the code, 'Finished' is printed.

Both bonuses have been implemented, singers collecting tshirts and the stage number of each performer is printed when a performer starts performing and finishes performing at a stage.