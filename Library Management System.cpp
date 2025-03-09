#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <limits>
using namespace std;

struct IssuedRecord {
    string userID;    
    string isbn;       
    time_t issueTime;  
};

class Book {
private:
    string title;
    string author;
    string publisher;
    int year;
    string isbn;
    string status;        
    string reservedBy;    
    time_t reservationExpiry; 
public:
    Book() : year(0), reservationExpiry(0) {}
    Book(const string &t, const string &a, const string &p, int y, const string &i)
      : title(t), author(a), publisher(p), year(y), isbn(i),
        status("Available"), reservedBy(""), reservationExpiry(0) {}
    string getTitle() const { return title; }
    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    int getYear() const { return year; }
    string getISBN() const { return isbn; }
    string getStatus() const { return status; }
    void setStatus(const string &s) { status = s; }
    string getReservedBy() const { return reservedBy; }
    void setReservedBy(const string &uid) { reservedBy = uid; }
    time_t getReservationExpiry() const { return reservationExpiry; }
    void setReservationExpiry(time_t t) { reservationExpiry = t; }
    void display(const string &currentUserID = "") const {
        cout << "Title     : " << title << "\n"
             << "Author    : " << author << "\n"
             << "Publisher : " << publisher << "\n"
             << "Year      : " << year << "\n"
             << "ISBN      : " << isbn << "\n";
        if (status == "Reserved") {
            time_t now = time(0);
            if (currentUserID == reservedBy && now <= reservationExpiry)
                cout << "Status    : Available (Reserved exclusively for you)\n";
            else
                cout << "Status    : Reserved\n";
        } else {
            cout << "Status    : " << status << "\n";
        }
        cout << "---------------------\n";
    }

    string toCSV() const {
        stringstream ss;
        ss << title << "," << author << "," << publisher << "," << year << "," << isbn << "," 
           << status << "," << reservedBy << "," << reservationExpiry;
        return ss.str();
    }
    static Book fromCSV(const string &line) {
        stringstream ss(line);
        string t, a, p, yearStr, i, s, rby, resExpStr;
        getline(ss, t, ',');
        getline(ss, a, ',');
        getline(ss, p, ',');
        getline(ss, yearStr, ',');
        getline(ss, i, ',');
        getline(ss, s, ',');
        getline(ss, rby, ',');
        getline(ss, resExpStr, ',');
        Book book(t, a, p, stoi(yearStr), i);
        book.setStatus(s);
        book.setReservedBy(rby);
        book.setReservationExpiry(resExpStr.empty() ? 0 : stol(resExpStr));
        return book;
    }
};

class Account {
private:
    vector<Book*> currentlyBorrowed;
    double fineAmount;
    bool fineSettlementPending; 
public:
    Account() : fineAmount(0.0), fineSettlementPending(false) {}
    void addBorrowedBook(Book* book) { currentlyBorrowed.push_back(book); }
    void removeBorrowedBook(Book* book) {
        auto it = find(currentlyBorrowed.begin(), currentlyBorrowed.end(), book);
        if (it != currentlyBorrowed.end())
            currentlyBorrowed.erase(it);
    }
    void addFine(double fine) { fineAmount += fine; }
    void clearFine() { fineAmount = 0.0; }
    double getFine() const { return fineAmount; }
    size_t getBorrowedCount() const { return currentlyBorrowed.size(); }
    bool isFineSettlementPending() const { return fineSettlementPending; }
    void requestFineSettlement() { fineSettlementPending = true; }
    void approveFineSettlement() { fineSettlementPending = false; clearFine(); }
    void displayBorrowedBooks() const {
        if(currentlyBorrowed.empty()){
            cout << "You have not borrowed any books.\n";
            return;
        }
        for(auto book : currentlyBorrowed) {
            book->display();
        }
    }
};

class User {
protected:
    string id;
    string name;
    string password;
    string role; 
    Account account;
public:
    User() {}
    User(const string &uid, const string &uname, const string &pass, const string &r)
      : id(uid), name(uname), password(pass), role(r) {}
    bool verifyPassword(const string &pass) const { return password == pass; }
    virtual void borrowBook(Book* book) = 0;
    virtual void returnBook(Book* book, int daysBorrowed) = 0;
    string getID() const { return id; }
    string getName() const { return name; }
    string getRole() const { return role; }
    Account& getAccount() { return account; }
    virtual void display() const {
        cout << "User ID: " << id << ", Name: " << name << ", Role: " << role 
             << ", Fine: " << account.getFine() << "\n";
    }
    string toCSV() const {
        stringstream ss;
        ss << id << "," << name << "," << password << "," << role << "," << account.getFine();
        return ss.str();
    }
    virtual ~User() {}
};

class Student : public User {
private:
    static const int maxBooks = 3;
    static const int borrowingPeriod = 15;
    const int fineRate = 10;
public:
    Student(const string &uid, const string &uname, const string &pass)
      : User(uid, uname, pass, "Student") {}
    virtual void borrowBook(Book* book) override {
        if(account.getFine() > 0 || account.isFineSettlementPending()) {
            cout << "You have an outstanding fine. Please clear fines before borrowing new books.\n";
            return;
        }
        if(account.getBorrowedCount() >= maxBooks) {
            cout << "You have reached your borrowing limit of " << maxBooks << " books.\n";
            return;
        }
        if(book->getStatus() == "Available") {
            book->setStatus("Borrowed");
            account.addBorrowedBook(book);
            cout << "Book successfully borrowed by student " << name << ".\n";
        }
        else if(book->getStatus() == "Reserved") {
            time_t now = time(0);
            if(book->getReservedBy() == id && now <= book->getReservationExpiry()){
                book->setStatus("Borrowed");
                book->setReservedBy("");
                book->setReservationExpiry(0);
                account.addBorrowedBook(book);
                cout << "Book successfully borrowed (from reservation) by student " << name << ".\n";
            } else {
                cout << "Book is reserved by another user.\n";
            }
        }
        else {
            cout << "Book is not available.\n";
        }
    }
    virtual void returnBook(Book* book, int daysBorrowed) override {
        book->setStatus("Available");
        account.removeBorrowedBook(book);
        if(daysBorrowed > borrowingPeriod) {
            int overdue = daysBorrowed - borrowingPeriod;
            double fine = overdue * fineRate;
            account.addFine(fine);
            cout << "Book returned after " << daysBorrowed << " days. Fine of " << fine << " rupees applied.\n";
        }
        else {
            cout << "Book returned on time.\n";
        }
        if(!book->getReservedBy().empty()) {
            book->setStatus("Reserved");
            book->setReservationExpiry(time(0) + 5 * 24 * 3600); 
            cout << "Book will be reserved exclusively for user " << book->getReservedBy() << " for 5 days.\n";
        }
    }
    virtual void display() const override {
        cout << "Student: ";
        User::display();
    }
};

class Faculty : public User {
private:
    static const int maxBooks = 5;
    static const int borrowingPeriod = 30; 
public:
    Faculty(const string &uid, const string &uname, const string &pass)
      : User(uid, uname, pass, "Faculty") {}
    virtual void borrowBook(Book* book) override {
        if(account.getFine() > 0) {
            cout << "You have an outstanding fine. Please clear it before borrowing new books.\n";
            return;
        }
        if(account.getBorrowedCount() >= maxBooks) {
            cout << "You have reached your borrowing limit of " << maxBooks << " books.\n";
            return;
        }
        if(book->getStatus() == "Available") {
            book->setStatus("Borrowed");
            account.addBorrowedBook(book);
            cout << "Book successfully borrowed by faculty " << name << ".\n";
        }
        else if(book->getStatus() == "Reserved") {
            time_t now = time(0);
            if(book->getReservedBy() == id && now <= book->getReservationExpiry()){
                book->setStatus("Borrowed");
                book->setReservedBy("");
                book->setReservationExpiry(0);
                account.addBorrowedBook(book);
                cout << "Book successfully borrowed (from reservation) by faculty " << name << ".\n";
            } else {
                cout << "Book is reserved by another user.\n";
            }
        }
        else {
            cout << "Book is not available.\n";
        }
    }
    virtual void returnBook(Book* book, int daysBorrowed) override {
        book->setStatus("Available");
        account.removeBorrowedBook(book);
        cout << "Book returned successfully.\n";
        if(!book->getReservedBy().empty()){
            book->setStatus("Reserved");
            book->setReservationExpiry(time(0) + 5 * 24 * 3600);
            cout << "Book is now reserved exclusively for user " << book->getReservedBy() << " for 5 days.\n";
        }
    } 
    virtual void display() const override {
        cout << "Faculty: ";
        User::display();
    }
};

class Librarian : public User {
public:
    Librarian(const string &uid, const string &uname, const string &pass)
      : User(uid, uname, pass, "Librarian") {}
    virtual void borrowBook(Book* ) override {
        cout << "Librarians cannot borrow books.\n";
    }
    virtual void returnBook(Book*, int ) override {
        cout << "Librarians cannot return books.\n";
    }
    virtual void display() const override {
        cout << "Librarian: ";
        User::display();
    }
};

class Library {
private:
    vector<Book> books;
    vector<User*> users;
    vector<IssuedRecord> issued;
public:
    void addNewBook(const Book &b) {
        if(searchBookByISBN(b.getISBN()) != nullptr) {
            cout << "A book with ISBN " << b.getISBN() << " already exists. Not added.\n";
            return;
        }
        books.push_back(b);
        saveBooks("books.csv");
    }
    Book* searchBookByISBN(const string &isbn) {
        for(auto &b : books)
            if(b.getISBN() == isbn)
                return &b;
        return nullptr;
    }
    void displayAllBooks(const string &currentUserID = "") {
        cout << "\n--- All Books ---\n";
        for(const Book &b : books)
            b.display(currentUserID);
    }
    void addNewUser(User* u) {
        if(getUserById(u->getID()) != nullptr) {
            cout << "A user with ID " << u->getID() << " already exists. Not added.\n";
            delete u;
            return;
        }
        users.push_back(u);
        saveUsers("users.csv");
    }
    User* getUserById(const string &uid) {
        for(auto u : users)
            if(u->getID() == uid)
                return u;
        return nullptr;
    }
    void displayAllUsers() {
        cout << "\n--- All Users ---\n";
        for(const User* u : users) {
            u->display();
            cout << "---------------------\n";
        }
    }
    void removeUserIfPossible(const string &uid) {
        User* u = getUserById(uid);
        if(!u) {
            cout << "User not found.\n";
            return;
        }
        if(u->getAccount().getBorrowedCount() > 0) {
            cout << "Cannot remove user " << uid << " because they have borrowed books.\n";
            return;
        }
        users.erase(remove_if(users.begin(), users.end(), [&](User* x){ return x->getID() == uid; }), users.end());
        delete u;
        cout << "User " << uid << " removed successfully.\n";
        saveUsers("users.csv");
    }
    void issueBook(const string &uid, const string &isbn) {
        Book* book = searchBookByISBN(isbn);
        if(!book) {
            cout << "Book with ISBN " << isbn << " not found.\n";
            return;
        }
        if(book->getStatus() == "Borrowed") {
            cout << "Book is already issued.\n";
            return;
        }
        if(book->getStatus() == "Reserved") {
            time_t now = time(0);
            if(book->getReservedBy() != uid || now > book->getReservationExpiry()){
                cout << "Book is reserved by another user.\n";
                return;
            }
        }
        User* u = getUserById(uid);
        if(u && (u->getAccount().getFine() > 0 || u->getAccount().isFineSettlementPending())) {
            cout << "Outstanding fine exists. Clear fines before borrowing.\n";
            return;
        }
        u->borrowBook(book); 
        if(book->getStatus() == "Borrowed") {
            IssuedRecord rec { uid, isbn, time(0) };
            issued.push_back(rec);
            saveBooks("books.csv");
            saveIssued("issued.csv");
            cout << "Book (ISBN " << isbn << ") issued to user " << uid << ".\n";
        }
    }
    void returnBook(const string &uid, const string &isbn, int daysBorrowed) {
        Book* book = searchBookByISBN(isbn);
        if(!book) {
            cout << "Book with ISBN " << isbn << " not found.\n";
            return;
        }
        auto it = remove_if(issued.begin(), issued.end(), [&](const IssuedRecord &r) {
            return r.userID == uid && r.isbn == isbn;
        });
        if(it != issued.end())
            issued.erase(it, issued.end());
        User* u = getUserById(uid);
        if(u)
            u->returnBook(book, daysBorrowed);
        if(!book->getReservedBy().empty()) {
            book->setStatus("Reserved");
            book->setReservationExpiry(time(0) + 5 * 24 * 3600);
            cout << "Book is now reserved exclusively for user " << book->getReservedBy() << " for 5 days.\n";
        } else {
            book->setStatus("Available");
        }
        saveBooks("books.csv");
        saveIssued("issued.csv");
        saveUsers("users.csv");
    }
    void displayIssuedRecords() {
        cout << "\n--- Issued Records ---\n";
        if(issued.empty()) {
            cout << "No books are currently issued.\n";
            return;
        }
        for(const IssuedRecord &r : issued) {
            cout << "User ID: " << r.userID << ", ISBN: " << r.isbn
                 << ", Issue Date: " << ctime(&r.issueTime);
        }
    }
    void displayBorrowingDetails() {
        cout << "\n--- Borrowing Details ---\n";
        if(issued.empty()){
            cout << "No books are currently issued.\n";
            return;
        }
        for(const IssuedRecord &r : issued) {
            cout << "User ID: " << r.userID << ", ISBN: " << r.isbn
                 << ", Issue Date: " << ctime(&r.issueTime);
        }
    }
    void reserveBook(const string &uid, const string &isbn) {
        Book* book = searchBookByISBN(isbn);
        if(!book) {
            cout << "Book with ISBN " << isbn << " not found.\n";
            return;
        }
        if(book->getStatus() != "Borrowed") {
            cout << "Book is available; you may borrow it.\n";
            return;
        }
        if(!book->getReservedBy().empty()) {
            cout << "Book is already reserved by another user.\n";
            return;
        }
        book->setReservedBy(uid);
        cout << "Book reserved successfully. Once returned, it will be available exclusively for you for 5 days.\n";
        saveBooks("books.csv");
    }
    void loadBooks(const string &filename) {
        ifstream inFile(filename);
        if(!inFile) {
            cout << "Books file \"" << filename << "\" not found. It will be created on saving.\n";
            return;
        }
        books.clear();
        string line;
        while(getline(inFile, line)) {
            if(line.empty()) continue;
            Book book = Book::fromCSV(line);
            books.push_back(book);
        }
        inFile.close();
        cout << "Loaded books from \"" << filename << "\"\n";
    }
    void saveBooks(const string &filename) {
        ofstream outFile(filename);
        if(!outFile) {
            cout << "Error writing to \"" << filename << "\"\n";
            return;
        }
        for(const Book &b : books)
            outFile << b.toCSV() << "\n";
        outFile.close();
        cout << "Saved books to \"" << filename << "\"\n";
    }
    void loadUsers(const string &filename) {
        ifstream inFile(filename);
        if(!inFile) {
            cout << "Users file \"" << filename << "\" not found. It will be created on saving.\n";
            return;
        }
        users.clear();
        string line;
        while(getline(inFile, line)) {
            if(line.empty()) continue;
            stringstream ss(line);
            vector<string> tokens;
            string token;
            while(getline(ss, token, ','))
                tokens.push_back(token);
            if(tokens.size() < 5) continue;
            string uid = tokens[0], uname = tokens[1], upass = tokens[2], urole = tokens[3];
            double ufine = stod(tokens[4]);
            User* u = nullptr;
            if(urole == "Student")
                u = new Student(uid, uname, upass);
            else if(urole == "Faculty")
                u = new Faculty(uid, uname, upass);
            else if(urole == "Librarian")
                u = new Librarian(uid, uname, upass);
            if(u) {
                u->getAccount().clearFine();
                u->getAccount().addFine(ufine);
                users.push_back(u);
            }
        }
        inFile.close();
        cout << "Loaded users from \"" << filename << "\"\n";
    }
    void saveUsers(const string &filename) {
        ofstream outFile(filename);
        if(!outFile) {
            cout << "Error writing to \"" << filename << "\"\n";
            return;
        }
        for(const User* u : users)
            outFile << u->toCSV() << "\n";
        outFile.close();
        cout << "Saved users to \"" << filename << "\"\n";
    }
    void loadIssued(const string &filename) {
        ifstream inFile(filename);
        if(!inFile) {
            cout << "Issued records file \"" << filename << "\" not found. It will be created on saving.\n";
            return;
        }
        issued.clear();
        string line;
        while(getline(inFile, line)) {
            if(line.empty()) continue;
            stringstream ss(line);
            string uid, isbn, timeStr;
            getline(ss, uid, ',');
            getline(ss, isbn, ',');
            getline(ss, timeStr, ',');
            IssuedRecord rec { uid, isbn, static_cast<time_t>(stol(timeStr)) };
            issued.push_back(rec);
        }
        inFile.close();
        cout << "Loaded issued records from \"" << filename << "\"\n";
    }
    void saveIssued(const string &filename) {
        ofstream outFile(filename);
        if(!outFile) {
            cout << "Error writing to \"" << filename << "\"\n";
            return;
        }
        for(const IssuedRecord &r : issued)
            outFile << r.userID << "," << r.isbn << "," << r.issueTime << "\n";
        outFile.close();
        cout << "Saved issued records to \"" << filename << "\"\n";
    }
    void loadAllData() {
        loadBooks("books.csv");
        loadUsers("users.csv");
        loadIssued("issued.csv");
    }
    void saveAllData() {
        saveBooks("books.csv");
        saveUsers("users.csv");
        saveIssued("issued.csv");
    }
    ~Library() {
        for(auto u : users)
            delete u;
    }
};

void studentMenu(Library &lib, User* user) {
    int choice;
    while (true) {
        cout << "\n--- Student Menu (" << user->getName() << ") ---\n";
        cout << "1. View All Books\n";
        cout << "2. Borrow Book\n";
        cout << "3. Reserve Book\n";
        cout << "4. Return Book\n";
        cout << "5. View Outstanding Fine\n";
        cout << "6. Request Fine Clearance\n";
        cout << "7. View Currently Borrowed Books\n";
        cout << "8. Logout\n";
        cout << "Enter your choice: ";
        if(!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        if(choice == 8) break;
        switch(choice) {
            case 1:
                lib.displayAllBooks(user->getID());
                break;
            case 2:
                if(user->getAccount().getFine() > 0 || user->getAccount().isFineSettlementPending()){
                    cout << "Cannot borrow new books until outstanding fines are cleared.\n";
                } else {
                    {
                      string isbn;
                      cout << "Enter ISBN to borrow: ";
                      cin >> isbn;
                      lib.issueBook(user->getID(), isbn);
                    }
                }
                break;
            case 3:
                {
                    string isbn;
                    cout << "Enter ISBN to reserve: ";
                    cin >> isbn;
                    lib.reserveBook(user->getID(), isbn);
                }
                break;
            case 4:
                {
                    string isbn;
                    int days;
                    cout << "Enter ISBN to return: ";
                    cin >> isbn;
                    cout << "Enter number of days since issue: ";
                    cin >> days;
                    lib.returnBook(user->getID(), isbn, days);
                }
                break;
            case 5:
                cout << "Outstanding fine: " << user->getAccount().getFine() << " rupees\n";
                break;
            case 6:
                if(user->getAccount().getFine() > 0 && !user->getAccount().isFineSettlementPending()){
                    user->getAccount().requestFineSettlement();
                    lib.saveUsers("users.csv");
                    cout << "Your fine clearance request has been sent for librarian approval.\n";
                } else if(user->getAccount().isFineSettlementPending()){
                    cout << "Your fine clearance request is pending approval.\n";
                } else {
                    cout << "No fine to clear.\n";
                }
                break;
            case 7:
                user->getAccount().displayBorrowedBooks();
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void facultyMenu(Library &lib, User* user) {
    int choice;
    while (true) {
        cout << "\n--- Faculty Menu (" << user->getName() << ") ---\n";
        cout << "1. View All Books\n";
        cout << "2. Borrow Book\n";
        cout << "3. Reserve Book\n";
        cout << "4. Return Book\n";
        cout << "5. View Currently Borrowed Books\n";
        cout << "6. Logout\n";
        cout << "Enter your choice: ";
        if(!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        if(choice == 6) break;
        switch(choice) {
            case 1:
                lib.displayAllBooks(user->getID());
                break;
            case 2:
                if(user->getAccount().getFine() > 0){
                    cout << "Outstanding fine exists. Clear it before borrowing new books.\n";
                } else {
                    {
                      string isbn;
                      cout << "Enter ISBN to borrow: ";
                      cin >> isbn;
                      lib.issueBook(user->getID(), isbn);
                    }
                }
                break;
            case 3:
                {
                    string isbn;
                    cout << "Enter ISBN to reserve: ";
                    cin >> isbn;
                    lib.reserveBook(user->getID(), isbn);
                }
                break;
            case 4:
                {
                    string isbn;
                    int days;
                    cout << "Enter ISBN to return: ";
                    cin >> isbn;
                    cout << "Enter number of days since issue: ";
                    cin >> days;
                    lib.returnBook(user->getID(), isbn, days);
                }
                break;
            case 5:
                user->getAccount().displayBorrowedBooks();
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void librarianMenu(Library &lib, User* user) {
    int choice;
    while (true) {
        cout << "\n--- Librarian Menu (" << user->getName() << ") ---\n";
        cout << "1. Add Book\n";
        cout << "2. Add User\n";
        cout << "3. Remove User\n";
        cout << "4. Search Book by ISBN\n";
        cout << "5. List All Books\n";
        cout << "6. View Borrowing Details\n";
        cout << "7. List All Users\n";
        cout << "8. Approve Fine Clearance for a User\n";
        cout << "9. Logout\n";
        cout << "Enter your choice: ";
        if(!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        if(choice == 9) break;
        switch(choice) {
            case 1:
                {
                    string title, author, publisher, isbn;
                    int year;
                    cout << "Enter title: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, title);
                    cout << "Enter author: ";
                    getline(cin, author);
                    cout << "Enter publisher: ";
                    getline(cin, publisher);
                    cout << "Enter year: ";
                    cin >> year;
                    cout << "Enter ISBN: ";
                    cin >> isbn;
                    if(lib.searchBookByISBN(isbn) != nullptr) {
                        cout << "Book with ISBN " << isbn << " already exists. Cannot add duplicate.\n";
                    } else {
                        Book newBook(title, author, publisher, year, isbn);
                        lib.addNewBook(newBook);
                    }
                }
                break;
            case 2:
                {
                    string uid, uname, upass, role;
                    cout << "Enter new User ID: ";
                    cin >> uid;
                    if(lib.getUserById(uid) != nullptr) {
                        cout << "User with ID " << uid << " already exists. Cannot add duplicate.\n";
                        break;
                    }
                    cout << "Enter name: ";
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    getline(cin, uname);
                    cout << "Enter password: ";
                    cin >> upass;
                    cout << "Enter role (Student/Faculty): ";
                    cin >> role;
                    if(role != "Student" && role != "Faculty") {
                        cout << "Invalid role. Only Student or Faculty allowed.\n";
                        break;
                    }
                    User* newUser = (role == "Student") ? static_cast<User*>(new Student(uid, uname, upass)): static_cast<User*>(new Faculty(uid, uname, upass));
                    lib.addNewUser(newUser);
                    cout << "User added successfully.\n";
                }
                break;
            case 3:
                {
                    string uid;
                    cout << "Enter User ID to remove: ";
                    cin >> uid;
                    lib.removeUserIfPossible(uid);
                }
                break;
            case 4:
                {
                    string isbn;
                    cout << "Enter ISBN to search: ";
                    cin >> isbn;
                    Book* b = lib.searchBookByISBN(isbn);
                    if(b) b->display();
                    else cout << "Book not found.\n";
                }
                break;
            case 5:
                lib.displayAllBooks();
                break;
            case 6:
                lib.displayBorrowingDetails();
                break;
            case 7:
                lib.displayAllUsers();
                break;
            case 8:
                {
                    string uid;
                    cout << "Enter User ID to approve fine clearance: ";
                    cin >> uid;
                    User* u = lib.getUserById(uid);
                    if(u && u->getAccount().isFineSettlementPending()){
                        u->getAccount().approveFineSettlement();
                        lib.saveUsers("users.csv");
                        cout << "Fine for user " << uid << " has been approved and cleared.\n";
                    } else {
                        cout << "No pending fine clearance for this user or user not found.\n";
                    }
                }
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

User* login(Library &lib) {
    string uid, pass;
    cout << "Enter User ID: ";
    cin >> uid;
    cout << "Enter Password: ";
    cin >> pass;
    User* user = lib.getUserById(uid);
    if(!user) {
        cout << "User not found.\n";
        return nullptr;
    }
    if(!user->verifyPassword(pass)){
        cout << "Invalid password.\n";
        return nullptr;
    }
    return user;
}

int main(){
    Library library;
    library.loadBooks("books.csv");
    library.loadUsers("users.csv");
    library.loadIssued("issued.csv");
    if(library.searchBookByISBN("ISBN-001") == nullptr) {
        library.addNewBook(Book("C++ Primer", "Stanley Lippman", "Addison-Wesley", 2012, "ISBN-001"));
        library.addNewBook(Book("Effective C++", "Scott Meyers", "O'Reilly", 2005, "ISBN-002"));
        library.addNewBook(Book("The C++ Programming Language", "Bjarne Stroustrup", "Addison-Wesley", 2013, "ISBN-003"));
        library.addNewBook(Book("Programming: Principles and Practice", "Bjarne Stroustrup", "Addison-Wesley", 2014, "ISBN-004"));
        library.addNewBook(Book("Modern C++ Design", "Andrei Alexandrescu", "Addison-Wesley", 2001, "ISBN-005"));
        library.addNewBook(Book("C++ Concurrency in Action", "Anthony Williams", "Manning", 2019, "ISBN-006"));
        library.addNewBook(Book("Clean Code", "Robert C. Martin", "Prentice Hall", 2008, "ISBN-007"));
        library.addNewBook(Book("Design Patterns", "Erich Gamma", "Addison-Wesley", 1994, "ISBN-008"));
        library.addNewBook(Book("Effective STL", "Scott Meyers", "O'Reilly", 2001, "ISBN-009"));
        library.addNewBook(Book("The Pragmatic Programmer", "Andrew Hunt", "Addison-Wesley", 1999, "ISBN-010"));
        library.saveBooks("books.csv");
    }
    if(library.getUserById("1") == nullptr) {
        library.addNewUser(new Student("1", "Alice", "alicepwd"));
        library.addNewUser(new Student("2", "Bob", "bobpwd"));
        library.addNewUser(new Student("3", "Charlie", "charliepwd"));
        library.addNewUser(new Student("4", "David", "davidpwd"));
        library.addNewUser(new Student("5", "Eva", "evapwd"));
        library.addNewUser(new Faculty("6", "Prof. Smith", "smithpwd"));
        library.addNewUser(new Faculty("7", "Prof. Johnson", "johnsonpwd"));
        library.addNewUser(new Faculty("8", "Prof. Williams", "williampwd"));
        library.addNewUser(new Librarian("9", "Librarian Karen", "karenpwd"));
        library.saveUsers("users.csv");
    }   
    int mainChoice;
    while (true) {
        cout << "\n--- Library Management System ---\n";
        cout << "1. Login\n";
        cout << "2. Exit\n";
        cout << "Enter your choice: ";
        if(!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        if(mainChoice == 2)
            break;
        if(mainChoice != 1) {
            cout << "Invalid choice.\n";
            continue;
        }
        User* currentUser = login(library);
        if(!currentUser)
            continue;
        string role = currentUser->getRole();
        if(role == "Student")
            studentMenu(library, currentUser);
        else if(role == "Faculty")
            facultyMenu(library, currentUser);
        else if(role == "Librarian")
            librarianMenu(library, currentUser);
    }
    library.saveBooks("books.csv");
    library.saveUsers("users.csv");
    library.saveIssued("issued.csv");
    cout << "Goodbye!\n";
    return 0;
}
