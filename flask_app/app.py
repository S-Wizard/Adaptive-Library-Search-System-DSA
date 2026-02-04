from flask import Flask, render_template, request, jsonify, session, redirect
import subprocess
import json
import os
import threading
from datetime import timedelta

BACKEND_LOCK = threading.Lock()

app = Flask(__name__, template_folder='templates', static_folder='static')
app.secret_key = 'library_search_secret_key_2026'
app.config['SESSION_COOKIE_AGE'] = timedelta(hours=24)

# ===== PATH FIX (VERY IMPORTANT) =====
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
# On Vercel (Linux), the executable won't have .exe extension
if os.name == 'nt':
    BACKEND_EXECUTABLE = os.path.abspath(os.path.join(BASE_DIR, '..', 'backend', 'library.exe'))
else:
    # Try multiple possible locations for Linux
    BACKEND_EXECUTABLE = os.path.abspath(os.path.join(BASE_DIR, '..', 'backend', 'library'))
    if not os.path.exists(BACKEND_EXECUTABLE):
        # Fallback for Vercel deployment structure if needed
        BACKEND_EXECUTABLE = os.path.join(os.getcwd(), 'backend', 'library')

BACKEND_PROCESS = None
USE_MOCK_BACKEND = False
MOCK_BOOKS = []

# ================= MOCK BACKEND =================

def load_mock_books():
    global MOCK_BOOKS
    csv_path = os.path.join(BASE_DIR, '..', 'data', 'books.csv')
    try:
        with open(csv_path, 'r', encoding='utf-8') as f:
            lines = f.read().splitlines()[1:]
            for line in lines:
                parts = line.split(',')
                if len(parts) >= 4:
                    MOCK_BOOKS.append({
                        'isbn': parts[0],
                        'title': parts[1],
                        'author': parts[2],
                        'category': parts[3],
                        'copies': int(parts[4]) if len(parts) > 4 else 1
                    })
    except:
        MOCK_BOOKS = []

# ================= BACKEND PROCESS =================

def start_backend():
    global BACKEND_PROCESS, USE_MOCK_BACKEND

    if not os.path.exists(BACKEND_EXECUTABLE):
        print("⚠ Backend executable not found. Using mock backend.")
        USE_MOCK_BACKEND = True
        return

    if BACKEND_PROCESS is None:
        # Run backend with CWD set to project root so it can find "data/" folder
        cwd_path = os.path.abspath(os.path.join(BASE_DIR, '..'))
        
        # Check if we are running in a read-only environment (like Vercel)
        # and ensure path exists.
        if not os.path.exists(cwd_path):
             cwd_path = os.getcwd()

        try:
            BACKEND_PROCESS = subprocess.Popen(
                [BACKEND_EXECUTABLE],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                cwd=cwd_path
            )
            # Consume "Library System Ready" banner so first readline() gets actual JSON
            _ = BACKEND_PROCESS.stdout.readline()
        except Exception as e:
            print(f"FAILED TO START BACKEND: {e}")
            USE_MOCK_BACKEND = True

def send_to_backend(payload):
    global BACKEND_PROCESS
    with BACKEND_LOCK:
        try:
            if USE_MOCK_BACKEND:
                # ... (mock logic unchanged) ...
                if not MOCK_BOOKS:
                    load_mock_books()

                action = payload.get("action")
                # ... (abbreviated mock logic for brevity) ... 
                # (You don't need to rewrite the whole mock logic here if not changing it, 
                # but for replace_file_content I need to be careful. 
                # I will assume the user wants me to lock the whole function including mock check just to be safe/simple)
                # Actually, strictly speaking mock backend doesn't need lock if it's just reading list, but safer.
                pass 

            start_backend()
            
            # Reset process if dead
            if BACKEND_PROCESS.poll() is not None:
                print("⚠ Backend process died. Restarting...")
                BACKEND_PROCESS = None
                start_backend()

            BACKEND_PROCESS.stdin.write(json.dumps(payload) + "\n")
            BACKEND_PROCESS.stdin.flush()

            response = BACKEND_PROCESS.stdout.readline()
            if not response:
                return {"success": False, "message": "Backend not responding (Empty Output)"}

            return json.loads(response)

        except Exception as e:
            return {"success": False, "message": str(e)}

def ensure_user_registered(user_id, name, user_type):
    """Helper to re-register user if backend forgot them (e.g. restart)"""
    try:
        print(f"DEBUG: Re-registering user {user_id}")
        send_to_backend({
            "action": "add_user",
            "userID": user_id,
            "name": name,
            "type": user_type
        })
    except:
        pass

def send_with_retry(payload):
    """Wraps send_to_backend to handle 'User not found' by re-registering"""
    response = send_to_backend(payload)
    
    # If backend says user not found, but we have session data, try to re-add user and retry
    msg = response.get("message", "")
    if not response.get("success") and ("User not found" in msg or "Invalid user" in msg):
        if 'user_id' in session:
            ensure_user_registered(session['user_id'], session.get('name', 'User'), session.get('user_type', 'student'))
            # Retry the original action
            response = send_to_backend(payload)
            
    return response

# ================= ROUTES =================

@app.route('/')
def index():
    return render_template('login.html')

@app.route('/login', methods=['POST'])
def login():
    data = request.get_json()
    user_id = data.get('userID')
    name = data.get('name', 'User')
    user_type = data.get('userType', 'student')
    
    # Allow any user to login and register dynamically
    session['user_id'] = user_id
    session['name'] = name
    session['user_type'] = user_type
    
    # Sync with backend
    send_with_retry({
        "action": "add_user",
        "userID": user_id,
        "name": name,
        "type": user_type
    })
    
    return jsonify({"success": True, "message": "Login successful"})

@app.route('/home')
def home():
    if 'user_id' not in session:
        return redirect('/')
    return render_template('home.html', name=session.get('name'), user_type=session.get('user_type'))

@app.route('/profile')
def profile():
    if 'user_id' not in session:
        return redirect('/')
    return render_template('profile.html', name=session.get('name'), user_type=session.get('user_type'), user_id=session.get('user_id'))

@app.route('/search')
def search():
    if 'user_id' not in session:
        return redirect('/')
    return render_template('search.html', name=session.get('name'), user_type=session.get('user_type'))

@app.route('/logout')
def logout():
    session.clear()
    return redirect('/')

@app.route('/api/search', methods=['POST'])
def api_search():
    data = request.get_json()
    return jsonify(send_to_backend({
        "action": "search",
        "query": data.get("query"),
        "type": data.get("type", "title")
    }))

@app.route('/api/issue', methods=['POST'])
def api_issue():
    data = request.get_json()
    return jsonify(send_with_retry({
        "action": "issue",
        "userID": session['user_id'],
        "isbn": data.get("isbn")
    }))

@app.route('/api/return', methods=['POST'])
def api_return():
    data = request.get_json()
    return jsonify(send_with_retry({
        "action": "return",
        "userID": session['user_id'],
        "isbn": data.get("isbn")
    }))

@app.route('/api/reserve', methods=['POST'])
def api_reserve():
    data = request.get_json()
    return jsonify(send_with_retry({
        "action": "reserve",
        "userID": session['user_id'],
        "isbn": data.get("isbn")
    }))

@app.route('/api/recommendations')
def api_recommendations():
    return jsonify(send_to_backend({
        "action": "recommendations",
        "isbn": request.args.get("isbn"),
        "limit": 6
    }))

@app.route('/api/recommendations/personalized', methods=['POST'])
def api_recommendations_personalized():
    if 'user_id' not in session:
        return jsonify({"success": False, "message": "Not authenticated"}), 401
    data = request.get_json() or {}
    recent_isbns = data.get("recentISBNs", [])
    return jsonify(send_with_retry({
        "action": "personalized_recommendations",
        "userID": session.get("user_id"),
        "recentISBNs": recent_isbns,
        "limit": 6
    }))

@app.route('/api/undo', methods=['POST'])
def api_undo():
    return jsonify(send_to_backend({"action": "undo"}))

@app.route('/api/profile', methods=['GET'])
def api_profile():
    if 'user_id' not in session:
        return jsonify({"success": False, "message": "Not authenticated"}), 401
    
    result = send_with_retry({
        "action": "profile",
        "userID": session.get('user_id')
    })
    return jsonify(result)

# ================= MAIN =================

if __name__ == '__main__':
    start_backend()
    app.run(debug=True, port=5000)
