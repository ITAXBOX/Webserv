#ifndef CGISTATE_HPP
#define CGISTATE_HPP

#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>

struct CgiState
{
    pid_t pid;
    int pipeIn[2];              // Parent -> Child (Write body here)
    int pipeOut[2];             // Child -> Parent (Read response here)
    std::string requestBody;    // Body to write to CGI
    std::string responseBuffer; // Output from CGI
    bool headersParsed;
    bool active;

    CgiState() : pid(-1), headersParsed(false), active(false)
    {
        pipeIn[0] = -1;
        pipeIn[1] = -1;
        pipeOut[0] = -1;
        pipeOut[1] = -1;
    }

    void closePipes()
    {
        if (pipeIn[0] != -1)
        {
            close(pipeIn[0]);
            pipeIn[0] = -1;
        }
        if (pipeIn[1] != -1)
        {
            close(pipeIn[1]);
            pipeIn[1] = -1;
        }
        if (pipeOut[0] != -1)
        {
            close(pipeOut[0]);
            pipeOut[0] = -1;
        }
        if (pipeOut[1] != -1)
        {
            close(pipeOut[1]);
            pipeOut[1] = -1;
        }
    }
};

#endif
