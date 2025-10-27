#include <unistd.h>                 // for some system calls.
#include <pwd.h>                    // for getting user info from UID.
#include <sys/stat.h>              // for stat() to get file metadata
#include <filesystem>              // For resolving symbolic links
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

// Fix C99 'restrict' keyword for C++ compatibility
#define restrict

extern "C" {
    #include <sudo_plugin.h>
}

// Declare global plugin_printf pointer
static sudo_printf_t g_plugin_printf = nullptr;

/*
 * This function is called when the plugin is loaded.
 * It is where you can initialize resources.
 */
static int io_open(unsigned int version,
                   sudo_conv_t conversation,
                   sudo_printf_t plugin_printf,
                   char* const settings[],
                   char* const user_info[],
                   char* const plugin_args[],
                   int argc,
                   char* const argv[],
                   char* const plugin_options[],
                   char* const user_env[],
                   const char** unused)
{
    g_plugin_printf = plugin_printf;
    
    // Try also inject an hook.so to LD_PRELOAD, which hook execve so on fork, when the "ls" exe start
    // and get into execve, ill catch it, print the pid, and re-invoke the "real" "ls".
    // "ls" it's just an example.
    // non the less, that didn't work for me unfurtionatly.
    
    //setenv("LD_PRELOAD", "/usr/local/libexec/sudo/hook.so", 1);

	char exec_path[1024] = {0};
	for (int i = 0; plugin_args[i] != nullptr; ++i) {
		if (strncmp(plugin_args[i], "command=", 8) == 0) {
		    strncpy(exec_path, plugin_args[i] + 8, sizeof(exec_path) - 1);
		    break;		    
		}
	}
	
	char userName[1024] = {0};
	for (int i = 0; user_info[i] != nullptr; ++i) {
		if (strncmp(user_info[i], "user=", 5) == 0) {
		    strncpy(userName, user_info[i] + 5, sizeof(userName) - 1);
		    break;		    
		}
	}

    struct stat statbuf;
    struct passwd* pw = nullptr;
    if (stat(exec_path, &statbuf) == 0) {
        pw = getpwuid(statbuf.st_uid);
    }
    
    if (g_plugin_printf) {
        g_plugin_printf(SUDO_CONV_INFO_MSG, "=== SUDO TARGET COMMAND INFO ===\n");
        g_plugin_printf(SUDO_CONV_INFO_MSG, "Invoking User: %s\n", userName);
        g_plugin_printf(SUDO_CONV_INFO_MSG, "Target Executable Path: %s\n", exec_path);
        g_plugin_printf(SUDO_CONV_INFO_MSG, "Target Process Owner: %s\n", (pw ? pw->pw_name : "Unknown"));
    } 
           
    return 1;
}

static void io_close(int exit_status, int error)
{
    // No cleanup needed
}

static int get_child_pid(pid_t pid) {
    std::ostringstream pathStream;
    pathStream << "/proc/" << pid << "/task/" << pid << "/children";
    std::string path = pathStream.str();

    std::ifstream file(path);
    if (!file.is_open()) {
        return -1; // Indicate error
    }

    std::string buffer;
    if (std::getline(file, buffer)) {
        std::istringstream iss(buffer);
        int childPid;
        if (iss >> childPid) {
            file.close();
            return childPid;
        }
    }

    file.close();
    return -1; // No child PID found
}

static void print_child_proc_pid(pid_t pid) {

    int childPid = get_child_pid(pid); // sudo_getpid() didn't work for me but it's a better option
    if (childPid == -1) {
        if (g_plugin_printf) {
            g_plugin_printf(SUDO_CONV_INFO_MSG, "No child PID has been found\n");
        }
        return;
    }

    if (g_plugin_printf) {
        g_plugin_printf(SUDO_CONV_INFO_MSG, "Sudo PID: %d, Child PID: %d\n", pid, childPid);
        g_plugin_printf(SUDO_CONV_INFO_MSG, "================================\n");
    }
}

static int io_ttyout(const char* buf, unsigned int len, const char** unused)
{
	// invoke it here, because here the child process ,must be created and still alive.
	
    static bool child_pid_logged = false;
    if (!child_pid_logged) {
        pid_t su_pid = getpid();
        print_child_proc_pid(su_pid);
        child_pid_logged = true;
    }
    return 1;
}

extern "C" struct io_plugin su_io_plugin = {
    SUDO_IO_PLUGIN,
    SUDO_API_VERSION,
    io_open,
    io_close,
    nullptr, // show_version
    nullptr, // log_ttyin
    io_ttyout,
    nullptr, // log_stdin
    nullptr,
    nullptr,
    nullptr, // log_winchange
    nullptr, // log_suspend
};

