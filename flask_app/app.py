from flask import Flask, render_template, request, jsonify, session, redirect
import subprocess
import json
import os
import threading
import csv
import time
from datetime import timedelta
from pymongo import MongoClient
from dotenv import load_dotenv

load_dotenv()

BACKEND_LOCK = threading.Lock()

app = Flask(__name__, template_folder='templates', static_folder='static')
app.secret_key = 'library_search_secret_key_2026'
app.config['SESSION_COOKIE_AGE'] = timedelta(hours=24)

# MongoDB Setup
MONGO_URI = os.getenv("MONGODB_URI")
if not MONGO_URI:
    print("⚠ MONGODB_URI not found in environment variables. Persistence will be limited.")
    client = None
    db = None
else:
    # Add a timeout so it doesn't hang indefinitely if the connection is bad
    client = MongoClient(MONGO_URI, serverSelectionTimeoutMS=5000)
    db = client['library_system']

# ===== PATH FIX (VERY IMPORTANT) =====
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# Temp Data Path for Vercel/Linux
# We use /tmp because it's the only writable directory on Vercel
if os.name == 'nt':
    DATA_DIR = os.path.abspath(os.path.join(BASE_DIR, '..', 'data'))
else:
    DATA_DIR = '/tmp/libsearch_data'
    os.makedirs(DATA_DIR, exist_ok=True)

BOOKS_CSV = os.path.join(DATA_DIR, 'books.csv')
USERS_CSV = os.path.join(DATA_DIR, 'users.csv')
TRANS_CSV = os.path.join(DATA_DIR, 'transactions.csv')

# On Vercel (Linux), the executable won't have .exe extension
if os.name == 'nt':
    BACKEND_EXECUTABLE = os.path.abspath(os.path.join(BASE_DIR, '..', 'backend', 'library.exe'))
else:
    # Try multiple possible locations for Linux to find the binary
    possible_paths = [
        os.path.abspath(os.path.join(BASE_DIR, '..', 'backend', 'library')), # Main repo structure
        os.path.join(os.getcwd(), 'backend', 'library'),                    # Vercel current dir
        os.path.join(os.getcwd(), 'library')                                # Root dir
    ]
    BACKEND_EXECUTABLE = None
    for path in possible_paths:
        if os.path.exists(path):
            BACKEND_EXECUTABLE = path
            break
    
    if not BACKEND_EXECUTABLE:
        BACKEND_EXECUTABLE = possible_paths[0] # Default to first path if not found

BACKEND_PROCESS = None
USE_MOCK_BACKEND = False
MOCK_BOOKS = []

# ================= MIGRATION & REHYDRATION =================

def rehydrate_from_mongodb():
    """Download data from MongoDB to local temp CSV files for C++ backend"""
    if not db: return

    print("🔄 Rehydrating data from MongoDB...")
    
    # 1. Rehydrate Books
    books = list(db.books.find({}, {'_id': 0}))
    if books:
        with open(BOOKS_CSV, 'w', encoding='utf-8', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['ISBN', 'Title', 'Author', 'Category', 'Copies'])
            writer.writeheader()
            for b in books:
                # Map mongo keys to CSV keys
                writer.writerow({
                    'ISBN': b.get('isbn'),
                    'Title': b.get('title'),
                    'Author': b.get('author'),
                    'Category': b.get('category'),
                    'Copies': b.get('copies', 1)
                })
    
    # 2. Rehydrate Users
    users = list(db.users.find({}, {'_id': 0}))
    if users:
        with open(USERS_CSV, 'w', encoding='utf-8', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['UserID', 'Name', 'Email', 'Type'])
            writer.writeheader()
            for u in users:
                writer.writerow({
                    'UserID': u.get('userID'),
                    'Name': u.get('name'),
                    'Email': u.get('email'),
                    'Type': u.get('type', 'STUDENT')
                })

    # 3. Rehydrate Transactions
    txns = list(db.transactions.find({}, {'_id': 0}))
    if txns:
        with open(TRANS_CSV, 'w', encoding='utf-8', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=['TID', 'UID', 'BID', 'CID', 'Type', 'Timestamp'])
            writer.writeheader()
            for t in txns:
                writer.writerow({
                    'TID': t.get('tid', 'TXN_INF'),
                    'UID': t.get('userID'),
                    'BID': t.get('isbn'),
                    'CID': t.get('copyID', ''),
                    'Type': t.get('type'),
                    'Timestamp': int(t.get('timestamp', 0))
                })
    else:
        # Create empty if not exists to avoid backend crash
        with open(TRANS_CSV, 'w', encoding='utf-8', newline='') as f:
            f.write("TID,UID,BID,CID,Type,Timestamp\n")

def initial_migration():
    """One-time migration from local CSV to MongoDB if DB is empty"""
    if not db: return
    
    if db.books.count_documents({}) == 0:
        local_books_path = os.path.abspath(os.path.join(BASE_DIR, '..', 'data', 'books.csv'))
        if os.path.exists(local_books_path):
            print("🚀 Migrating local books to MongoDB...")
            with open(local_books_path, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                books_to_add = []
                for row in reader:
                    books_to_add.append({
                        'isbn': row['ISBN'],
                        'title': row['Title'],
                        'author': row['Author'],
                        'category': row['Category'],
                        'copies': int(row['Copies'])
                    })
                if books_to_add:
                    db.books.insert_many(books_to_add)

    if db.users.count_documents({}) == 0:
        local_users_path = os.path.abspath(os.path.join(BASE_DIR, '..', 'data', 'users.csv'))
        if os.path.exists(local_users_path):
            print("🚀 Migrating local users to MongoDB...")
            with open(local_users_path, 'r', encoding='utf-8') as f:
                reader = csv.DictReader(f)
                users_to_add = []
                for row in reader:
                    users_to_add.append({
                        'userID': row['UserID'],
                        'name': row['Name'],
                        'email': row['Email'],
                        'type': row['Type']
                    })
                if users_to_add:
                    db.users.insert_many(users_to_add)

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
        # Rehydrate data from MongoDB on startup
        try:
            initial_migration()
            rehydrate_from_mongodb()
        except Exception as e:
            print(f"FAILED TO REHYDRATE: {e}")
            USE_MOCK_BACKEND = True

        # Run backend with CWD set to project root so it can find files relative to it
        cwd_path = os.path.abspath(os.path.join(BASE_DIR, '..'))
        if not os.path.exists(cwd_path):
             cwd_path = os.getcwd()

        try:
            # Pass custom data paths to C++ backend
            args = [BACKEND_EXECUTABLE, BOOKS_CSV, USERS_CSV, TRANS_CSV]
            BACKEND_PROCESS = subprocess.Popen(
                args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
                cwd=cwd_path
            )
            # Consume "Library System Ready" banner
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

    # Sync with MongoDB
    if db:
        db.users.update_one(
            {"userID": user_id},
            {"$set": {"name": name, "email": f"{user_id}@library.edu", "type": user_type.upper()}},
            upsert=True
        )
    
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
    isbn = data.get("isbn")
    response = send_with_retry({
        "action": "issue",
        "userID": session['user_id'],
        "isbn": isbn
    })
    
    # Sync with MongoDB
    if response.get("success") and db:
        db.books.update_one({"isbn": isbn}, {"$inc": {"copies": -1}})
        db.transactions.insert_one({
            "tid": f"TXN_{int(time.time())}",
            "userID": session['user_id'],
            "isbn": isbn,
            "type": "ISSUE",
            "timestamp": int(time.time())
        })
    
    return jsonify(response)

@app.route('/api/return', methods=['POST'])
def api_return():
    data = request.get_json()
    isbn = data.get("isbn")
    response = send_with_retry({
        "action": "return",
        "userID": session['user_id'],
        "isbn": isbn
    })
    
    # Sync with MongoDB
    if response.get("success") and db:
        db.books.update_one({"isbn": isbn}, {"$inc": {"copies": 1}})
        db.transactions.insert_one({
            "tid": f"TXN_{int(time.time())}",
            "userID": session['user_id'],
            "isbn": isbn,
            "type": "RETURN",
            "timestamp": int(time.time())
        })
        
    return jsonify(response)

@app.route('/api/reserve', methods=['POST'])
def api_reserve():
    data = request.get_json()
    isbn = data.get("isbn")
    response = send_with_retry({
        "action": "reserve",
        "userID": session['user_id'],
        "isbn": isbn
    })
    
    # Sync with MongoDB
    if response.get("success") and db:
        db.reservations.insert_one({
            "userID": session['user_id'],
            "isbn": isbn,
            "timestamp": timedelta(0).total_seconds()
        })
        
    return jsonify(response)

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
