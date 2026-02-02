#!/usr/bin/env python3
import os

# Robust path finding for uploads directory
POSSIBLE_PATHS = [
    "www/uploads",       # From project root (most likely for Webserv)
    "../uploads",        # From cgi-bin directory
    "uploads"            # Fallback
]

UPLOAD_DIR = "www/uploads" # Default
for p in POSSIBLE_PATHS:
    if os.path.exists(p) and os.path.isdir(p):
        UPLOAD_DIR = p
        break

print("Content-Type: text/html\r\n\r\n")
print("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>File Manager</title>")
print("<link rel='stylesheet' href='../style.css'>")
print("""<script>
    function updateFileName(input) {
        const label = document.getElementById('file-label-text');
        if (input.files && input.files.length > 0) label.innerText = input.files[0].name;
    }
</script>""")
print("</head><body>")
print("<div class='container'>")
print("<header><h1>File Manager</h1><p class='subtitle'>Upload, View, and Manage Files</p></header>")
print("<nav><a href='../index.html'>Dashboard</a><a href='../methods.html'>Methods</a><a href='file_manager.py' class='active'>File Manager</a><a href='../cgi.html'>CGI Tests</a><a href='../sessions.html'>Sessions</a></nav>")

print("<div class='card-grid'>")

# CARD 1: UPLOAD FORM
print("<div class='card'>")
print("<h2>Upload New File</h2>")
print("<p style='color: var(--text-light); margin-bottom: 20px;'>Select a file to save to the server.</p>")
print("<form action='/uploads' method='POST' enctype='multipart/form-data'>")
print("<div class='upload-zone' style='border: 2px dashed #e0e0e0; padding: 20px; text-align: center; border-radius: 8px; margin-bottom: 15px;'>")
print("<input type='file' name='file' id='file' class='file-input' onchange='updateFileName(this)' required>")
print("<label for='file' class='file-label' style='display: inline-block; padding: 10px 20px; background: #4a90e2; color: white; border-radius: 5px; cursor: pointer;'> <span id='file-label-text'>Choose a file...</span></label>")
print("</div>")
print("<button type='submit' class='btn' style='width: 100%;'>Upload File</button>")
print("</form>")
print("</div>")

# CARD 2: FILE LIST
print("<div class='card'>")
print(f"<h2>Existing Files <span style='font-size: 0.6em; font-weight: normal; color: #888'>(Dir: {UPLOAD_DIR})</span></h2>")
print("<div class='file-list'>")

try:
    if not os.path.exists(UPLOAD_DIR):
        print(f"<p style='color: orange'>Directory not found: {UPLOAD_DIR} (CWD: {os.getcwd()})</p>")
    else:
        files = os.listdir(UPLOAD_DIR)
        if not files:
            print("<p>No files uploaded yet.</p>")
        else:
            print("<table style='width: 100%; border-collapse: collapse;'>")
            print("<thead><tr style='text-align: left; border-bottom: 2px solid #eee;'>")
            print("<th style='padding: 10px;'>Filename</th><th style='padding: 10px;'>Size</th><th style='padding: 10px;'>Actions</th></tr></thead>")
            print("<tbody>")
            
            for f in files:
                full_path = os.path.join(UPLOAD_DIR, f)
                if os.path.isfile(full_path):
                    size = os.path.getsize(full_path)
                    print(f"<tr style='border-bottom: 1px solid #eee;'>")
                    print(f"<td style='padding: 10px;'>{f}</td>")
                    print(f"<td style='padding: 10px;'>{size} B</td>")
                    print(f"<td style='padding: 10px;'>")
                    print(f"<button onclick=\"deleteFile('{f}')\" class='btn-danger'>Delete</button>")
                    print(f"</td></tr>")
            
            print("</tbody></table>")
except Exception as e:
    print(f"<p style='color: red'>Error listing files: {e}</p>")

print("</div></div>")
print("</div>") # End card-grid

# JavaScript for DELETE request
print("""
<script>
async function deleteFile(filename) {
    if (!confirm('Are you sure you want to delete ' + filename + '?')) return;
    
    try {
        const response = await fetch('/uploads/' + filename, {
            method: 'DELETE'
        });
        
        if (response.ok) {
            alert('File deleted successfully');
            location.reload(); 
        } else {
            alert('Failed to delete file: ' + response.statusText);
        }
    } catch (error) {
        alert('Error: ' + error);
    }
}
</script>
""")

# CSS specific to this page
print("""
<style>
.btn-danger {
    background-color: #e74c3c;
    color: white;
    border: none;
    padding: 5px 10px;
    border-radius: 4px;
    cursor: pointer;
}
.btn-danger:hover { background-color: #c0392b; }
.btn-small {
    background-color: #4a90e2;
    color: white;
    text-decoration: none;
    padding: 5px 10px;
    border-radius: 4px;
    font-size: 0.9em;
}
</style>
""")

print("<footer><p>WebServ Project &copy; 2026</p></footer>")
print("</div></body></html>")
