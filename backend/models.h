#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <ctime>

/* ================= ENUMS ================= */

enum class UserType { STUDENT, FINAL_YEAR_STUDENT, FACULTY };
enum class TransactionType { ISSUE, RETURN };

/* ================= COPY (LINKED LIST NODE) ================= */

struct Copy {
    std::string copyID;
    bool isAvailable;
    std::string issuedTo;
    long long issuedDate;
    Copy* next;

    Copy(const std::string& id)
        : copyID(id), isAvailable(true),
          issuedDate(0), next(nullptr) {}
};

/* ================= BOOK ================= */

struct Book {
    std::string isbn;
    std::string title;
    std::string author;
    std::string category;

    int totalCopies;
    int availableCopies;

    Copy* copiesHead;

    long long searchFrequency;
    long long borrowImpact;

    Book()
        : totalCopies(0), availableCopies(0),
          copiesHead(nullptr),
          searchFrequency(0), borrowImpact(0) {}

    Book(const std::string& i, const std::string& t,
         const std::string& a, const std::string& c, int total)
        : isbn(i), title(t), author(a), category(c),
          totalCopies(total), availableCopies(total),
          copiesHead(nullptr),
          searchFrequency(0), borrowImpact(0) {

        // Create linked list of copies
        for (int k = 0; k < totalCopies; k++) {
            Copy* newCopy = new Copy(isbn + "_COPY_" + std::to_string(k));
            newCopy->next = copiesHead;
            copiesHead = newCopy;
        }
    }

    // Get first available copy
    Copy* getAvailableCopy() {
        Copy* curr = copiesHead;
        while (curr) {
            if (curr->isAvailable)
                return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    ~Book() {
        Copy* curr = copiesHead;
        while (curr) {
            Copy* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
};

/* ================= USER ================= */

struct User {
    std::string userID;
    std::string name;
    std::string email;
    UserType type;
    long long registrationDate;

    User(const std::string& id, const std::string& n,
         const std::string& e, UserType t)
        : userID(id), name(n), email(e),
          type(t), registrationDate(time(nullptr)) {}

    User() : registrationDate(0) {}
};

/* ================= TRANSACTION ================= */

struct Transaction {
    std::string transactionID;
    std::string userID;
    std::string bookID;
    std::string copyID;
    TransactionType type;
    long long timestamp;

    Transaction(const std::string& tid, const std::string& uid,
                const std::string& bid, const std::string& cid,
                TransactionType t)
        : transactionID(tid), userID(uid),
          bookID(bid), copyID(cid),
          type(t), timestamp(time(nullptr)) {}
};

/* ================= RESERVATION ================= */

struct Reservation {
    std::string reservationID;
    std::string userID;
    std::string bookID;
    long long timestamp;
    int priorityScore;

    Reservation(const std::string& rid, const std::string& uid,
                const std::string& bid, UserType type)
        : reservationID(rid), userID(uid),
          bookID(bid), timestamp(time(nullptr)) {

        if (type == UserType::FACULTY)
            priorityScore = 3;
        else if (type == UserType::FINAL_YEAR_STUDENT)
            priorityScore = 2;
        else
            priorityScore = 1;
    }
};

/* ================= SEARCH RESULT ================= */

struct SearchResult {
    std::string bookID;
    std::string isbn;
    std::string title;
    std::string author;
    std::string category;
    int availableCopies;
    int totalCopies;
    long long relevanceScore;

    SearchResult()
        : availableCopies(0),
          totalCopies(0),
          relevanceScore(0) {}
};

#endif
