# 📚 Adaptive Library Search System

> A high-performance, hybrid library management system featuring a **C++ Search Engine** backend and a **Flask** frontend.

![Dashboard Preview](https://via.placeholder.com/800x400?text=Library+Dashboard+Preview)

## 🚀 Features

### Core Functionality
- **Hybrid Architecture**: Leverages C++ for high-speed data processing (Tries, AVL Trees, Custom Graphs) and Python/Flask for a modern web interface.
- **Dynamic Search**: Instant search by Title or Author using optimized Trie data structures.
- **Real-Time Availability**: Live tracking of book copies and borrower status.

### User Experience
- **Dynamic User Profiles**: Real-time tracking of borrowed books, reservations, and history.
- **Personalized Recommendations**: Graph-based recommendation engine that suggests books based on reading history and similar users.
- **Undo Actions**: Global "Undo" button to revert accidental issues or returns using a Transaction Stack.

### Data Persistence
- **CSV Storage**: All data (Books, Users) is persisted in `data/books.csv` and `data/users.csv`.
- **Auto-Registration**: New users are automatically registered and saved upon first login.

## 🛠️ Tech Stack

- **Backend**: C++17
    - *Libraries*: `nlohmann/json`
    - *Structures*: Trie, AVL Tree, Graph, Stack
- **Frontend**: Python (Flask)
    - *UI*: HTML5, CSS3 (Glassmorphism), JavaScript (Fetch API)
- **Communication**: JSON over Standard I/O (Subprocess)

## ⚙️ Setup & Installation

### Prerequisites
- Python 3.8+
- G++ Compiler (MinGW or equivalent) supporting C++17

### 1. Compile the Backend
The C++ backend must be compiled before running the application.
```bash
g++ -std=c++17 -Ibackend/include backend/main.cpp backend/library_engine.cpp backend/avl_tree.cpp backend/trie.cpp backend/recommendation_graph.cpp -o backend/library.exe
```

### 2. Install Python Dependencies
```bash
pip install flask
```

### 3. Run the Application
Start the Flask server:
```bash
python flask_app/app.py
```
Access the dashboard at `http://127.0.0.1:5000`.

## 📂 Project Structure

```
├── backend/               # C++ Source Code
│   ├── include/          # Header files
│   ├── library_engine.cpp # Core logic
│   └── main.cpp          # Entry point & JSON Handler
├── data/                 # CSV Data Stores
│   ├── books.csv
│   └── users.csv
├── flask_app/            # Frontend Application
│   ├── static/           # CSS, JS, Images
│   ├── templates/        # HTML Templates
│   └── app.py            # Flask Server & Subprocess Manager
└── README.md             # Documentation
```

## 📝 Usage Guide

1.  **Login**: Enter any User ID (e.g., "U101") to register/login.
2.  **Dashboard**: Browse "Featured Books" or search for specific titles.
3.  **Borrow/Reserve**: Click command buttons on book cards.
4.  **Profile**: View your borrowed books and return them.
5.  **Undo**: Use the Undo button in the navbar to revert the last action.
