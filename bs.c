#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

    pid_t child_a, child_b;
    int pipe_a[2],pipe_b[2];
    char mesazhi1[] = "First message";
    char mesazhi2[] = "Second message";

    char buf[50];

    int first_pipe = pipe(pipe_a);
    int second_pipe = pipe(pipe_b);

    if(first_pipe == -1 || second_pipe == -1 ){
        perror("pipe");
        exit(1);
    }

    child_a = fork();

    if (child_a == 0) {
        /* Child A code */

        printf("%s\n","the first child is writing to pipe a" );
        write(pipe_a[1],mesazhi1, strlen(mesazhi1) + 1);
        write(pipe_a[1],mesazhi2, strlen(mesazhi2) + 1);

        read(pipe_b[0], buf, strlen(mesazhi1) + 1);
        printf("Reading from buffer for child 1 gives:  %s \n",buf);        
        read(pipe_b[0], buf, strlen(mesazhi2) + 1);
        printf("Reading from buffer for child 1 gives:  %s \n",buf);        

    } else {
        child_b = fork();

        if (child_b == 0) {
            /* Child B code */

            printf("%s\n","the second child is writing to pipe b" );


            read(pipe_a[0],buf, strlen(mesazhi1) + 1);
            printf("Reading from buffer for child 2 gives:  %s \n",buf);            
            read(pipe_a[0],buf, strlen(mesazhi2) + 1);
            printf("Reading from buffer for child 2 gives:  %s \n",buf);            

            write(pipe_b[1],mesazhi1, strlen(mesazhi1) + 1);
            write(pipe_b[1],mesazhi2, strlen(mesazhi2) + 1);


            printf("%s\n","the second child reads data from pipe a" );


        } else {
            /* Parent Code */

            int returnStatusA,returnStatusB;    
            waitpid(child_a, &returnStatusA, 0);  // Parent process waits here for child to terminate.
            waitpid(child_b, &returnStatusB, 0);  // Parent process waits here for child to terminate.


            if (returnStatusA == 0 && returnStatusB == 0)  // Verify child process terminated without error.  
            {
               printf("%s\n", "The child processes terminated normally.\n"); 
            }

            if (returnStatusA == 1 && returnStatusB == 1)      
            {
               printf("%s\n", "The child processes terminated with an error!. \n" );    
            }



            printf("%s\n","The parent terminates two childs");
        }
    }
}

//end