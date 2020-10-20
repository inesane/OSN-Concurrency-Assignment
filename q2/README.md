Assumptions
I have taken a delay for the students to reach the gate of the campus to be a random integer between 2 and 5 seconds.

I use 3 structures and 3 threads, one each for the pharmaceutical companies, vaccination zones and students.

3 Mutexes have been used as well to ensure that multiple threads are not running through the same code simultaneously.

Functions

company_function: This function is the function that we run the thread for all the pharmaceutical companies through. We run this function while the number of students who haven't been vaccinated are not 0. This function checks if all the vaccines manufactured in the previous round of manufacturing by a specific company have been used up already by the vaccination zones and executes the manufacturing process for the vaccines. For each manufacturing process, it produces a random number of batches (1 - 5) of vaccines, each batch containing a random number of vaccines (10-20)

zone_function: This function is the function that we run the thread for all the vaccination zones through. This function checks if the vaccination zone has run out of vaccines or not and if it has, it gets a batch of vaccines delivered from one of the pharmaceutical companies to the vaccination zone. It also chooses a random number of slots, each slot corresponding to a place for a student to get vaccinated in the current vaccination phase. It initiates the vaccination phases as well.

student_function: This function is the function that we run the thread for all the students through. This function takes each student from the gate through the whole vaccination process. Firstly, it checks if the number of times the current student has been vaccinated is 3 or more. If it is, it sends the student home for another online semester as the student has not received the necessary antibodies from a vaccine on 3 separate occasions. If the student has been vaccinated less than 3 times, they are allocated a vaccination zone and a slot in the vaccination zone. Then, using the given probabilites for each vaccine, it is calculated whether the student has received the required antibodies from the vaccine or not. If the student has, they are allowed to go to college. If they do not receive the antibodies, the student is sent back to the gate to go through the vaccination process again.

These functions are continuosly run until each student has either been approved to go to college or are sent home.