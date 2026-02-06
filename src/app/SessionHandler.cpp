#include "app/SessionHandler.hpp"

SessionHandler::SessionHandler()
{
}

SessionHandler::~SessionHandler()
{
}

HttpResponse SessionHandler::handle(const HttpRequest &request)
{
    Logger::info("Handling session request");
    
    HttpResponse response;
    SessionManager *sm = SessionManager::getInstance();
    std::string sessionId = request.getCookie("SESSIONID");
    std::string action = request.getHeader("X-Action"); // Simple way to signal action
    
    std::string body = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>WebServ - Session Test</title><link rel='stylesheet' href='style.css'></head><body>";
    body += "<div class='container'><header><h1>Session Test Environment</h1><p class='subtitle'>Live Session Data Inspector</p></header>";
    body += "<nav><a href='index.html'>Dashboard</a><a href='methods.html'>Methods</a><a href='cgi-bin/file_manager.py'>File Manager</a><a href='cgi.html'>CGI Tests</a><a href='sessions.html' class='active'>Sessions</a></nav>";
    body += "<div class='card-grid'>";
    
    // Status Card
    body += "<div class='card'><h2>Session Status</h2>";
    if (sessionId.empty() || !sm->getSession(sessionId))
    {
        // No valid session, create one
        sessionId = sm->createSession();
        response.addCookie("SESSIONID", sessionId, 1800);
        body += "<div style='padding: 10px; background: #e3f2fd; border-radius: 5px; color: #1976d2;'><strong>New Session Created</strong></div>";
        body += "<p style='margin-top:10px'>ID: <code>" + sessionId + "</code></p>";
    }
    else
    {
        body += "<div style='padding: 10px; background: #e8f5e9; border-radius: 5px; color: #2e7d32;'><strong>Session Active</strong></div>";
        body += "<p style='margin-top:10px'>ID: <code>" + sessionId + "</code></p>";
        
        // Handle simple data storage
        if (request.getMethod() == HTTP_POST)
        {
            // Parse simple form data (key=value)
            std::string reqBody = request.getBody();
            
            // Simple URL-encoded body parser
            std::map<std::string, std::string> params;
            size_t pos = 0;
            while (pos < reqBody.length()) {
                size_t amp = reqBody.find('&', pos);
                if (amp == std::string::npos) amp = reqBody.length();
                
                std::string pair = reqBody.substr(pos, amp - pos);
                size_t eq_local = pair.find('=');
                if (eq_local != std::string::npos) {
                    std::string k = pair.substr(0, eq_local);
                    std::string v = pair.substr(eq_local + 1);
                    
                    // Simple Decode: + to space
                    for (size_t i = 0; i < v.length(); ++i) if (v[i] == '+') v[i] = ' ';
                    // Decode URL percent encoding if needed (basic implementation)
                    // ...

                    params[k] = v;
                }
                pos = amp + 1;
            }

            // Check for destroy action first
            if (params.count("action") && params["action"] == "destroy") {
                sm->destroySession(sessionId);
                // Redirect to self to clear cookie/get new session
                response.setStatus(302, "Found");
                response.addHeader("Location", "/session_test");
                response.addCookie("SESSIONID", "", 0); // Clear cookie
                return response;
            }

            if (params.count("key") && params.count("value")) {
                std::string key = params["key"];
                std::string value = params["value"];
                
                if (!key.empty() && !value.empty()) {
                    sm->setSessionData(sessionId, key, value);
                    body += "<div style='padding: 10px; background: #e3f2fd; border-radius: 5px; color: #1976d2; margin-bottom: 20px;'><strong>Data Saved</strong></div>";
                    body += "<p style='color: green'>Saved: " + key + " = " + value + "</p>";
                }
            }
        }
    }
    body += "</div>";

    // Data Store Card
    body += "<div class='card'><h2>Session Data Store</h2>";
    Session *session = sm->getSession(sessionId);
    if (session && !session->data.empty())
    {
        body += "<ul class='feature-list'>";
        for (std::map<std::string, std::string>::iterator it = session->data.begin(); it != session->data.end(); ++it)
        {
            body += "<li><strong>" + it->first + ":</strong> " + it->second + "</li>";
        }
        body += "</ul>";
    }
    else
    {
        body += "<p style='color: #7f8c8d'>No data stored in this session yet.</p>";
    }
    body += "</div>";

    // Action Card
    body += "<div class='card'><h2>Modify Session</h2>";
    body += "<p>Add key-value pairs to your persistent session.</p>";
    body += "<form method='POST' action='/session_test' style='margin-top: 15px;'>";
    body += "<div style='margin-bottom: 15px;'><label style='display:block; margin-bottom:5px; font-weight:600'>Key</label><input type='text' name='key' style='width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 6px;' required></div>";
    body += "<div style='margin-bottom: 15px;'><label style='display:block; margin-bottom:5px; font-weight:600'>Value</label><input type='text' name='value' style='width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 6px;' required></div>";
    body += "<button type='submit' class='btn' style='width: 100%; background-color: var(--secondary-color); color: var(--text-color); border:none; cursor:pointer;'>Save to Session</button>";
    body += "</form>";
    
    // Add Destroy Session Button
    body += "<form method='POST' action='/session_test' style='margin-top: 20px; border-top: 1px solid #eee; padding-top: 20px;'>";
    body += "<input type='hidden' name='action' value='destroy'>";
    body += "<button type='submit' class='btn' style='width: 100%; background-color: #e74c3c; color: white; border: none; cursor:pointer;'>Destroy Session</button>";
    body += "</form>";
    
    body += "</div>";
    
    body += "</div>"; // End card-grid
    body += "</div></body></html>";
    
    response.setStatus(HTTP_OK, "OK");
    response.setBody(body);
    response.addHeader("Content-Type", "text/html");

    return response;
}
