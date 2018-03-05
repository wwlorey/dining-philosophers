#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

//run compiled code (for 5 Phils) with mpirun -n 5 program

// Constant declarations
const unsigned int REQUEST = 0;
const unsigned int NOTIFY = 1;

using namespace std;

//this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 3;//10; 

//if you change this base, update the Makefile "clean" accordingly to remove old poems
const string fileBase = "outFile"; 

int main ( int argc, char *argv[] ) 
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  int tag = 1;

  bool rightAvailable;
  bool leftAvailable;
  bool used = true; // Every file (fork) is 'used' initially


  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of total processes.
  p = MPI::COMM_WORLD.Get_size ( );

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank ( );
  
  // Initialize availability 
  if (id == 0) {
    // 0th process doesn't start with any forks :(
    rightAvailable = false;
    leftAvailable = false;
  } else if (id == p - 1) {
    // The last process gets both forks
    rightAvailable = true;
    leftAvailable = true;
  } else {
    // Every other process gets the left fork
    rightAvailable = false;
    leftAvailable = true;
  }

  //Safety check - need at least 2 philosophers to make sense
  if (p < 2) {
    MPI::Finalize ( );
    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;
  
  //setup message storage locations
  int msgIn, msgOut;
  int leftNeighbor = (id == 0 ? p - 1 : id - 1);
  int rightNeighbor = (id + 1) % p;

  pomerize P;

  string lFile = fileBase + to_string(leftNeighbor);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );

  while (numWritten < MAXMESSAGES) {

    /*
    //send 1 test message to each neighbor
    msgOut = rand() % p; //pick a number/message
    MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag ); 
    msgOut = rand() % p; //pick a new number/message
    MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag ); 
        
    //receive 1 test message from each neighbor
    MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status );
    std::cout << "Receiving message " << msgIn << " from Philosopher ";
    std::cout << status.Get_source() << std::endl;

    MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status );
    std::cout << "Receiving message " << msgIn << " from Philosopher ";
    std::cout << status.Get_source() << std::endl;	

    //LET'S JUST IGNORE THE MESSAGES AND ASSUME IT'S SAFE TO WRITE TO THE FILE!
    //std::cout << "ID: " << id << " CARELESSLY writing to " << lFile << " and " << rFile << endl;
    //If you want to see correct poems, change MAXMESSAGES to something VERY small and add this sleep
    //sleep(id); //will delay each process so the initial interleaving(s) will likely be OK
    */

    if (!(leftAvailable && rightAvailable && used)) {
      if (!leftAvailable) {
        // Request left fork
        cout << "Phil " << id << " is requesting left" << endl << endl;
        MPI::COMM_WORLD.Send(&REQUEST, 1, MPI::INT, leftNeighbor, tag); 
        // Wait for response
        cout << "Phil " << id << " waiting for left" << endl << endl;
        MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, leftNeighbor, tag, status);
        cout << "Phil " << id << " got left" << endl << endl;
        // if (msgIn == NOTIFY)
          leftAvailable = true;
      }

      if (!rightAvailable) {
        // Request right fork
        cout << "Phil " << id << " is requesting right" << endl << endl;
        MPI::COMM_WORLD.Send(&REQUEST, 1, MPI::INT, rightNeighbor, tag); 
        // Wait for response
        cout << "Phil " << id << " waiting for right" << endl << endl;
        MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, rightNeighbor, tag, status);

        cout << "Phil " << id << " got right" << endl << endl;

        // if (msgIn == NOTIFY)
          rightAvailable = true;
      }
    }

    used = false;
  
    /* WRITE */
    //construct poem & output stanzas into the files 'simultaneously'
    //we do this with an intermediate variable so both files contain the same poem!
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
    numWritten++;
    cout << "************Phil " << id << " has written " << numWritten << " times" << endl << endl;
    /* END WRITE */

    used = true;
    
    // Handle message requests
    while (leftAvailable || rightAvailable) {
      cout << "Phil " << id << " is ready to relinquish control " << endl << endl;
      MPI::COMM_WORLD.Recv(&msgIn, 1, MPI::INT, MPI::ANY_SOURCE, tag, status);
      cout << "Phil " << id << " is ready to relinquish control with Source: " << status.Get_source() << ", Msg: " << msgIn << endl << endl;

      if (status.Get_source() == leftNeighbor) {
        leftAvailable = false;
        cout << "Phil " << id << " relinquishing left..." << endl << endl;
        MPI::COMM_WORLD.Send(&NOTIFY, 1, MPI::INT, leftNeighbor, tag); 
        cout << "Phil " << id << " relinquished left!" << endl << endl;
      }
      else if (status.Get_source() == rightNeighbor) {
        rightAvailable = false;

        cout << "Phil " << id << " relinquishing right..." << endl << endl;
        MPI::COMM_WORLD.Send(&NOTIFY, 1, MPI::INT, rightNeighbor, tag); 
        cout << "Phil " << id << " relinquished right!" << endl << endl;
      }
    }

  }

  // Send done message to neighbors


  foutLeft.close();
  foutRight.close();
  
  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}
