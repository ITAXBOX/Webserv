#include "app/CgiExecutor.hpp"

CgiExecutor::CgiExecutor() {}

CgiExecutor::~CgiExecutor() {}

void CgiExecutor::start(const HttpRequest &request, const std::string &scriptPath, const std::string &interpreterPath, CgiState &state)
{
    if (pipe(state.pipeIn) == -1 || pipe(state.pipeOut) == -1)
    {
        Logger::error("Failed to create pipes for CGI");
        state.active = false;
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        Logger::error("Failed to fork for CGI");
        state.closePipes();
        state.active = false;
        return;
    }

    if (pid == 0)
    {
        // Close unused ends (Parent's ends)
        close(state.pipeIn[1]);  // Parent writes to this
        close(state.pipeOut[0]); // Parent reads from this

        // Redirect stdin from pipeIn[0]
        if (dup2(state.pipeIn[0], STDIN_FILENO) == -1)
        {
            Logger::error("dup2 stdin failed");
            exit(1);
        }
        close(state.pipeIn[0]);

        // Redirect stdout to pipeOut[1]
        if (dup2(state.pipeOut[1], STDOUT_FILENO) == -1)
        {
            Logger::error("dup2 stdout failed");
            exit(1);
        }
        close(state.pipeOut[1]);

        // Create environment and args
        char **envp = createEnvp(request, scriptPath);
        char **argv = createArgv(scriptPath, interpreterPath);

        // Execute
        execve(argv[0], argv, envp);

        // If execve returns, it failed
        Logger::error("execve failed for: " + scriptPath);
        freeArray(envp);
        freeArray(argv);
        exit(1);
    }
    else
    {
        state.pid = pid;
        state.active = true;
        state.requestBody = request.getBody();

        // Close unused ends (Child's ends)
        close(state.pipeIn[0]);
        close(state.pipeOut[1]);
        state.pipeIn[0] = -1;
        state.pipeOut[1] = -1;

        // Set non-blocking on remaining ends
        fcntl(state.pipeIn[1], F_SETFL, O_NONBLOCK);
        fcntl(state.pipeOut[0], F_SETFL, O_NONBLOCK);

        Logger::debug(Logger::fdMsg("CGI started, pid", pid));
    }
}

char **CgiExecutor::createArgv(const std::string &scriptPath, const std::string &interpreterPath)
{
    char **argv = new char *[3];

    // If interpreter is provided (e.g. /usr/bin/python3), use it locally
    // argv[0] = interpreter, argv[1] = script, argv[2] = NULL
    if (!interpreterPath.empty())
    {
        argv[0] = strdup(interpreterPath.c_str());
        argv[1] = strdup(scriptPath.c_str());
        argv[2] = NULL;
    }
    else
    {
        // Execute directly (requires executable permissions and shebang)
        argv[0] = strdup(scriptPath.c_str());
        argv[1] = NULL;
        argv[2] = NULL;
    }

    return argv;
}

char **CgiExecutor::createEnvp(const HttpRequest &request, const std::string &scriptPath)
{
    std::map<std::string, std::string> env;

    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"] = "HTTP/1.1";
    env["SERVER_SOFTWARE"] = "Webserv/1.0";

    env["REQUEST_METHOD"] = request.getMethodString();
    env["SCRIPT_FILENAME"] = scriptPath;
    env["SCRIPT_NAME"] = scriptPath; // Should be relative to root ideally
    env["QUERY_STRING"] = "";        // Extract from URI if needed

    // Extract Query String from URI
    std::string uri = request.getUri();
    size_t qPos = uri.find('?');
    if (qPos != std::string::npos)
        env["QUERY_STRING"] = uri.substr(qPos + 1);

    // Headers to Env
    std::string contentLength = request.getHeader("Content-Length");
    if (!contentLength.empty())
        env["CONTENT_LENGTH"] = contentLength;

    std::string contentType = request.getHeader("Content-Type");
    if (!contentType.empty())
        env["CONTENT_TYPE"] = contentType;

    // Add all other headers as HTTP_ variables
    const std::map<std::string, std::string> &headers = request.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::string key = it->first;
        // Skip Content-Length and Content-Type as they are already set
        if (key == "Content-Length" || key == "Content-Type")
            continue;

        std::string envKey = "HTTP_";
        for (size_t j = 0; j < key.length(); ++j)
        {
            char c = key[j];
            if (c == '-')
                envKey += '_';
            else
                envKey += std::toupper(c);
        }
        env[envKey] = it->second;
    }

    // Convert to char**
    char **envp = new char *[env.size() + 1];
    int i = 0;
    for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
    {
        std::string s = it->first + "=" + it->second;
        envp[i++] = strdup(s.c_str());
    }
    envp[i] = NULL;

    return envp;
}

void CgiExecutor::freeArray(char **array)
{
    if (!array)
        return;
    for (int i = 0; array[i]; ++i)
        free(array[i]);
    delete[] array;
}
