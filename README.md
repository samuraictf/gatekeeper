# Gatekeeper

Gatekeeper is C program that will do network redirection, filtering, and sandboxing for local services. In the context of Defcon CTF and xinetd services the intent is to have gatekeeper replace a give service binary. Gatekeeper will then be in the middle of all network communications and in control of the environment that runs the service binary.

## Redirection

Here is a simple example running bash:
    $ ./gatekeeper -l stdio -r stdio:/bin/bash
    echo test
    test

In this example the -l argument specifies where gatekeeper will listen and the -r argument specifies where gatekeeper will redirect. We are using stdio for both listen and redirect arguments here just like we would with an xinetd service. The redirect argument requires a process to exec in the stdio case. In Defcon CTF this would be the renamed service binary. We can also make gatekeeper listend :

    $ ./gatekeeper -l tcpipv4:1234 -r stdio:/bin/bash &
    TCP 0.0.0.0:1234
    $ nc localhost 1234
    echo foo
    foo

Here we've opened up a new tcp port on 1234 locally and redirected to /bin/bash. The same can be done for ipv6 with tcpipv6:1234 or udp with udpipv4:1234. You can also specify an local interface ip to listen on as tcpipv4:192.168.1.1:1234 etc. For CTF's we may run into services that implement their own network service code. We can still use gatekeeper in this case by patching the port number of the service, telling gatekeeper to listen on the original service port, and then redirecting to the new service port. For example:

    $ nc -l 1236 &
    $ ./gatekeeper -l tcpipv4:1234 -r tcpipv4:127.0.0.1:1236 &
    TCP 0.0.0.0:1234
    $ nc localhost 1234
    TCP 127.0.0.1:1236
    hello
    hello

In this example we started a netcat listener on port 1236 then started gatekeeper to listen on tcp port 1234 and redirect to tcpport 1236 on localhost. Gatekeeper logged the socket open and connection with TCP:xxx messages after we connected to port 1234 with netcat. Then we typed hello and the background netcat process on port 1236 output hello.

Now you have a basic sense of the redirection capabilities of gatekeeper. These can be mixed and matched much like you could do with socat. When used in stdio mode or with an xinetd service the logging can create a bit of an issue because our stdio fd's are all redirected to a networt sockets. To solve this gatekeeper can emit log messages to an external server. For example:

    $ nc -l -u 2090 &
    $ ./gatekeeper -l tcpipv4:1234 -r stdio:/bin/bash -o 127.0.0.1:2090
    TCP 0.0.0.0:1234
    Unable to bind socket

Here we started a netcat listener on udp port 2090 and then attempted to start gatekeeper listening on tcpipv4 port 1234 redirecting to /bin/bash just like in our first example. Only this time there was a problem. Gatekeeper couldn't bind to port 1234. Something was already listening there. Indeed the process from our previous example was not killed and the new gatekeeper process was unable to bind to port 1234. This remote logging feature is useful for tracking errors and other connection information on a live system. 

These parameters can be configured via environment variables as well:
    GK_LISTENSTR=tcpipv4:1234 GK_REDIRECTSTR=stdio:/bin/bash GK_LOGSRV=127.0.0.1:2090 ./gatekeeper

## Local Defenses

### Alarm

Let's talk about some of the sandboxing capabilities. The most basic capability is to set an alarm on the gatekeeper process. This will kill the redirection after x number of seconds and help to automatically drop long standing connections. For example:

    $ ./gatekeeper -l stdio -r stdio:/bin/bash -a 3
    echo test
    test
    (3 seconds pass)
    Alarm clock

### Rlimit

We can also limit disk access with an RLIMIT_FSIZE setting and umask(0777) call before the exec. For example:
    
    $ ./gatekeeper -l stdio -r stdio:/bin/bash -d
    Setting RLIMIT_FSIZE
    touch ttt
    ls -la ttt
    ---------- 1 bool bool 0 Jun 28 11:01 ttt
    echo foo>bar
    $ 
    $ ls -la bar
    ---------- 1 bool bool 0 Jun 28 11:02 bar

Here we pass in the -d flag to limit disk access. When we touch a file we can see that the default permissions are 000 due to the umask and that when we try to write any data to a file the gatekeeper process terminated booting us back to a shell. The file was still created but again it has no permissions and the file size is 0. This is useful for stopping cron jobs or other backdoors from being deployed on our system. However, CTF process may require access to disk so we should only use this option if we are sure that a process will not need disk access.

### Randomized FD's

Sometimes shellcode will make assumptions about which file descriptor will be assigned to open files. We can help to break this kind of shellcode by randomizing the next file descriptor opened by a child process. Open fd's are inherited by child processes. We will open a minimum of 255 new fd's to also break shellcode searching through file descriptors as a byte value. Gatekeeper implements this with the -f flag.

    $ ./gatekeeper -l stdio -r stdio:/bin/bash -f
    ls -la /proc/self/fd
    total 0
    dr-x------ 2 bool bool  0 Jun 28 14:01 .
    dr-xr-xr-x 9 bool bool  0 Jun 28 14:01 ..
    lr-x------ 1 bool bool 64 Jun 28 14:01 0 -> pipe:[4825686]
    l-wx------ 1 bool bool 64 Jun 28 14:01 1 -> pipe:[4825687]
    lr-x------ 1 bool bool 64 Jun 28 14:01 10 -> /dev/urandom
    lr-x------ 1 bool bool 64 Jun 28 14:01 100 -> /dev/urandom
    ...

### Randomized Child Environment

Exploits sometimes require known or fixed offsets to stack addresses. For exploits making assumptions about stack offsets we can help vary those offsets by adding garbage variables to the forked child's environment. 

Example: 
    $ ./gatekeeper -l stdio -r stdio:/bin/bash -e
    env
    JpInn0HjfAthFNCj2vZPfZl2Zqb0m39tuMD0bEhxZGklX3bEqkAmNDFGMY3leHvtjtkbmabiJ4RWGtu5TgsYVh3xqTto6i4mLOXpf61wd7cF41A1p0o3HyoBl0VT6HeRgdNpMzPydtxnpEK7YZ9OprAVrgvdKCOPLJbc6vEMOkJuqh9eMoF7zLgoekvIV7lGOfNuenHJw8P1Ud4MINmif2GNCcZqkGx5bYDPFJgejH2gc18jgYM8p0f39YaRFREzPQ766Q9cyVInHUg6pmxbbHcA7URBPX7J9J=t5IbdMEWJpYMVnTkmzrcxpUVrGCc9FoYzJu4fckYunRRQnG91nRsZASigGmkT6OlH6Rriq2bzJFjJhhYwF1Wir5hxHRVeM5OBpJkx478aAmVBUPJdpEMu6Rl2UJa4BnFnx9k1qzjlYGmPzdjAebqlECnbo4BmYWcyr46wjte8wD8gXusaZkPVzvuRe4AEWvXCZUeL83qBbMsq0htDqVMHFqGbAsgpCcM2nl0W2PuzxICybQD8fdps73JsKoP19CcsViErIfxCukCZv8dEsZzrx1PFlA83Xwg1p3ZSLnyDZtKSdCduWYpQBdgXtOLs
    GsWDZwus87ykIj5ECxtdAZcnD5Q2PG8C0ixltV8raDtYz6YeTk0mpMhEAfimKevCVtI6J3KWeGHxO9VqkAoxaWcNleFa9SHIkz5awsBkhEZMTEi=HUeZtgD7RbDEUzPX9gH0SplIvxgD7QZhjfhypi



### Chroot

#### Setup

The next defense to cover could be considered a bit unfair. In 2013 the linux kernel introduced the ability to disassociate parts of the process execution context with an unshare() call. The capabilities unshared are inherited by forked child processes. This allows us to do things that we once would have needed root to do. The main one we have implemented into gatekeeper is the ability to chroot as a limited user. All we do is unshare(CLONE_NEWUSER); and then chroot(chrootstr); that's it. Since the service is running as limited user it is actually quite difficult to escape the chroot environment. First, lets setup our chroot for /bin/bash and /bin/ls. If our binaries are dynamically linked we will need to copy over relevant libraries:

    $ ldd /bin/bash
        linux-vdso.so.1 =>  (0x00007ffe0c9f0000)
        libtinfo.so.5 => /lib/x86_64-linux-gnu/libtinfo.so.5 (0x00007fc933a63000)
        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007fc93385f000)
        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fc933499000)
        /lib64/ld-linux-x86-64.so.2 (0x00007fc933cae000)
    $ mkdir chroot
    $ cd chroot
    $ mkdir bin lib lib64
    $ for i in $(ldd /bin/bash /bin/ls | grep -v dynamic | cut -d " " -f 3 | sed 's/://' | sort | uniq); do echo $i; cp --parents $i .; done; 
        /bin/bash
        /bin/ls
        /lib/x86_64-linux-gnu/libacl.so.1
        /lib/x86_64-linux-gnu/libattr.so.1
        /lib/x86_64-linux-gnu/libc.so.6
        /lib/x86_64-linux-gnu/libdl.so.2
        /lib/x86_64-linux-gnu/libpcre.so.3
        /lib/x86_64-linux-gnu/libselinux.so.1
        /lib/x86_64-linux-gnu/libtinfo.so.5
    $ cp --parents /lib64/ld-linux-x86-64.so.2 .

The for loop parses the output of ldd of /bin/bash and /bin/ls, extracts the filenames, and copies them into the chroot. Only execute this after you have changed directory to the chroot directory. There is one last file to setup in /lib64 the ld-linux loader. Now we run gatekeeper and wish all other teams lots of luck:

    $ ./gatekeeper -l stdio -r stdio:/bin/bash -t ./chroot -o 127.0.0.1:2090
    pwd
    /
    ls
    bin
    lib
    lib64

#### Detection

One way you can detect this kind of chroot is to look at inodes and uid's of files:
    $ ./gatekeeper -l stdio -r stdio:/bin/bash -t ./chroot -o 127.0.0.1:2090
    ls -lai
    total 20
    25559231 drwxr-x--- 5 65534 65534 4096 Jun 28 16:19 .
    25559231 drwxr-x--- 5 65534 65534 4096 Jun 28 16:19 ..
    25559293 drwxr-x--- 2 65534 65534 4096 Jun 28 16:24 bin
    25559295 drwxr-x--- 3 65534 65534 4096 Jun 28 16:24 lib
    25559294 drwxr-x--- 2 65534 65534 4096 Jun 28 16:27 lib64

We can see that the inode of . and .. are both 25559231 typically this is a much lower number for root direcotries. Look at ls -lai / to see yours. If we are not in a chroot . and .. will have different inode numbers for any subdirectory under /. 

The other thing we see is that the uid and gid on the directories are 65534/65534 this is a speical uid/gid called the overflow uid/gid and comes from /proc/sys/kernel/overflow{uid,gdi}. Due to the ease of detection, easy of implementation, and game breaking power of this techinque we are expecting for this strategy to either be banned outright or disabled in the game kernel. However, even a few hours of a perfect defense is very good for us. So, until that happens we need to be aware of one issue caused by this chroot. 

#### Defcon issues

Defcon CTF organizers use a file called /dev/ctf to verify that a service binary is running on the correct machine. This is a custom kernel module that when read it returns random looking data that we assume can be decoded by the organizers to verify we aren't redirecting network communications to a different machine. Running our binary inside a chroot will break this because there is no /dev available. We will be detected as not running on our game machine and our service will scored as down. 

To get around this we need to patch the service binaries to read from a constant file descriptor instead of opening /dev/ctf then we need gatekeeper to open /dev/ctf before the fork/exec call and dup that to the constant file fd. The constant fd we will use for this is fd 1337. You can specify -k /dev/ctf on the command line to enable this fd leak. For example:

    $ ./gatekeeper -l stdio -r stdio:/bin/bash -k /dev/urandom
    ls -la /proc/self/fd
    total 0
    dr-x------ 2 bool bool  0 Jun 28 13:51 .
    dr-xr-xr-x 9 bool bool  0 Jun 28 13:51 ..
    lr-x------ 1 bool bool 64 Jun 28 13:51 0 -> pipe:[4849044]
    l-wx------ 1 bool bool 64 Jun 28 13:51 1 -> pipe:[4849045]
    lr-x------ 1 bool bool 64 Jun 28 13:51 1337 -> /dev/urandom
    l-wx------ 1 bool bool 64 Jun 28 13:51 2 -> pipe:[4849045]
    lr-x------ 1 bool bool 64 Jun 28 13:51 3 -> pipe:[4849044]
    lr-x------ 1 bool bool 64 Jun 28 13:51 4 -> /proc/1824/fd
    l-wx------ 1 bool bool 64 Jun 28 13:51 6 -> pipe:[4849045]

If you are going to run a patched service binary without gatekeeper you will first need to setup this file descriptor doing something like this:

    exec 1337</dev/ctf && /path/to/patched/service/binary

    $ exec 1337</dev/urandom && /bin/bash
    $ ls -la /proc/self/fd
    total 0
    dr-x------ 2 bool bool  0 Jun 28 13:56 .
    dr-xr-xr-x 9 bool bool  0 Jun 28 13:56 ..
    lrwx------ 1 bool bool 64 Jun 28 13:56 0 -> /dev/pts/2
    lrwx------ 1 bool bool 64 Jun 28 13:56 1 -> /dev/pts/2
    lr-x------ 1 bool bool 64 Jun 28 13:56 1337 -> /dev/urandom
    lrwx------ 1 bool bool 64 Jun 28 13:56 2 -> /dev/pts/2
    lr-x------ 1 bool bool 64 Jun 28 13:56 3 -> /proc/2792/fd

### Blacklist Source IP Address
Coming soon.

## Content Filtering
Coming soon.

## Inotify

Linux provides a system to receive callback notifications for various file activities. Simply put, with inotify we can get a callback when our flag is opened and kill processes before the flag is read and sent to an attacking team. This is not part of the gatekeeper command line because it only needs to one once per service not once per service connection. This is designed to be run as the service user because once a callback is received a kill -9 signals will be sent to all processes except for the inotify process and it's parent (usually the shell).

    Terminal 1:
        test@ubuntu$ echo foo > /home/test/flag
        test@ubuntu$ ./inotify /home/test/flag

    Terminal 2:
        test@ubuntu:/home/test$ ls -la flag
        -rw-r--r-- 1 test test 4 Jun 28 12:30 flag
        test@ubuntu:/home/test$ cat flag
        user@ubuntu:~$

There is a race condition here that will sometimes let the flag be read and written before the inotify process can kill the offending pid. 