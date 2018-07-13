# Unix process management

Creation of a new process in unix like system uses a _fork_ and _exec_ model.

* **fork**: Creates a new process by duplicating the calling process, wherein the new process is called a `child` and the
calling process is called `parent`. Both of these process have separate memory spaces but with the same contents due to
duplication. For more information on fork `man 2 fork`

* **exec**: exec actually is a family of functions like `execve` `execv` etc. These family function differs in how they
interpret their arguments. An exec function replaces the current process image with a new process image, that is to say
it modifies a process to run a new process. For more information `man 3 exec`

Together fork and exec consists the process creation pipeline in Unix like system.

## **Some Pointers**

### fork

* Creates and initializes a process control block(Each process have a process control block) in the kernel
* Creates a new address space for the new process to run.
* Initializes the new address space with a copy of parent address space.
* Inherit execution context from the parent.
* Informs the scheduler that a new process is ready to be run.

To distinguish between the parent and the child process fork return value is used, for the parent it is the process ID
of the child(which will surely be different from parent) and to child it returns 0(signifying success).

> Chrome for unix like system relies heavily on this fork and exec model, each tab that is opened in chrome is a unique
process spwanned using fork and exec, which is to say each tab has its own process image and address space.

* Under linux fork is implemented using copy on write, so the only penalty incurred by fork is the time and memory required
to duplicate the parents page table and create unique task strucutre for the child.

There are some embedded devices which does not have a MMU, fork cannot be implemented on such devices. The fuction `vfork`
is used for them. It is a special case of clone, which is used to create a new process without calling the page tables of the
parent process(so it gives better performance than fork). For more information `man 2 vfork`

### exec

* Loads a new program into current address space.
* Copy arguments to this address space.
* Initializes hardware context to start execution at the `start`.

### clone 

fork itself is implemented using the clone system call, similar to fork it creates a new process but unlike fork it allows the 
child process to share parts of its execution context with the parent process. It is more commonly used to implement threads, wherein
the created child shares the memory and the address space but have different register and kernel states.

clone again comes in two flavours one a system call and other wrapper from glibc library. The copy is optimised using the copy
on write semantics.

## MAN page

* fork
* exec
* vfork
* posix_spawn
* dup, dup2
* clone
