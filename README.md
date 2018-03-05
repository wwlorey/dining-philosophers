# Dining (writing) Philosophers

*Contributers: Christian Parsons & William Lorey*

### Assignment Overview

For this project, we were given [this code](https://github.com/wwlorey/dining-philosophers/commit/5cd92812890c10e140840067245581f4c8d65ea7#diff-57049a8364d01c5163dc3cc8a9491cb3R1)
and told to tweak the algorithm by using `MPI` (message passing interface) so that all philosophers (`phils`) could write to their shared files as many times as they wanted without corrupting 
the files or causing deadlock. The baseline C++ code provided a basic dining phils infrastructure with message passing examples and 
file writing functionality.

Note: In order to write to their files (or *forks* as we called them, to adhere to the classic dining phils problem), the phils had to have access
to **both** of their files.

### Our Solution

As documented in the main C++ file, our approach was to employ the `dirty / clean fork` paradigm (represented by the booleans `leftAvailable`
and `rightAvailable` as well as the boolean `used`). A phil would first check if it had access to both of its forks before writing and subsequently relinquishing control of the forks.
A phil could only write if the fork(s) had been used, aiding in the prevention of starvation.
If a phil wished to write but did *not* have access to both forks, it would request the fork(s) it needed from its neighbor(s) and block until
the neighbors fufilled the request. To prevent deadlock, phils were not alloud to `hold and wait` with their forks (i.e. a phil would not take one fork
and continue without picking up the other fork). When a phil finished writing, it would relinquish control of its forks to the waiting phils. 
Once a phil had written all of its desired number of lines to the files (forks), it would send
an exit command to the neighbor(s) requesting the fork(s) they were using, so the neighbor(s) knew that they would no longer have to relinquish control
of said fork(s) when they finished writing.
