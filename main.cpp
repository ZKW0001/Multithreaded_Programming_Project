
/*******************************************************************************
  * main.cpp
  * 
  * This file contains the main code to simulate the 4x100 meter relay with threads
  * 
  * Date: 2024-11-5
  * 
  * Author: Kaiwen Zhao
********************************************************************************/


#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <random>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <array>
#include "cs_helper_DoNotModify.hpp"

using namespace std;

const int NUM_TEAMS = 4; // number of teams in the race
const int NUM_MEMBERS = 4; // number of athletes in the team

// Data for team/athelete initialisation. The Women’s 4x100 meter relay at the Tokyo 2020 Olympics. The teams took between 41 and 42 seconds.
std::array< string, 4> astrTeams = { "Jamaica", "United States", "Great Britain", "Switzerland" };
std::array< std::array<std::string, 4>, 4> aastrCompetitors = { {
    { "Briana Williams", "Elaine Thompson-Herah", "Shelly-Ann Fraser-Pryce", "Shericka Jackson" },
    { "Javianne Oliver", "Teahna Daniels", "Jenna Prandini", "Gabrielle Thomas" },
    { "Asha Philip", "Imani Lansiquot", "Dina Asher-Smith", "Daryll Neita" },
    { "Ajla Del Ponte", "Mujinga Kambundji", "Salomé Kora", "Riccarda Dietsche" }
} };

class RandomTwister {
public:
    RandomTwister(float min, float max) : distribution(min, max) {}  // Initialises uniform_real_distribution to initialize to a specific range of numbers
    float generate() // Returns a random float within the specified range
    {
        //Make the Random number generator thread-safe by adding a simple std::mutex to the class and locking it before generating the random number
		std::lock_guard<std::mutex> lock(mtx); // ensure thread-safety
        return distribution(engine);
    }
private:
    // std::random_device creates a seed value for the mt19937 instance creation. It creates a seed value for the “mt” random number generator
    std::mt19937 engine{ std::random_device{}() }; // Mersenne Twister random number generator engine, with a seed from random_device() - static
    std::uniform_real_distribution<float> distribution; // This uniform_real_distribution transforms the engine output into the required (min, max) range and data type.
	std::mutex mtx; // Mutex to ensure thread-safety
};



// Global mutex for thread-safe printing
std::mutex thrd_print_mtx;
void thrd_print(const std::string& str) {  // Thread safe print
    std::lock_guard<std::mutex> lock(thrd_print_mtx);
    cout << str;
}

barrier barrier_allthreads_started(1 + (NUM_TEAMS * NUM_MEMBERS)); // Need all the thread to reach here before the start can continue.
//Part 1.3 Create another barrier array and name it "barrier_go" which you will use to make all threads wait until the race official starts the race
barrier barrier_go(1 + NUM_TEAMS * NUM_MEMBERS);  // Barrier for starting race
//Part 2.1  Create a std::atomic variable of type bool, initalised to false and name it "winner". You will use it to ensure just the winning thread claims to have won the race.
std::atomic<bool> winner{ false }; // fasle means no winner yet
std::atomic<int> drop_runner_index{ -1 }; // Initialize with -1 (no runner selected yet)


void thd_runner_4x4x100m(Competitor& a, Competitor* pPrevA, RandomTwister& generator, RandomTwister& Droptime, int runner_index) {
    thrd_print(a.getPerson() + " ready, \n");
    barrier_allthreads_started.arrive_and_wait();
    barrier_go.arrive_and_wait();
    thrd_print(a.getPerson() + " started, \n");


    // If the competitor does not have a pointer to a previous competitor, then it must be the first runner of that team.
    if (pPrevA == NULL)  thrd_print(a.getPerson() + " started, \n");
	else {// If they are not the first runner in that team, then they need to wait for the previous runner to give them the baton.
        {// Brackets to reduce mutex scope
    
        std::unique_lock<std::mutex> lock(pPrevA->mtx); // Lock the previous runner's mutex
		pPrevA->baton.wait(lock, [&pPrevA]() { return pPrevA->bFinished; }); // Wait for the baton to arrive.
		}

        thrd_print(a.getPerson() + " (" + a.getTeamName() + ")" + " took the baton from " + pPrevA->getPerson() + " (" + pPrevA->getTeamName() + ")\n");
    }

    //The fSprintDuration_seconds
    float fSprintDuration_seconds = generator.generate();
    // thrd_print("Runner index: " + std::to_string(runner_index) + ", Drop index: " + std::to_string(drop_runner_index.load()) + "\n");

    // Check if this is the preselected drop runner
    if (runner_index == drop_runner_index.load()) {
        float DropTime = Droptime.generate();
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(DropTime * 1000)));

        thrd_print(a.getPerson() + " dropped the baton. (" + a.getTeamName() + ")\n");
        thrd_print(a.getPerson() + " picked up the baton. (" + a.getTeamName() + " + " + std::to_string(DropTime) + " s + 2 s penalty)\n");
        fSprintDuration_seconds += DropTime + 2.0f;
    }

    // The std::this_thread::sleep_for
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(fSprintDuration_seconds * 1000)));
    a.setTime(fSprintDuration_seconds);
    thrd_print("Leg " + std::to_string(a.numBatonExchanges()) + ": " + a.getPerson() + " ran in " + std::to_string(fSprintDuration_seconds) + " seconds. (" + a.getTeamName() + ")\n");

    if (a.numBatonExchanges() == NUM_MEMBERS)  // The last athlete in the team has crossed the finish line (crossing the line counts as a baton exchage)
    {
        // Print "finished" only if this is the first thread to complete
        if (!winner.exchange(true)) 
        {
            std::cout << "\n Team " << a.getTeamName() << " is the WINNER!\n" << std::endl;
        }
    }
}

// Create a class called Race
class Race {
private:
    Team aTeams[NUM_TEAMS];  //can be global
    Competitor athlete[NUM_TEAMS][NUM_MEMBERS];  // 2D array of competitors
    std::thread thread_competitor[NUM_TEAMS][NUM_MEMBERS];  // 2D array of threads.

    // Change the random number generation to between 10 s and 12 s.
    RandomTwister randGen_sprint_time{ 10.0f, 12.0f };  
	RandomTwister Droptime{ 1.0f, 3.0f };  // Random time for dropping the baton

public:
    Race() {
        for (int i = 0; i < NUM_TEAMS; ++i) {
			aTeams[i].setTeam(astrTeams[i]);  // Create the team information
            for (int j = 0; j < NUM_MEMBERS; ++j) {
                athlete[i][j].set(aastrCompetitors[i][j], &aTeams[i]);    // Create the athlete information
            }
        }

        // Randomly select one runner (excluding indices 0, 4, 8, and 12)
        std::random_device rd;
        std::mt19937 mt(rd());
        std::vector<int> eligible_runners;

        for (int i = 0; i < NUM_TEAMS * NUM_MEMBERS; ++i) {
            if (i != 0 && i != 4 && i != 8 && i != 12) {
                eligible_runners.push_back(i);
            }
        }
        std::uniform_int_distribution<int> random_index(0, eligible_runners.size() - 1);
		drop_runner_index.store(eligible_runners[random_index(mt)]);  // Randomly select a runner to drop the baton (store in drop_runner_index)

        //thrd_print("Drop runner index selected: " + std::to_string(drop_runner_index.load()) + "\n");
    }

    void initializeRace() {
		// Create the threads
        for (int i = 0; i < NUM_TEAMS; ++i) {
            for (int j = 0; j < NUM_MEMBERS; ++j) {
				int runner_index = i * NUM_MEMBERS + j; // Calculate the runner index

                //Start the thd_runner_4x4x100m instead.
                if (j == 0) {
                    thread_competitor[i][j] = std::thread(thd_runner_4x4x100m,
                        std::ref(athlete[i][j]), nullptr, std::ref(randGen_sprint_time), std::ref(Droptime), runner_index);
                }
                else {
                    thread_competitor[i][j] = std::thread(thd_runner_4x4x100m,
                        std::ref(athlete[i][j]), &athlete[i][j - 1], std::ref(randGen_sprint_time), std::ref(Droptime), runner_index);
                }
            }
        }
    }

    void startRace() {
        // Wait for all threads to be running including the main thread
        // Wait at the barrier until all threads arrive

        barrier_allthreads_started.arrive_and_wait();
        thrd_print("\n\nThe race official raises her starting pistol...\n");

        // Change the random number generation to between 3 s and 5 s.
        float fStarterGun_s = RandomTwister(3.0f, 5.0f).generate();
        // The std::this_thread::sleep_for to simulate the time taken for the starter to fire the gun.
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(fStarterGun_s * 1000)));

        //Apply the final barrier_go arrive_and_wait here to start all the competitors running.
        // Wait at the barrier until all threads arrive
        barrier_go.arrive_and_wait();
        thrd_print("\nGO !\n\n");
    }

    // Join all threads
    void joinThreads() {
        for (int i = 0; i < NUM_TEAMS; ++i) {
            for (int j = 0; j < NUM_MEMBERS; ++j) {
                //For all thread_competitor[i][j], test the thread is joinable, and if so, join it

                if (thread_competitor[i][j].joinable()) {
                    thread_competitor[i][j].join();
                }
            }
        }

        // Print the results for each team
        std::cout << "\n\nTEAM RESULTS" << std::endl;
        for (int i = 0; i < NUM_TEAMS; ++i) {
            aTeams[i].printTimes();
        }
        std::cout << std::endl;
    }
};


int main() {
    Race race;
	race.initializeRace();
    race.startRace();
    race.joinThreads();
    return 0;
}