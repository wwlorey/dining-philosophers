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
and `rightAvailable` as well as the booleans `leftUsed` and `rightUsed`). A phil first generates its messages and then checks to see if it has any messages. These messages include whether a neighbor has exited, a neighbor has given up a fork, or whether a neighbor is giving up a fork. Then, if the phil still has access to both forks for writing, it will write its messages (marking that the left fork and right fork are 'used' immediately after writing its first message), checking to see if any other phil wants to use the current phil's forks. If another phil wants to use the current phil's forks, the current phil will only give them up if they have 'used' their forks and they are available. Using the 'used' check helps prevent phil starvation.

If a phil wished to write but did *not* have access to both forks, it would request the fork(s) it needed from its neighbor(s) and block until
the neighbors fulfilled the request. To prevent deadlock, phils were not allowed to `hold and wait` with their forks (i.e. a phil would not take one fork
and continue without picking up the other fork).
Once a phil had written all of its desired number of lines to the files (forks), it would send
an exit command to the neighbor(s) requesting the fork(s) they were using, so the neighbor(s) knew that they would no longer have to relinquish control
of said fork(s) when they finished writing.
