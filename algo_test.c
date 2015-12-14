/* NTUA ECE Algorithms Tester
 *
 * For every problem, Dr Fotakis releases test inputs and outputs
 * This tester aims to receive an executable and print out statistics
 * and the difference between your outputs and the correct ones
 *
 * -d directory
 *      The directory in which the testcases are stored. Default is "tests".
 *      In the directory the names should be input(whatever)-output(whatever).
 *      For example if there is a file named input_huge.txt, its output should
 *      be in output_huge.txt.
 *
 * -n number
 *      Number of testcases. The last testcases tend to be larger
 *      and you might want to avoid them while writing your program.
 *
 * -t seconds
 *      Seconds the tester will wait for each testcase. If the program run for
 *      at that time, it will be terminated and an appropriate message will be displayed
 *
 * Issues
 *      Because the program compares your output to the desired character by
 *      character, you should make sure you have the same line changes.
 *      On windows it is "\r\n" while on Unix it is plain "\n".
 *      In that case you can use the command
 *
 *      sed -i 's/\r//g' test_directory/output*
 *
 * Konstantinos Ameranis, 19/11/15
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BUFF_SIZE 65536

volatile sig_atomic_t timeout_flag = 0;

/* Gets calles when a program timeouts */
void handle_alarm(int sig) {
    if(sig == SIGALRM)
        timeout_flag = 1;
}

/* Makes sure the entry is a regular entry file */
int input_filter(const struct dirent *entry) {
    if(access(entry->d_name, R_OK) == -1)
        return 0;
    if(entry->d_type != DT_REG)
        return 0;
    return !strncmp(entry->d_name, "input", 5);
}


int main(int argc, char** argv) {
    char *previous_dir;
    char *test_dir = NULL;
    char *executable = NULL;
    char buffer[BUFF_SIZE];
    char buffer2[BUFF_SIZE];
    int cases = -1;
    int time = -1;
    int c;
    int good = 1;
    /* Check arguements */
    opterr = 0;
    while((c = getopt(argc, argv, "d:n:t:")) != -1) {
        switch(c) {
            case 'd':
                test_dir = optarg;
                break;
            case 'n':
                cases = atoi(optarg);
                break;
            case 't':
                time = atoi(optarg);
            case '?':
                if(optopt == 'd' || optopt == 'n' || optopt == 't')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
    }
    if(optind == argc) {
        fprintf(stderr, "You must provide an executable\n");
        return 1;
    }
    executable = strdup(argv[optind]);

    /* Check that the executable is good */
    struct stat statbuf;
    /* We can execute it */
    if(access(executable, X_OK) == -1) {
        fprintf(stderr, "%s: Executable does not exist or cannot be executed\n", executable);
        exit(1);
    }
    /* Executable is a regular file */
    if(stat(executable, &statbuf) == -1)
        return 0;
    if(!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "%s: Executable must be a regular file\n", executable);
        exit(1);
    }

    /* Check that the test directory is good */
    if(test_dir == NULL)
        test_dir = strdup("tests");
    /* We can read it */
    if(access(test_dir, R_OK) == -1) {
        fprintf(stderr, "%s: Can't open test directory\n", test_dir);
        exit(1);
    }
    /* Test directory is a directory */
    if(stat(test_dir, &statbuf) != 0)
        return 0;
    if(!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "%s: The tests should be in a directory\n", test_dir);
        exit(1);
    }
    /* Who has more than a hundred test cases??? */
    if(cases == -1)
        cases = 100;
    if(getcwd(buffer, sizeof(buffer)) == NULL) {
        perror("getcwd");
        exit(1);
    }
    /* We use alarm to kill long testcases */
    signal(SIGALRM, handle_alarm);  // Change to SIGACTION

    /* Get every input file */
    previous_dir = strdup(buffer);
    chdir(test_dir);
    struct dirent ** entries;

    int files = scandir(".", &entries, input_filter, versionsort);
    int i = 0;
    while (i < files && i < cases) {
        int read_fd = open(entries[i]->d_name, O_RDONLY);
        int fd[2];
        if(pipe(fd) == -1) {
            perror("pipe");
            exit(1);
        }
        pid_t p;
        int status;
        p = fork();
        if(p < 0) {
            perror("fork");
            exit(1);
        }
        if(p == 0) {
            close(0);
            dup(read_fd);
            close(read_fd);

            close(1);
            dup(fd[1]);
            close(fd[1]);
            close(fd[0]);
            chdir(previous_dir);
            char* arg[] = { executable, NULL };
            if(execvp(arg[0], arg) == -1) {
                perror("execvp");
                exit(1);
            }
        }
        close(fd[1]);
        strcpy(buffer, "output");
        strcpy(buffer + 6, (entries[i]->d_name) + 5);
        int out_fd = open(buffer, O_RDONLY);
        if(out_fd < 0) {
            perror(buffer);
            wait(&status);
            free(entries[i]);
            i++;
            good = 0;
            continue;
        }
        struct timespec start={0,0}, finish={0,0};
        clock_gettime(CLOCK_REALTIME, &start);
        if(time != -1)
            alarm(time);
        wait(&status);
        if(timeout_flag) {
            kill(p, SIGTERM);
            printf("%s:\tTestcase #%d timed out...\n", entries[i]->d_name, i+1);
            timeout_flag = 0;
            free(entries[i]);
            i++;
            good = 0;
            continue;
        }
        clock_gettime(CLOCK_REALTIME, &finish);

        if(status % 256 != 0) {
            fprintf(stderr, "%d ", status);
            fprintf(stderr, "Input %s finished abruptly\n", entries[i]->d_name);
            good = 0;
        }
        int count = 0, r = 0;
        while((count = read(out_fd, buffer, BUFF_SIZE)) != 0) {
            buffer[count] = '\0';
            r = read(fd[0], buffer2, BUFF_SIZE);
            buffer2[r] = '\0';
            if(strcmp(buffer, buffer2) != 0) {
                printf("Test #%d with input file %s failed\n", i, entries[i]->d_name);
                printf("Your output:\t%s", buffer2);
                printf("Correct output:\t%s", buffer);
                printf("Testcase #%d completed in: %lfs\n\n", i+1,
                        ((double)finish.tv_sec + 1.0e-9*finish.tv_nsec) -
                        ((double)start.tv_sec + 1.0e-9*start.tv_nsec));
                good = 0;
            }
            //if(strncmp(buffer, buffer2, strlen(buffer) - 2) == 0 && r == count) {
            else {
                printf("%s:\tTestcase #%d completed in: %lfs\n", entries[i]->d_name, i+1,
                        ((double)finish.tv_sec + 1.0e-9*finish.tv_nsec) -
                        ((double)start.tv_sec + 1.0e-9*start.tv_nsec));
            }
            free(entries[i]);
            i++;
        }
        }
        chdir(previous_dir);
        if(i > 0 && good)
            printf("All your outputs are correct\n");
        else if(i == 0)
            printf("No inputs found in directory %s\n", test_dir);
        return 0;
    }
