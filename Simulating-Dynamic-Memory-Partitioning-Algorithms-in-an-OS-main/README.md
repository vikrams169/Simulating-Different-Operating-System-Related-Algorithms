# Simulating-Dynamic-Memory-Partitioning-Algorithms-in-an-OS
A C program to compare the performance of first-fit, best-fit, and next-fit algorithms while using dynamic memory partitioning in an OS.<br>
<h4>What does the program do?</h4>
The program simulates three different dynamic memory partitioning algorithms (best-fit, first-fit, and next-fit), and compares their performance based on the average memory utilization and turnaround time. The program works based on a user specified physical memory and OS allocated space size. The user also specifies arguments which directly limit the randomly generated size, process duration and arrival rate of different processes. After a user specified program running time (usually large enough to reach steady state), the program terminates and shows the average memory utilization and turnaround time, after which the program can be run using similar parameters on the algorithms to compare performance as required.<br>
<h4>Working of the Program</h4>
<ul><li>Initially, the user specifies a couple command line arguments which govern the size of the physical memory, space to be allocated to the OS, process arrival rate, limits on the sizes and running durations of randomly generated processes, the dynamic memory partitioning algorithm to use out of the three (best-fit, first-fit, and next-fit), and the total time the program will run.
<li>The physical memory is maintained as a linked list, with each node representing a block, either allocated to a process, or empty, having a size, start and end memory location, and other features.
<li>Two other lists, namely the <i>pending processes</i> and <i>allocated processes</i> are maintained as well. When a new process is spawned, it is by default added to the <i>pending processes</i> list, after which all the processes in the <i>allocated processes</i> list (having processes that have already been allocated space in the physical memory and are running their duration) is scanned, and processes which have completed their running duration are removed. After that, all the processes in the <i>pending processes</i> list are checked to see if they can be placed in the physical memory (by the algorithm specified by the user), and if it can, that process is removed from the <i>pending processes</i> list, and added to the <i>allocated processes</i> list.
<li>After time out occurs based on the user specified running time, the program reports the average memory utilization and turnaround time to compare performance on running the program again with different parameters.
<li>By running the program for 120 seconds (approximately good enough for steady state) each time, observations have been noted for each of the algorithms to compare the performance of the different algorithms.</ul>
<h4>Comparing the Performance of the Different Algorithms</h4>
From the trends seen in the tables filled above, the performance of the three algorithms (based off both the memory utilization and average turnaround time) can be generalised (under the limits of experimental error) as:

    First Fit > Next Fit > Best Fit
<ul><li>The reason why the best-fit algorithm performs the worst is that it often leaves small chunks of memory unutilized (high external fragmentation) and thus leaves those bits unused for long periods thus harming both the memory utilization and average turnaround time. Between the first-fit and the next-fit algorithms, first-fit performs better, again intuitively since it has a larger range to scan the physical memory from, instead of just from the last allocated block.
<li>On increasing the value of ‘p’, and thus increasing the size of the physical memory, there's more space for more processes to be allocated (considering that the process size remains in the same range), thus increasing the memory utilization. The average turnaround time would decrease as well since most allocations can be done in a fewer number of scans.
<li>On increasing ‘n’, thus increasing the process arrival rate, the memory utilization would obviously increase, since there would naturally be more processes waiting to get allocated as soon as any space frees up. At the same time, the average turnaround time would also increase since more processes waiting at one point ensures that there would be more waiting between process creation and allocation due to mutual competition.
<li>On increasing ‘m’ or the process size (indirectly), memory utilization would intuitively decrease since larger processes compete for the memory allocation, often leaving many chunks unutilized (high external fragmentation). On the other hand, the average turnaround time would increase since the probability for a larger sized process to find a suitable block of memory available for use in a fewer number of scans is unlikely.
<li>On increasing the value of ‘t’, thus also increasing the process duration, the memory utilization has observed to more or less increase. Though there isn’t any hard reason to support this, we can imagine that if process duration on an average increases, then for a fixed process arrival rate, there would likely be more processes in the pending queue, thus ensuring a faster allocation after deallocating the current process. The same intuition can be used to explain the higher turnaround time (process arrival rate remains fixed, thus increasing the time between process creation and allocation).</ul>
<h4>How do you compile and run the program?</h4>
To compile the code, a command of the following structure can be run:

    gcc main.c -o main.out -pthread
And then to run the code, run the command:

    ./main.out [p] [q] [n] [m] [n] [t] [a] [N] [T]
p: Capacity of Physical Memory (in MB)<br><br>
q: Space Occupied by the OS in the Physical Memory in MB (under the condition q < 0.2*p)<br><br>
n: Used to Characterize the Process Arrival Rate (0.1*n < PAR < 1.2*n)<br><br>
m: Used to Characterize the Size of the Processes (0.5*m < SoP < 3*m, in multiples of 10 MB)<br><br>
t: Used to Characterize the Duration of the Processes (0.5*t  < DoP < 6*t, in multiples of 5 minutes)<br><br>
a: Used to Characterize the Algorithm to be Used (0 -> first-fit, 1 -> best-fit, 2 -> next-fit)<br><br>
N: Max Size of the Pending Queue<br><br>
N: Max Size of the Pending Queue<br><br>
T: Overall time the program will run (in seconds). Note: Should be long enough to achieve steady state<br><br>
An example of a command to run the code with arbitrary arguments would look like:

    ./main.out 1000 200 10 10 10 0 10 60
