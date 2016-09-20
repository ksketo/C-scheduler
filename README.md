# C-scheduler

A simple round-robin scheduler in C. The scheduler is controlled through a shell program (shell.c).

### Shell commands

- **p** : prints a list of the running processes
- **k** : reads the id of a running process and terminates it
- **e** : reads the name of an executable (e.g. prog) and the scheduler creates a new process for that executable
- **q** : terminate shell
- **h(l)** : reads the id of a running process and sets its priority to high (or low)
