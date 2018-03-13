/**
 * Christian Parsons, William Lorey
 * CS3800 Section A
 * Dining Philosophers problem solution using message passing
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

// Message types
const unsigned int REQUEST = 0;
const unsigned int NOTIFY = 1;
const unsigned int EXIT = 2;

using namespace std;

// this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 20;

const string fileBase = "outFile";

void handleReceiveMessage(const int message, bool &fileAvailable)
{
  switch (message)
  {
    case EXIT:
      // Our neighbor finished writing. We no longer need to relinquish our fork
      // when we're finished, so we say it's not available
      fileAvailable = false;
      break;
    case NOTIFY:
      // Business as usual. We received notice that we can use our fork
      fileAvailable = true;
      break;
  }
  return;
}

void write(ofstream &foutLeft, ofstream &foutRight, pomerize &P)
{
  // construct poem & output stanzas into the files 'simultaneously'
  // we do this with an intermediate variable so both files contain the same poem!
  string stanza1, stanza2, stanza3;

  stanza1 = P.getLine();
  foutLeft << stanza1 << endl;
  foutRight << stanza1 << endl;

  stanza2 = P.getLine();
  foutLeft << stanza2 << endl;
  foutRight << stanza2 << endl;

  stanza3 = P.getLine();
  foutLeft << stanza3 << endl << endl;
  foutRight << stanza3 << endl << endl;
  return;
}

int main(int argc, char *argv[])
{
  int id; // my MPI ID
  int p;  // total MPI processes
  MPI::Status status;
  int tag = 1;
  int source;

  bool rightAvailable; // my right fork's (file's) availability
  bool leftAvailable;  // my left fork's availability
  bool used = true;    // Every fork is 'used' initially

  // Initialize MPI.
  MPI::Init(argc, argv);

  // Get the number of total processes.
  p = MPI::COMM_WORLD.Get_size();

  // Determine the rank of this process.
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

  // Safety check - need at least 2 philosophers to make sense
  if (p < 2)
  {
    MPI::Finalize();
    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
    return 1; // non-normal exit
  }

  srand(id + time(NULL)); // ensure different seeds...

  int numWritten = 0;

  // setup message storage locations
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
    if (!leftAvailable || !rightAvailable || !used)
    {

      // At this point, we can't write to our files. Ask for access

      if (!leftAvailable)
      {
        // Request left fork
        MPI::COMM_WORLD.Send(&REQUEST, 1, MPI::INT, leftNeighbor, tag);
        // Wait for response
        MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, leftNeighbor, tag, status);

        handleReceiveMessage(msgIn, leftAvailable);
      }

      if (!rightAvailable)
      {
        // Request right fork
        MPI::COMM_WORLD.Send(&REQUEST, 1, MPI::INT, rightNeighbor, tag);
        // Wait for response
        MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, rightNeighbor, tag, status);

        handleReceiveMessage(msgIn, rightAvailable);
      }
    }

    used = false; // We have not yet 'used' our forks

    write(foutLeft, foutRight, P);
    numWritten++;

    used = true; // We have 'used' our forks

    // Handle message requests
    while (leftAvailable || rightAvailable)
    {
      MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status);

      source = status.Get_source();
      if (source == leftNeighbor)
      {
        // We've given away our left fork
        leftAvailable = false;

        // Determine if we're finished sending messages or not
        msgOut = (numWritten == MAXMESSAGES ? EXIT : NOTIFY);

        // Inform our left neighbor that they can use the fork
        MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, leftNeighbor, tag);
      }

      else if (source == rightNeighbor)
      {
        rightAvailable = false;

        msgOut = (numWritten == MAXMESSAGES ? EXIT : NOTIFY);

        MPI::COMM_WORLD.Send(&msgOut, 1, MPI::INT, rightNeighbor, tag);
      }
    }
  }

  foutLeft.close();
  foutRight.close();

  //  Terminate MPI.
  MPI::Finalize();
  return 0;
}
