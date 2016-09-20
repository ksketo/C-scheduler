#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 20                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */
#define SHELL_EXECUTABLE_NAME "shell" /* executable for shell */


typedef struct node
{
int ID;
int Pid;
int prior;
char *priorName;
struct node *next;
struct node *pre;
}pro;

int status,stoped=0,first=0,mainpid,shellPid,idsf=1;
pro *head=NULL, *cur, *temp = NULL,*print,*low=NULL,*high=NULL;


/* Print a list of all tasks currently being scheduled.  */
static void
sched_print_tasks(void)
{
	cur=low;
	if (low!=NULL)
	{
		printf("\nH lista Diergasiwn (Low):\n");
		do
		{
			printf("ID:%d Pid-> %d Priority:%s",cur->ID,cur->Pid,cur->priorName);
			printf(", with next: %d and pre: %d\n",cur->next->Pid,cur->pre->Pid);
			cur = cur->next;
		}while(cur != low);
		printf("\n");
	}
	cur=high;
	if (high!=NULL)
	{
		printf("\nH lista Diergasiwn (High):\n");
		do
		{
			printf("ID:%d Pid-> %d Priority:%s",cur->ID,cur->Pid,cur->priorName);
			printf(", with next: %d and pre: %d\n",cur->next->Pid,cur->pre->Pid);
			cur = cur->next;
		}while(cur != high);
		printf("\n");
	}
	
	printf("H Trexousa diergasia: ID = %d , PID = % d !\n\n",temp->ID,temp->Pid); 
	
}

/* Send SIGKILL to a task determined by the value of its
 * scheduler-specific id.
 */
static int
sched_kill_task_by_id(int id)
{	
	int found=0;
	pro *deltemp;
	cur = low;
  if (low != 0)
  {
	do
	{	
		if (cur->ID == id) 
		{
			if (cur->ID == 1) {head = head->next; stoped =1;} 
			found=1;
			cur->pre->next = cur->next;
			cur->next->pre = cur->pre;
			if (low == cur) low = cur->next;
			deltemp = cur;
			cur = cur->next;
			first = 0;
			kill(deltemp->Pid,SIGKILL);
			free(deltemp);
		}
		else cur = cur->next;
	}
	while((cur != low) && (found != 1));
   }
   if(found == 0) printf("De vrethike i Low diergasia me ID %d !\n",id);
   

  cur = high;
  if (high != 0)
  {
	do
	{	
		if (cur->ID == id) 
		{
			if (cur->ID == 1) {head = head->next; stoped =1;}
			found=1;
			cur->pre->next = cur->next;
			cur->next->pre = cur->pre;
			if (high == cur) high = cur->next;
			deltemp = cur;
			cur = cur->next;
			first = 0;
			kill(deltemp->Pid,SIGKILL);
			free(deltemp);
		}
		else cur = cur->next;
	}
	while((cur != high) && (found != 1));
   }
   if(found == 0) printf("De vrethike i High diergasia me ID %d !\n",id);
	return -ENOSYS;
}


/* Create a new task.  */
static void
sched_create_task(char *executable)
{
	char *newargv[] = { NULL,NULL,NULL,NULL };
	char *newenviron[] = { NULL };
	idsf++;
	printf("Ftiaxnw diergasia me id %d kai name: %s\n",idsf,executable);
	cur=malloc(sizeof(pro));
	cur->ID = idsf;
	if (low != NULL) 
	{
		cur->pre = low->pre;
		cur->next = low;
		low->pre = cur;
		cur->pre->next = cur;
	}
	else
	{
		low= cur;
		cur->next = cur;
		cur->pre = cur;
	}
	cur->prior = 0;
	cur->priorName = "low";
	first = 0;
	cur->Pid = fork();
			
	if (cur->Pid< 0) {
		perror("parent: fork");
		exit(1);
	}
	if (cur->Pid == 0) {
		// Child 
		first=0;
		raise(SIGSTOP);
		newargv[0]=executable;
		execve(executable,newargv,newenviron);	
		exit(0);
	}	
}

void sched_high_task(int id)
{
	int found=0;
	cur = low;
	do
	{	
		if (cur->ID == id) 
		{
			found = 1;
			if (cur->next != cur)
			{
				cur->pre->next = cur->next;
				cur->next->pre = cur->pre;
				if (low == cur) low = cur->next;
			}
			else
			{
				low=NULL;
			}
			cur->prior = 1;
			cur->priorName = "high";
			if (high==NULL) 
			{
				high=cur;
				cur->next = cur;
				cur->pre = cur;
			}
			else
			{
				cur->pre = high->pre;
				cur->next = high;
				high->pre = cur;
				cur->pre->next = cur;
			}	
		}
		else cur = cur->next;
	}
	while((cur != low) && (found != 1));	
	if(found == 0) 
	{
		printf("De vrethike low diergasia me id:%d !",id);
	}
}

void sched_low_task(int id)
{
	int found=0;
	cur = high;
   if (high!=NULL)
   {
	do
	{	
		if (cur->ID == id) 
		{
			found = 1;
			if (cur->next != cur)
			{
				cur->pre->next = cur->next;
				cur->next->pre = cur->pre;
				if (high == cur) high = cur->next;
			}
			else
			{
				high=NULL;
			}
			cur->prior = 0;
			cur->priorName = "low";
			if (low==NULL) 
			{
				low=cur;
				low->next = low;
				low->pre = low;
			}
			else
			{
				cur->pre =low->pre;
				cur->next = low;
				low->pre = cur;
				cur->pre->next = cur;
			}	
		}
		else cur = cur->next;
	}
	while((cur != high) && (found != 1));	
   }
   if(found == 0) printf("De vrethike high diergasia me id:%d !",id);
}

/* Process requests by the shell.  */
static int
process_request(struct request_struct *rq)
{
	switch (rq->request_no) {
		case REQ_PRINT_TASKS:
			sched_print_tasks();
			return 0;

		case REQ_KILL_TASK:
			return sched_kill_task_by_id(rq->task_arg);

		case REQ_EXEC_TASK:
			sched_create_task(rq->exec_task_arg);
			return 0;
		case REQ_HIGH_TASK:
			sched_high_task(rq->task_arg);
			return 0;
		case REQ_LOW_TASK:
			sched_low_task(rq->task_arg);
			return 0;
		case REQ_KILL_SHELL:
			return sched_kill_task_by_id(1);

		default: return -ENOSYS;
	}
}

/* SIGALRM handler: Gets called whenever an alarm goes off.
 * The time quantum of the currently executing process has expired,
 * so send it a SIGSTOP. The SIGCHLD handler will take care of
 * activating the next in line.
 */
static void
sigalrm_handler(int signum)
{
	kill(temp->Pid,SIGSTOP);
	stoped = 1;
}

/* SIGCHLD handler: Gets called whenever a process is stopped,
 * terminated due to a signal, or exits gracefully.
 *
 * If the currently executing task has been stopped,
 * it means its time quantum has expired and a new one has
 * to be activated.
 */
static void
sigchld_handler(int signum)
{
	pro *deltemp;

	//if(stoped == 1) printf("Wp, kapoio paidi stamatise");

	if (first == 0) first++;
	else if (first == 1)
	{
		first--;		


		if (stoped == 0)
		{
			printf("~~~ H diergasia %d (%s) teliwse ti bgazw apo ti lista\n",temp->ID,temp->priorName);
			if ((temp->prior == 0) && ( high != NULL))
			{
				low = temp;
				temp= high->pre;	
			}
			if (temp->next == temp)
			{
				if(temp->prior == 0)
				{
					printf("\n\n~~~ Teliwsan oles oi diergasies!!!!!\n");
					kill(mainpid,SIGKILL);
				}
				else
				{
					if (low == NULL) 
					{
						printf("\n\n~~~ Teliwsan oles oi diergasies!!!!!\n");
						kill(mainpid,SIGKILL);
					}
					else
					{
						high=NULL;
						temp =low;
					}
				}
			}
			else
			{
				temp->pre->next = temp->next;
				temp->next->pre = temp->pre;
				if (low == temp) low = temp->next;
				if (high == temp) high = temp->next;
				deltemp = temp;
				temp = temp->next;
				free(deltemp);
			}
		}
		else
		{
			printf("~~~ Telos xronou gia ti diergasia %d (%s) \n",temp->ID,temp->priorName);
			stoped = 0;
			temp = temp->next;
			if ((temp->prior == 0) && ( high != NULL))
			{
				low = temp;
				temp= high->pre;	
			}	
		}
		printf("\n~~~ Ksekinaw ti diergasia %d (%s) \n",temp->ID,temp->priorName);
		kill(temp->Pid,SIGCONT);
		alarm(SCHED_TQ_SEC);
	}
}

/* Disable delivery of SIGALRM and SIGCHLD. */
static void
signals_disable(void)
{
	sigset_t sigset;

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0) {
		perror("signals_disable: sigprocmask");
		exit(1);
	}
}

/* Enable delivery of SIGALRM and SIGCHLD.  */
static void
signals_enable(void)
{
	sigset_t sigset;
	kill(head->Pid, SIGCONT);
	temp = head;
	alarm(SCHED_TQ_SEC);

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGALRM);
	sigaddset(&sigset, SIGCHLD);
	if (sigprocmask(SIG_UNBLOCK, &sigset, NULL) < 0) {
		perror("signals_enable: sigprocmask");
		exit(1);
	}
}


/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;cur=malloc(sizeof(pro));
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

static void
do_shell(char *executable, int wfd, int rfd)
{
	char arg1[10], arg2[10];
	char *newargv[] = { executable, NULL, NULL, NULL };
	char *newenviron[] = { NULL };

	sprintf(arg1, "%05d", wfd);
	sprintf(arg2, "%05d", rfd);
	newargv[1] = arg1;
	newargv[2] = arg2;

	raise(SIGSTOP);
	execve(executable, newargv, newenviron);

	/* execve() only returns on error */
	perror("scheduler: child: execve");
	exit(1);
}

/* Create a new shell task.
 *
 * The shell gets special treatment:
 * two pipes are created for communication and passed
 * as command-line arguments to the executable.
 */
static void
sched_create_shell(char *executable, int *request_fd, int *return_fd)
{
	pid_t p;
	int pfds_rq[2], pfds_ret[2];

	if (pipe(pfds_rq) < 0 || pipe(pfds_ret) < 0) {
		perror("pipe");
		exit(1);
	}

	p = fork();
	shellPid = p;
	if (p < 0) {
		perror("scheduler: fork");
		exit(1);
	}

	if (p == 0) {
		/* Child */
		close(pfds_rq[0]);
		close(pfds_ret[1]);
		do_shell(executable, pfds_rq[1], pfds_ret[0]);
		assert(0);
	}
	/* Parent */
	close(pfds_rq[1]);
	close(pfds_ret[0]);
	*request_fd = pfds_rq[0];
	*return_fd = pfds_ret[1];
}

static void
shell_request_loop(int request_fd, int return_fd)
{
	int ret;
	struct request_struct rq;

	/*
	 * Keep receiving requests from the shell.
	 */
	for (;;) {
		if (head->ID != 1) break;
		if (read(request_fd, &rq, sizeof(rq)) != sizeof(rq)) {
			perror("scheduler: read from shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}

		signals_disable();
		ret = process_request(&rq);
		signals_enable();
		if (head->ID != 1) break;
		if (write(return_fd, &ret, sizeof(ret)) != sizeof(ret)) {
			perror("scheduler: write to shell");
			fprintf(stderr, "Scheduler: giving up on shell request processing.\n");
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	mainpid = getpid();
	
	/* Two file descriptors for communication with the shell */
	static int request_fd, return_fd;

	/* Create the shell. */
	sched_create_shell(SHELL_EXECUTABLE_NAME, &request_fd, &return_fd);
	
	head=malloc(sizeof(pro));
	head->next = head;
	head->pre = head;
	head->ID = 1;
	head->Pid = shellPid;
	head->prior = 0;
	head->priorName="low";
	low = head;
	

	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();


	kill(head->Pid, SIGCONT);
	temp = head;
	alarm(SCHED_TQ_SEC);	
	
	shell_request_loop(request_fd, return_fd);

	/* Now that the shell is gone, just loop forever
	 * until we exit from inside a signal handler.
	 */
	while (pause())
		;

	/* Unreachable */
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
