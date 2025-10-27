Sudo PAM plugin candidate challenge.
The main task was to create a sudo PAM plugin which collects and prints out all possible data about the program executed by sudo.
For example if the user runs the following:
 $ sudo ls
The plugin should output all available data about “ls” process such as pid, absolute path, 
process owner etc.

PAM Module:
1. I didn't have any information whatsoever on this matter. SO I have started to research.
2. The request was a PAM plugin. SO I read about PAM modules, and created one that log to stdout
   pid, absolute path, process owner. Only to realise PAM open session occure before the actual 
   command process is created, and close after process is done entierly.
   So, I cannot get the actual command process (not the sudo pid) from PAM module.
3. Nontheless, I have created a pam module that prints the command itself, just for fun because I have already created it.

APPLY THE SO :
1. Use "make all" in  "pam_plugin_project" dir --> pam_sudo_plugin.so will be created.
2. On X86 Linux, copy it to /usr/lib/x86_64-linux-gnu/security/pam_sudo_plugin.so
3. now manualy add or make sure that in file: /etc/pam.d/sudo, there is a line: session  optional  pam_sudo_plugin.so
   'optional': Means PAM won’t fail if your plugin has issues. You can change it to required if you want stricter enforcement.
   
   
I/O Sudo Plugin:
1. So I still had an issue geting data on the actual command PID. SO I learn about Sudo plugin.
2. I start to examine IO plugin (maybe I should use Policy plugin I'm not sure).
3. First I try to get the child PID in  io_open, but the child also as before wasn't created here yet.
4. So I use io_ttyout function. Whenever the child want to use the tty - it must be alive.
5. In io_ttyout first I try to use sudo_getpid() to get the child pid - but it didn't work for me (should be vest practice).
6. so I get the pid I actualy have (the sudo) and go over /proc under my sudo pid try to find its childs.
   worked file for most of cases.

APPLY THE SO :
1. Use "make all" in  "pam_plugin_project/io_plugin" dir --> sudo_io_plugin.so will be created.
2. copy the SO to a root friendly folder like : /usr/local/libexec/sudo/sudo_io_plugin.so
3. save copy of /etc/sudo.conf to sudo.conf.orig (in case something get bad).
4. now manualy add or make sure that in file: /etc/sudo.conf,  there is a line:
   Plugin su_io_plugin /usr/local/libexec/sudo/sudo_io_plugin.so


TRY HOOKING EXECV didn't work :(
1. I try to print or get the child cmd pid from itself. I know sudo remove the LD_PRELOAD when it starts
2. But, I thought maybe if we set the LD_PRELOAD env from the sudo plugin, it won't be removed before the fork to do the actual cmd.
3. I try to set the LD_PRELOAD with a hook.so from the sudo_io_plugin.cpp (setenv("LD_PRELOAD", "/usr/local/libexec/sudo/hook.so", 1);).
4. I created the hook.so from hook.c. You can compile it using make in folder "pam_plugin_project/hook" dir.
5. I moved the hook.so to /usr/local/libexec/sudo/hook.so (+x).
6. I edit etc/sudoers with line: Defaults        env_keep += "LD_PRELOAD"
7. but still, it didn;t work for me unfortunately.


I suggest apply both PAM Module & I/O Sudo Plugin, and see for yourself how its working :)
