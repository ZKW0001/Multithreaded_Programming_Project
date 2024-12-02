# Multithreaded_Programming_Project

The program simulates a 4x100 meter relay race with 4 teams, leveraging multithreading in C++ to model the concurrent actions of athletes in the race. The simulation incorporates randomness, baton exchange mechanics, and potential errors like dropping the baton, adding realism to the model.  

- START CONDITION
When the simulation starts, the program ensures synchronization by using a barrier `barrier_allthreads_started`. This ensures that all threads (16 subthreads representing the athlete and the main thread) reach the same point before proceeding further. Once all athletes are ready, the main thread simulates a race official raising the starting pistol and then triggers the race after a short randomized delay between 3 seconds and 5 seconds. After the "GO!" signal, each thread (athlete) begins running, with the sprint times being simulated using random values between 10s and 12s.  

- HANDOVER OF THE BATON
The current athlete locks the previous athlete's mutex `pPrevA->mtx` to ensure thread-safe access and then waits on the previous athlete's condition variable `pPrevA->baton` until the previous athlete signals they have finished `bFinished == true`  

One random athlete will drop the baton in this rely. A panlty of 2s will be added to the team's final results.  

- WINNER CLAIM
The winner is determined using an atomic flag `winner`, which ensures only one thread can claim victory by atomically setting the flag to true using `exchange()`. If the flag was previously false, the thread declares its team as the winner; otherwise, it does nothing, ensuring only one winner is announced even with concurrent threads.  

## Example output
```text
Elaine Thompson-Herah ready,
Briana Williams ready,
Shelly-Ann Fraser-Pryce ready,
Shericka Jackson ready,
Javianne Oliver ready,
Teahna Daniels ready,
Jenna Prandini ready,
Gabrielle Thomas ready,
Asha Philip ready,
Imani Lansiquot ready,
Dina Asher-Smith ready,
Daryll Neita ready,
Ajla Del Ponte ready,
Mujinga Kambundji ready,
Salomé Kora ready,
Riccarda Dietsche ready,


The race official raises her starting pistol...

GO !

Ajla Del Ponte started,
Ajla Del Ponte started,
Briana Williams started,
Briana Williams started,
Shericka Jackson started,
Gabrielle Thomas started,
Jenna Prandini started,
Dina Asher-Smith started,
Asha Philip started,
Imani Lansiquot started,
Daryll Neita started,
Teahna Daniels started,
Mujinga Kambundji started,
Salomé Kora started,
Javianne Oliver started,
Shelly-Ann Fraser-Pryce started,
Elaine Thompson-Herah started,
Riccarda Dietsche started,
Asha Philip started,
Javianne Oliver started,
Elaine Thompson-Herah (Jamaica) took the baton from Briana Williams (Jamaica)
Leg 1: Briana Williams ran in 10.358055 seconds. (Jamaica)
Leg 1: Javianne Oliver ran in 11.072844 seconds. (United States)
Teahna Daniels (United States) took the baton from Javianne Oliver (United States)
Imani Lansiquot (Great Britain) took the baton from Asha Philip (Great Britain)
Leg 1: Asha Philip ran in 11.221483 seconds. (Great Britain)
Mujinga Kambundji (Switzerland) took the baton from Ajla Del Ponte (Switzerland)
Leg 1: Ajla Del Ponte ran in 11.353963 seconds. (Switzerland)
Dina Asher-Smith (Great Britain) took the baton from Imani Lansiquot (Great Britain)
Leg 2: Imani Lansiquot ran in 10.035055 seconds. (Great Britain)
Shelly-Ann Fraser-Pryce (Jamaica) took the baton from Elaine Thompson-Herah (Jamaica)
Leg 2: Elaine Thompson-Herah ran in 11.624727 seconds. (Jamaica)
Salomé Kora (Switzerland) took the baton from Mujinga Kambundji (Switzerland)
Leg 2: Mujinga Kambundji ran in 11.112641 seconds. (Switzerland)
Dina Asher-Smith dropped the baton. (Great Britain)
Dina Asher-Smith picked up the baton. (Great Britain + 1.540623 s + 2 s penalty)
Jenna Prandini (United States) took the baton from Teahna Daniels (United States)
Leg 2: Teahna Daniels ran in 11.779644 seconds. (United States)
Riccarda Dietsche (Switzerland) took the baton from Salomé Kora (Switzerland)
Shericka Jackson (Jamaica) took the baton from Shelly-Ann Fraser-Pryce (Jamaica)
Leg 3: Shelly-Ann Fraser-Pryce ran in 11.223081 seconds. (Jamaica)
Leg 3: Salomé Kora ran in 10.726634 seconds. (Switzerland)
Gabrielle Thomas (United States) took the baton from Jenna Prandini (United States)
Leg 3: Jenna Prandini ran in 11.495650 seconds. (United States)
Leg 3: Dina Asher-Smith ran in 14.363584 seconds. (Great Britain)
Daryll Neita (Great Britain) took the baton from Dina Asher-Smith (Great Britain)
Leg 4: Riccarda Dietsche ran in 10.871929 seconds. (Switzerland)

 Team Switzerland is the WINNER!

Leg 4: Shericka Jackson ran in 11.668170 seconds. (Jamaica)
Leg 4: Gabrielle Thomas ran in 10.646404 seconds. (United States)
Leg 4: Daryll Neita ran in 11.526846 seconds. (Great Britain)


TEAM RESULTS
Team Jamaica = 44.874 s
Team United States = 44.9945 s
Team Great Britain = 47.147 s
Team Switzerland = 44.0652 s
