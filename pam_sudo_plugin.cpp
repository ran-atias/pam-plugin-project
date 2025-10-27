#include <security/pam_modules.h>	// gives access to PAM entry points like pam_sm_open_session
#include <security/pam_ext.h>		// functions like pam_info() for printing messages to the user
#include <unistd.h>					// for getpid() and other system calls.
#include <pwd.h>					// for getting user info from UID.
#include <sys/stat.h>				// for stat() to get file metadata
#include <filesystem>				// For resolving symbolic links
#include <fstream>
#include <sstream>
#include <string>

// extern "C" Ensures the functions inside use C linkage, which is required for PAM to load them correctly.
// without this, the C++ compiler would mangle the function names and PAM wouldn’t find them.
extern "C" {

// this is the entry point PAM calls when a session is opened (when sudo is run).
int pam_sm_open_session(pam_handle_t* pamh, int flags, int argc, const char** argv) { 
    
    pid_t parentPid = getpid(); // gets the current process ID (sudo process).							
    std::stringstream ss;
    ss << parentPid;
    std::string pidStr = ss.str();
    
    // Read the command-line arguments of the child process
    std::string cmdlinePath = "/proc/" + pidStr + "/cmdline"; // Command-line arguments   
    std::ifstream cmdlineFile(cmdlinePath);
    std::string args, token;
    while (std::getline(cmdlineFile, token, '\0')) { // Arguments are null-separated
        if (!args.empty()) args += " "; // Separate arguments with spaces
        args += token;
    }

    // Use pam_info to print to terminal
    // These lines use pam_info() to print formatted messages to the terminal.
    // This is more reliable than std::cout for PAM modules.
    pam_info(pamh, "Command Line: %s", args.c_str());
    return PAM_SUCCESS;
}

// this is the entry point PAM calls when a session is close (when sudo is run).
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

}
