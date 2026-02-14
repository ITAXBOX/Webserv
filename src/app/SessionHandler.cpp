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

    std::string body = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>WebServ - Session Manager</title>";
    body += "<style>*{margin:0;padding:0;box-sizing:border-box}:root{--bg:#0a0a0f;--surface:#141419;--border:rgba(240,192,64,.08);--text:#f1f1f1;--text-dim:#b4b4b4;--primary:#f0c040;--success:#4caf50;--error:#e74c3c;--info:#2196f3;--radius:12px}body{background:var(--bg);color:var(--text);font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,sans-serif;line-height:1.6;min-height:100vh;padding:20px}.container{max-width:1200px;margin:0 auto}header{text-align:center;padding:3rem 1rem;border-bottom:1px solid var(--border);margin-bottom:2rem}h1{font-size:2.5rem;color:var(--primary);margin-bottom:.5rem;font-weight:700}h2{font-size:1.4rem;color:var(--text);margin-bottom:1rem;font-weight:600}h3{font-size:1.1rem;color:var(--text);margin-bottom:.5rem}.subtitle{color:var(--text-dim);font-size:1.1rem}.back-btn-container{display:flex;justify-content:center;margin-bottom:2rem}.back-btn{display:inline-flex;align-items:center;gap:.5rem;color:var(--text);text-decoration:none;padding:.7rem 1.5rem;border-radius:20px;background:var(--surface);border:1px solid var(--border);transition:all .3s;font-size:.95rem}.back-btn:hover{background:var(--primary);color:var(--bg);border-color:var(--primary);transform:translateX(-3px)}.card-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(380px,1fr));gap:2rem;margin-bottom:2rem}.card{background:var(--surface);border:1px solid var(--border);border-radius:var(--radius);padding:2rem;transition:transform .3s}.card:hover{transform:translateY(-2px)}.status-badge{display:inline-block;padding:.75rem 1.5rem;border-radius:8px;font-weight:600;margin-bottom:1rem}.status-badge.success{background:rgba(76,175,80,.15);color:var(--success);border:1px solid rgba(76,175,80,.3)}.status-badge.info{background:rgba(33,150,243,.15);color:var(--info);border:1px solid rgba(33,150,243,.3)}.status-badge.new{background:rgba(240,192,64,.15);color:var(--primary);border:1px solid rgba(240,192,64,.3)}code{background:rgba(240,192,64,.1);color:var(--primary);padding:.3rem .6rem;border-radius:6px;font-size:.9rem;font-family:monospace;word-break:break-all}.data-list{list-style:none;margin:1rem 0}.data-list li{padding:.75rem 1rem;background:rgba(240,192,64,.05);border-left:3px solid var(--primary);margin-bottom:.5rem;border-radius:4px}.data-list li strong{color:var(--primary);margin-right:.5rem}.empty-state{color:var(--text-dim);font-style:italic;padding:1.5rem;text-align:center;background:rgba(180,180,180,.05);border-radius:8px}.form-group{margin-bottom:1.5rem}label{display:block;margin-bottom:.5rem;font-weight:600;color:var(--text-dim);font-size:.9rem}input[type=text]{width:100%;padding:.85rem 1rem;background:rgba(255,255,255,.05);border:1px solid var(--border);border-radius:8px;color:var(--text);font-size:1rem;transition:all .3s}input[type=text]:focus{outline:none;border-color:var(--primary);background:rgba(240,192,64,.08)}.btn{width:100%;padding:.85rem 1.5rem;border:none;border-radius:8px;font-size:1rem;font-weight:600;cursor:pointer;transition:all .3s}.btn-primary{background:linear-gradient(135deg,var(--primary),#d4a830);color:var(--bg)}.btn-primary:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(240,192,64,.3)}.btn-danger{background:linear-gradient(135deg,var(--error),#c0392b);color:#fff;margin-top:1.5rem}.btn-danger:hover{transform:translateY(-2px);box-shadow:0 4px 12px rgba(231,76,60,.3)}.divider{border-top:1px solid var(--border);margin:1.5rem 0;padding-top:1.5rem}.session-id{font-size:.85rem;color:var(--text-dim);margin-top:.5rem}.success-msg{background:rgba(76,175,80,.15);color:var(--success);padding:.75rem 1rem;border-radius:8px;margin:.75rem 0;border-left:3px solid var(--success)}footer{text-align:center;padding:2rem 1rem;margin-top:3rem;border-top:1px solid var(--border);color:var(--text-dim);font-size:.9rem}@media(max-width:768px){h1{font-size:2rem}.card-grid{grid-template-columns:1fr}.container{padding:0}}</style></head><body>";
    body += "<div class='container'><header><h1>🔐 Session Manager</h1><p class='subtitle'>Live Session Testing & State Management</p></header>";
    body += "<div class='back-btn-container'><a href='/index.html' class='back-btn'>← Go Back</a></div>";
    body += "<div class='card-grid'>";

    // Status Card
    body += "<div class='card'><h2>📊 Session Status</h2>";
    if (sessionId.empty() || !sm->getSession(sessionId))
    {
        // No valid session, create one
        sessionId = sm->createSession();
        response.addCookie("SESSIONID", sessionId, 1800);
        body += "<div class='status-badge new'>✨ New Session Created</div>";
        body += "<p class='session-id'>Session ID:<br><code>" + sessionId + "</code></p>";
        body += "<p style='margin-top:1rem;color:var(--text-dim);font-size:.9rem'>Your session will expire in 30 minutes of inactivity.</p>";
    }
    else
    {
        body += "<div class='status-badge success'>✓ Session Active</div>";
        body += "<p class='session-id'>Session ID:<br><code>" + sessionId + "</code></p>";
        body += "<p style='margin-top:1rem;color:var(--text-dim);font-size:.9rem'>Session is being tracked across requests.</p>";

        // Handle simple data storage
        if (request.getMethod() == HTTP_POST)
        {
            // Parse simple form data (key=value)
            std::string reqBody = request.getBody();

            // Simple URL-encoded body parser
            std::map<std::string, std::string> params;
            size_t pos = 0;
            while (pos < reqBody.length())
            {
                size_t amp = reqBody.find('&', pos);
                if (amp == std::string::npos)
                    amp = reqBody.length();

                std::string pair = reqBody.substr(pos, amp - pos);
                size_t eq_local = pair.find('=');
                if (eq_local != std::string::npos)
                {
                    std::string k = pair.substr(0, eq_local);
                    std::string v = pair.substr(eq_local + 1);

                    // Simple Decode: + to space
                    for (size_t i = 0; i < v.length(); ++i)
                        if (v[i] == '+')
                            v[i] = ' ';
                    // Decode URL percent encoding if needed (basic implementation)

                    params[k] = v;
                }
                pos = amp + 1;
            }

            // Check for destroy action first
            if (params.count("action") && params["action"] == "destroy")
            {
                sm->destroySession(sessionId);
                // Redirect to self to clear cookie/get new session
                response.setStatus(302, "Found");
                response.addHeader("Location", "/session_test");
                response.addCookie("SESSIONID", "", 0); // Clear cookie
                return response;
            }

            if (params.count("key") && params.count("value"))
            {
                std::string key = params["key"];
                std::string value = params["value"];

                if (!key.empty() && !value.empty())
                {
                    sm->setSessionData(sessionId, key, value);
                    body += "<div class='success-msg'>✓ Data Saved Successfully</div>";
                    body += "<p style='color:var(--success);font-weight:600'>" + key + " = " + value + "</p>";
                }
            }
        }
    }
    body += "</div>";

    // Data Store Card
    body += "<div class='card'><h2>💾 Session Data Store</h2>";
    Session *session = sm->getSession(sessionId);
    if (session && !session->data.empty())
    {
        body += "<ul class='data-list'>";
        for (std::map<std::string, std::string>::iterator it = session->data.begin(); it != session->data.end(); ++it)
            body += "<li><strong>" + it->first + ":</strong> " + it->second + "</li>";
        body += "</ul>";
    }
    else
        body += "<div class='empty-state'>📭 No data stored in this session yet.<br>Use the form below to add key-value pairs.</div>";
    body += "</div>";

    // Action Card
    body += "<div class='card'><h2>✏️ Modify Session</h2>";
    body += "<p style='color:var(--text-dim);margin-bottom:1.5rem'>Add key-value pairs to your persistent session storage.</p>";
    body += "<form method='POST' action='/session_test'>";
    body += "<div class='form-group'><label>🔑 Key</label><input type='text' name='key' placeholder='e.g., username' required></div>";
    body += "<div class='form-group'><label>📝 Value</label><input type='text' name='value' placeholder='e.g., John Doe' required></div>";
    body += "<button type='submit' class='btn btn-primary'>💾 Save to Session</button>";
    body += "</form>";

    // Add Destroy Session Button
    body += "<div class='divider'></div>";
    body += "<h3>🔥 Danger Zone</h3>";
    body += "<p style='color:var(--text-dim);margin-bottom:1rem;font-size:.9rem'>Destroy this session and clear all stored data.</p>";
    body += "<form method='POST' action='/session_test'>";
    body += "<input type='hidden' name='action' value='destroy'>";
    body += "<button type='submit' class='btn btn-danger' onclick='return confirm(\"Are you sure you want to destroy this session?\")'>🗑️ Destroy Session</button>";
    body += "</form>";

    body += "</div>";

    body += "</div>"; // End card-grid
    
    // Add info section
    body += "<div class='card' style='margin-top:2rem'><h3>ℹ️ How It Works</h3>";
    body += "<p style='color:var(--text-dim);line-height:1.8'>";
    body += "Sessions are managed server-side with a unique ID stored in a cookie. ";
    body += "Your browser automatically sends the <code>SESSIONID</code> cookie with each request, ";
    body += "allowing the server to retrieve and persist your data across multiple HTTP requests. ";
    body += "Sessions expire after 30 minutes of inactivity.";
    body += "</p></div>";
    
    body += "<footer><p>WebServ Session Manager • Built with C++98 • Cookie-based authentication</p></footer>";
    body += "</div></body></html>";

    response.setStatus(HTTP_OK, "OK");
    response.setBody(body);
    response.addHeader("Content-Type", "text/html");

    return response;
}
