/**
 * Christian Parsons, William Lorey
 * Section A
 * Dining Philosophers problem solution using message passing
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

#define STARS "********"

// run compiled code (for 5 Phils) with mpirun -n 5 program

// Constant declarations
const unsigned int REQUEST = 0;
const unsigned int NOTIFY = 1;
const unsigned int EXIT = 2;

using namespace std;

// this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 10;

// if you change this base, update the Makefile "clean" accordingly to remove old poems
const string fileBase = "outFile";

int main(int argc, char *argv[])
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  int tag = 1;
  int source;

  bool rightAvailable; // my right fork's (file's) availability
  bool leftAvailable;  // my left fork's availability

  bool leftDone = false; // Track whether our neighbors have finished
  bool rightDone = false;

  bool leftUsed = true; // Every fork is 'used' initially
  bool rightUsed = true;

  string stanza1, stanza2, stanza3;

  //  Initialize MPI.
  MPI::Init(argc, argv);

  //  Get the number of total processes.
  p = MPI::COMM_WORLD.Get_size();

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank();

  // Initialize availability depending on the phil's ID
  if (id == 0)
  {
    // 0th phil doesn't start with any forks :(
    rightAvailable = false;
    leftAvailable = false;
  }
  else if (id == p - 1)
  {
    // The last phil gets both forks
    rightAvailable = true;
    leftAvailable = true;
  }
  else
  {
    // Every other phil gets their left fork
    rightAvailable = false;
    leftAvailable = true;
  }

  //Safety check - need at least 2 philosophers to make sense
  if (p < 2)
  {
    MPI::Finalize();
    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;

  //setup message storage locations
  int msgIn, msgOut;
  int leftNeighbor = (id == 0 ? p - 1 : id - 1); // compute left neighbor by getting the previous phil's ID (includes wrap-around)
  int rightNeighbor = (id + 1) % p;

  pomerize P;

  string lFile = fileBase + to_string(leftNeighbor);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app);
  ofstream foutRight(rFile.c_str(), ios::out | ios::app);

  while (numWritten < MAXMESSAGES)
  {
    bool foundAny = false;

    // Generate our poems
    stanza1 = P.getLine();
    stanza2 = P.getLine();
    stanza3 = P.getLine();

    // Message Handler
    while (MPI::COMM_WORLD.Iprobe(MPI::ANY_SOURCE, tag))
    {
      foundAny = true;
      // cout << "Phil " << id << " is receiving a message..." << endl;
      MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status); // either neighbor
      source = status.Get_source();

      // cout << "Phil " << id << " received message " << msgIn << " from " << source << endl;

      if (source == leftNeighbor)
      {
        if (msgIn == EXIT)
        {
          // The phil to our left is finished writing. We no longer need to relinquish our left fork when we're finished,
          // so we say it's not available
          // cout << "Phil " << id << " got exit message from " << source << endl;
          leftAvailable = true;
          leftDone = true;
          leftUsed = false;
        }
        else if (msgIn == NOTIFY)
        {
          // Business as usual. We received notice that we can use our left fork
          // cout << "Phil " << id << " got left fork from " << source << endl;
          leftAvailable = true;
          leftUsed = false;
        }
        else
        {
          // cout << "Phil " << id << " got message from " << source << " requesting left fork" << endl;
          if (leftUsed && leftAvailable)
          {
            msgOut = NOTIFY;
            MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, leftNeighbor, tag);
            leftAvailable = leftUsed = false;
          }
        }
      }
      else if (source == rightNeighbor)
      {
        // cout << "Phil " << id << " received message " << msgIn << " from " << source << endl;
        if (msgIn == EXIT)
        {
          // The phil to our left is finished writing. We no longer need to relinquish our left fork when we're finished,
          // so we say it's not available
          // cout << "Phil " << id << " got exit message from " << source << endl;
          rightAvailable = true;
          rightDone = true;
          rightUsed = false;
        }
        else if (msgIn == NOTIFY)
        {
          // Business as usual. We received notice that we can use our left fork
          // cout << "Phil " << id << " got right fork from " << source << endl;
          rightAvailable = true;
          rightUsed = false;
        }
        else
        {
          // cout << "Phil " << id << " got message from " << source << " requesting fork" << endl;

          // Don't give up our fork if we haven't had a chance to use it
          if (rightUsed && rightAvailable)
          {
            msgOut = NOTIFY;
            MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, rightNeighbor, tag);
            rightAvailable = rightUsed = false;
          }
        }
      }
      else
      {
        cout << "Phil " << id << " received message " << msgIn << " from a non-neighbor source: " << source << endl;
      }
    }

    if (!foundAny)
    {
      // cout << "Phil " << id << " sees no messages!" << endl;

      // If we have both of our files, then we can write
      if (leftAvailable && rightAvailable)
      {
        cout << STARS << "Phil " << id << " is writing to " << lFile << " and " << rFile << " " << STARS << endl;

        // Begin writing
        foutLeft << stanza1 << endl;
        foutRight << stanza1 << endl;

        foutLeft << stanza2 << endl;
        foutRight << stanza2 << endl;

        foutLeft << stanza3 << endl << endl;
        foutRight << stanza3 << endl << endl;
        // End writing

        cout << STARS << "Phil " << id << " DONE is writing to " << lFile << " and " << rFile << " " << STARS << endl;

        numWritten++;

        // Phil had a chance to write to their files
        leftUsed = rightUsed = true;

        // leftAvailable = rightAvailable = true;

        // Notify our neighbors that we are done writing to our files or exiting
        if (numWritten == MAXMESSAGES)
        {
          // We want this to be blocking, so we can assure that our neighbors know we are exiting
          msgOut = EXIT;
          if (!rightDone)
            MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, rightNeighbor, tag);

          if (!leftDone)
            MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, leftNeighbor, tag);
        }
        else
        {
          msgOut = NOTIFY;
          if (!rightDone)
          {
            MPI::COMM_WORLD.Isend(&msgOut, 1, MPI::INT, rightNeighbor, tag);
            rightAvailable = rightUsed = false;
          }

          if (!leftDone)
          {
            MPI::COMM_WORLD.Isend(&msgOut, 1, MPI::INT, leftNeighbor, tag);
            leftAvailable = leftUsed = false;
          }
          // rightAvailable = false;
          // leftAvailable = false;

          // MPI::COMM_WORLD.Isend(&msgOut, 1, MPI::INT, rightNeighbor, tag);
          // rightAvailable = false;
          // MPI::COMM_WORLD.Isend(&msgOut, 1, MPI::INT, leftNeighbor, tag);
          // leftAvailable = false;
        }
      }
      else
      {
        if (!leftAvailable)
        {
          msgOut = REQUEST;
          // cout << "Phil " << id << " is requesting left" << endl;
          MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, leftNeighbor, tag);
          // cout << "Phil " << id << " sent request for left!" << endl;
        }

        if (!rightAvailable)
        {
          msgOut = REQUEST;
          // cout << "Phil " << id << " is requesting right" << endl;
          MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, rightNeighbor, tag);
          // cout << "Phil " << id << " sent request for right!" << endl;
        }
      }
      // cout << "Phil " << id << " sees no messages from Phil " << leftNeighbor << "!" << endl;
    }
  }

  foutLeft.close();
  foutRight.close();

  // sleep(p);

  cout << STARS << "Phil " << id << " is finalizing" << STARS << endl;

  //  Terminate MPI.
  MPI::Finalize();
  return 0;
}
